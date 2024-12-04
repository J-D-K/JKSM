#include "FS/IO.hpp"
#include "FS/SaveMount.hpp"
#include "Logger.hpp"
#include "StringUtil.hpp"
#include "UI/Strings.hpp"
#include <condition_variable>
#include <cstring>
#include <ctime>
#include <memory>
#include <mutex>
#include <vector>

namespace
{
    // Buffer size used for reading and writing files.
    constexpr size_t FILE_BUFFER_SIZE = 0x10000;
    // This is so my 3DS doesn't choke to death trying to write zip files.
    constexpr size_t ZIP_BUFFER_SIZE = 0x8000;
    // This is the struct the threads share so they can coordinate with eachother.
    typedef struct
    {
            // Protect buffer from corruption.
            std::mutex BufferMutex;
            std::condition_variable BufferCondition;
            bool BufferIsFull = false;
            // This is the buffer read into.
            std::unique_ptr<unsigned char[]> SharedBuffer;
            // The main thread needs to know these to write.
            size_t BytesRead = 0;
    } SharedThreadData;
} // namespace

void ReadThreadFunction(FsLib::File &SourceFile, SharedThreadData *Data)
{
    uint64_t FileSize = SourceFile.GetSize();
    uint64_t TotalBytesRead = 0;
    while (TotalBytesRead < FileSize)
    {
        // Read data from source file.
        Data->BytesRead = SourceFile.Read(Data->SharedBuffer.get(), FILE_BUFFER_SIZE);
        // Check and log.
        if (Data->BytesRead == 0)
        {
            Logger::Log("Error reading from file: %s", FsLib::GetErrorString());
        }

        // Update total byte count
        TotalBytesRead += Data->BytesRead;

        // Signal to other thread it can start a copy and write.
        Data->BufferIsFull = true;
        Data->BufferCondition.notify_one();

        // Wait until buffer is full is false. This will release the mutex right away, but since the main/write thread copies the buffer, it doesn't matter if we read to it right away.
        std::unique_lock<std::mutex> BufferLock(Data->BufferMutex);
        Data->BufferCondition.wait(BufferLock, [Data]() { return Data->BufferIsFull == false; });
    }
}

void FS::CopyDirectoryToDirectory(System::ProgressTask *Task, const FsLib::Path &Source, const FsLib::Path &Destination, bool Commit)
{
    FsLib::Directory SourceDir(Source);
    if (!SourceDir.IsOpen())
    {
        Logger::Log("Error opening directory: %s", FsLib::GetErrorString());
        return;
    }

    for (uint32_t i = 0; i < SourceDir.GetEntryCount(); i++)
    {
        if (SourceDir.EntryAtIsDirectory(i))
        {
            FsLib::Path NewSource = Source / SourceDir[i];
            FsLib::Path NewDestination = Destination / SourceDir[i];
            // Check to make sure it exists first...
            if (!FsLib::DirectoryExists(NewDestination) && !FsLib::CreateDirectory(NewDestination))
            {
                Logger::Log("Error creating destination directory: %s", FsLib::GetErrorString());
                continue;
            }
            FS::CopyDirectoryToDirectory(Task, NewSource, NewDestination, Commit);
        }
        else
        {
            // Full paths.
            FsLib::Path FullSource = Source / SourceDir[i];
            FsLib::Path FullDestination = Destination / SourceDir[i];

            // Files
            FsLib::File SourceFile(FullSource, FS_OPEN_READ);
            FsLib::File DestinationFile(FullDestination, FS_OPEN_CREATE | FS_OPEN_WRITE, SourceFile.GetSize());

            // Check
            if (!SourceFile.IsOpen() || !DestinationFile.IsOpen())
            {
                Logger::Log("Error opening one of the files: %s", FsLib::GetErrorString());
                continue;
            }

            // Get FullSource as UTF8 and update task
            char UTF8Buffer[0x301] = {0};
            StringUtil::ToUTF8(FullSource.CString(), UTF8Buffer, 0x301);
            if (Task)
            {
                Task->SetStatus(UI::Strings::GetStringByName(UI::Strings::Names::CopyingFile, 0), UTF8Buffer);
                Task->Reset(static_cast<double>(SourceFile.GetSize()));
            }

            // Create shared data for threads and allocate the buffer.
            SharedThreadData Data;
            Data.SharedBuffer = std::make_unique<unsigned char[]>(FILE_BUFFER_SIZE);

            // Grab file size quick.
            uint64_t FileSize = SourceFile.GetSize();

            // Spawn read thread early.
            std::thread ReadThread(ReadThreadFunction, std::ref(SourceFile), &Data);

            // Total bytes written and localbuffer to copy to. This is scoped so the buffer frees itself earlier.
            uint64_t TotalBytesWritten = 0;
            std::unique_ptr<unsigned char[]> LocalBuffer(new unsigned char[FILE_BUFFER_SIZE]);
            while (TotalBytesWritten < FileSize)
            {
                uint32_t BytesRead = 0;
                // Wait for buffer to be filled.
                {
                    std::unique_lock<std::mutex> BufferLock(Data.BufferMutex);
                    Data.BufferCondition.wait(BufferLock, [&Data]() { return Data.BufferIsFull == true; });

                    // Copy from shared to local.
                    BytesRead = Data.BytesRead;
                    std::memcpy(LocalBuffer.get(), Data.SharedBuffer.get(), BytesRead);

                    // Signal to other thread to continue reading.
                    Data.BufferIsFull = false;
                    Data.BufferCondition.notify_one();
                }
                // Write data.
                DestinationFile.Write(LocalBuffer.get(), BytesRead);

                // Update count
                TotalBytesWritten += BytesRead;

                // Update progress
                if (Task)
                {
                    Task->SetCurrent(static_cast<double>(TotalBytesWritten));
                }
            }

            // Join read thread
            ReadThread.join();

            // Need to close destination to commit. FsLib::File automatically closes its handle when out of scope, so normally this isn't needed.
            if (Commit)
            {
                DestinationFile.Close();
                FsLib::ControlDevice(SAVE_MOUNT);
            }
        }
    }
}

void FS::CopyDirectoryToZip(System::ProgressTask *Task, const FsLib::Path &Source, zipFile Destination)
{
    FsLib::Directory SourceDir(Source);
    if (!SourceDir.IsOpen())
    {
        Logger::Log("Error opening source directory: %s", FsLib::GetErrorString());
        return;
    }

    for (uint32_t i = 0; i < SourceDir.GetEntryCount(); i++)
    {
        if (SourceDir.EntryAtIsDirectory(i))
        {
            FsLib::Path NewSource = Source / SourceDir[i];
            FS::CopyDirectoryToZip(Task, NewSource, Destination);
        }
        else
        {
            FsLib::Path FullSource = Source / SourceDir[i];
            FsLib::File SourceFile(FullSource, FS_OPEN_READ);
            if (!SourceFile.IsOpen())
            {
                Logger::Log("Error opening source file for ZIP: %s", FsLib::GetErrorString());
                continue;
            }

            // Get time using C standard stuff cause I don't feel like doing it with ctrulib
            std::time_t Timer;
            std::time(&Timer);
            std::tm *LocalTime = localtime(&Timer);

            zip_fileinfo FileInfo = {.tmz_date = {.tm_sec = LocalTime->tm_sec,
                                                  .tm_min = LocalTime->tm_min,
                                                  .tm_hour = LocalTime->tm_hour,
                                                  .tm_mday = LocalTime->tm_mday,
                                                  .tm_mon = LocalTime->tm_mon,
                                                  .tm_year = 1900 + LocalTime->tm_year},
                                     .dosDate = 0,
                                     .internal_fa = 0,
                                     .external_fa = 0};

            // We need to convert the UTF16 to UTF for the zip. This is a pain thanks to 3DS needing UTF-16 paths, but here we go.
            // Full path.
            FsLib::Path ZipFileName = Source / SourceDir[i];
            // Get pointer to C string.
            const char16_t *PathPointer = ZipFileName.CString();
            // Find where the path begins...
            const char16_t *PathBegin = std::char_traits<char16_t>::find(PathPointer, ZipFileName.GetLength(), u'/') + 1;
            // Covert to UTF-8
            char UTF8Buffer[FsLib::MAX_PATH] = {0};
            utf16_to_utf8(reinterpret_cast<uint8_t *>(UTF8Buffer), reinterpret_cast<const uint16_t *>(PathBegin), FsLib::MAX_PATH);
            // That should do it.

            int ZipError = zipOpenNewFileInZip(Destination, UTF8Buffer, &FileInfo, NULL, 0, NULL, 0, NULL, Z_DEFLATED, 1);
            if (ZipError != Z_OK)
            {
                Logger::Log("Error opening file in ZIP: %i", ZipError);
                continue;
            }

            // Set task stuff.
            if (Task)
            {
                Task->SetStatus(UI::Strings::GetStringByName(UI::Strings::Names::AddingToZip, 0), UTF8Buffer);
                Task->Reset(static_cast<double>(SourceFile.GetSize()));
            }

            uint32_t TotalCopied = 0;
            std::unique_ptr<unsigned char[]> FileBuffer(new unsigned char[ZIP_BUFFER_SIZE]);
            while (TotalCopied < SourceFile.GetSize())
            {
                uint32_t BytesRead = SourceFile.Read(FileBuffer.get(), ZIP_BUFFER_SIZE);

                ZipError = zipWriteInFileInZip(Destination, FileBuffer.get(), BytesRead);
                if (ZipError == Z_OK)
                {
                    Logger::Log("%s", FsLib::GetErrorString());
                }
                TotalCopied += BytesRead;

                if (Task)
                {
                    Task->SetCurrent(TotalCopied);
                }
            }
            zipCloseFileInZip(Destination);
        }
    }
}

void FS::CopyZipToDirectory(System::ProgressTask *Task, unzFile Source, const FsLib::Path &Destination, bool Commit)
{
    Logger::Log("CopyZipToDir");

    int UnzError = unzGoToFirstFile(Source);
    if (UnzError != UNZ_OK)
    {
        Logger::Log("Error opening file in zip for restore.");
        Task->Finish();
        return;
    }

    do
    {
        if (unzOpenCurrentFile(Source) != UNZ_OK)
        {
            Logger::Log("Error opening file in zip!");
            continue;
        }
        Logger::Log("unzOpenCurrent");
        // This really is all needed.
        unz_file_info FileInfo = {0};
        char FileNameUTF8[FsLib::MAX_PATH] = {0};
        char16_t FileNameUTF16[FsLib::MAX_PATH] = {0};
        if (unzGetCurrentFileInfo(Source, &FileInfo, FileNameUTF8, FsLib::MAX_PATH, NULL, 0, NULL, 0) != UNZ_OK)
        {
            Logger::Log("Error reading file info from zip!");
            continue;
        }
        Logger::Log("unzGetCurrentInfo");

        // Convert to UTF-16 for Path
        StringUtil::ToUTF16(FileNameUTF8, FileNameUTF16, FsLib::MAX_PATH);
        FsLib::Path DestinationPath = Destination / FileNameUTF16;
        Logger::Log("UTF-8 -> UTF-16");

        // Make sure we create entire path before file name.
        FsLib::Path TargetDir = DestinationPath.SubPath(DestinationPath.FindLastOf(u'/'));

        char DirBuffer[0x40] = {0};
        StringUtil::ToUTF8(TargetDir.CString(), DirBuffer, 0x40);
        Logger::Log("TargetDir: %s", DirBuffer);

        // Check to make sure it isn't the root, it doesn't exist already and if it was created.
        if (!(std::char_traits<char16_t>::compare(TargetDir.CString(), u"save:", 6) == 0) && !FsLib::DirectoryExists(TargetDir) &&
            !FsLib::CreateDirectoriesRecursively(TargetDir))
        {
            Logger::Log("Error creating target directory for restore: %s", FsLib::GetErrorString());
            continue;
        }
        Logger::Log("Create target dir.");

        FsLib::File DestinationFile(DestinationPath, FS_OPEN_CREATE | FS_OPEN_WRITE, FileInfo.uncompressed_size);
        if (!DestinationFile.IsOpen())
        {
            Logger::Log("Error opening destination file for writing: %s", FsLib::GetErrorString());
            continue;
        }
        Logger::Log("Destination file.");

        int ReadCount = 0, TotalCount = 0;
        std::unique_ptr<unsigned char[]> ReadBuffer(new unsigned char[FILE_BUFFER_SIZE]);
        Task->SetStatus(UI::Strings::GetStringByName(UI::Strings::Names::CopyingFile, 0), FileNameUTF8);
        while ((ReadCount = unzReadCurrentFile(Source, ReadBuffer.get(), FILE_BUFFER_SIZE)) > 0)
        {
            DestinationFile.Write(ReadBuffer.get(), static_cast<size_t>(ReadCount));
        }

        if (Commit)
        {
            DestinationFile.Close();
            FsLib::ControlDevice(SAVE_MOUNT);
        }
    } while (unzGoToNextFile(Source) != UNZ_END_OF_LIST_OF_FILE);
}
