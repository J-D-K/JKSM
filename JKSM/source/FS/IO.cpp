#include "FS/IO.hpp"

#include "FS/SaveMount.hpp"
#include "Logger.hpp"
#include "StringUtil.hpp"
#include "Strings.hpp"

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

void ReadThreadFunction(fslib::File &SourceFile, SharedThreadData &Data)
{
    // Record file size for loop.
    uint64_t FileSize = SourceFile.get_size();

    // Loop until entire file is read.
    for (uint64_t TotalRead = 0; TotalRead < FileSize;)
    {
        // Read from source
        Data.BytesRead = SourceFile.read(Data.SharedBuffer.get(), FILE_BUFFER_SIZE);
        if (Data.BytesRead == 0)
        {
            Logger::Log("Error reading from file: %s", fslib::error::get_string());
            // To do: Handle this better if it occurs.
        }

        // Update loop count
        TotalRead += Data.BytesRead;

        // Signal to other thread it can copy and write the buffer.
        Data.BufferIsFull = true;
        Data.BufferCondition.notify_one();

        // Wait until buffer is "empty" again.
        std::unique_lock<std::mutex> BufferLock(Data.BufferMutex);
        Data.BufferCondition.wait(BufferLock, [&Data]() { return Data.BufferIsFull == false; });
    }
}

void FS::CopyDirectoryToDirectory(System::ProgressTask *Task,
                                  const fslib::Path &Source,
                                  const fslib::Path &Destination,
                                  bool Commit)
{
    fslib::Directory SourceDir(Source);
    if (!SourceDir.is_open())
    {
        Logger::Log("Error opening directory: %s", fslib::error::get_string());
        return;
    }

    for (uint32_t i = 0; i < SourceDir.get_count(); i++)
    {
        // Test to make sure JKSM doesn't copy the ._secure_value file to the save if it exists. This might be a little unsafe.
        if (std::char_traits<char16_t>::compare(u"._secure_value", SourceDir[i].get_filename(), 14) == 0) { continue; }

        if (SourceDir[i].is_directory())
        {
            fslib::Path NewSource      = Source / SourceDir[i];
            fslib::Path NewDestination = Destination / SourceDir[i];
            // Check to make sure it exists first...
            if (!fslib::directory_exists(NewDestination) && !fslib::create_directory(NewDestination))
            {
                Logger::Log("Error creating destination directory: %s", fslib::error::get_string());
                continue;
            }
            FS::CopyDirectoryToDirectory(Task, NewSource, NewDestination, Commit);
        }
        else
        {
            // Full paths.
            fslib::Path FullSource      = Source / SourceDir[i];
            fslib::Path FullDestination = Destination / SourceDir[i];

            // Files
            fslib::File SourceFile(FullSource, FS_OPEN_READ);
            fslib::File DestinationFile(FullDestination, FS_OPEN_CREATE | FS_OPEN_WRITE, SourceFile.get_size());

            // Check
            if (!SourceFile.is_open() || !DestinationFile.is_open())
            {
                Logger::Log("Error opening one of the files: %s", fslib::error::get_string());
                continue;
            }

            // Get FullSource as UTF8 and update task
            char UTF8Buffer[0x301] = {0};
            StringUtil::ToUTF8(FullSource.full_path(), UTF8Buffer, 0x301);
            if (Task)
            {
                Task->SetStatus(Strings::GetStringByName(Strings::Names::CopyingFile, 0), UTF8Buffer);
                Task->Reset(static_cast<double>(SourceFile.get_size()));
            }

            // Create shared data for threads and allocate the buffer.
            SharedThreadData Data;
            Data.SharedBuffer = std::make_unique<unsigned char[]>(FILE_BUFFER_SIZE);

            // Grab file size quick.
            uint64_t FileSize = SourceFile.get_size();
            // Spawn read thread early.
            std::thread ReadThread(ReadThreadFunction, std::ref(SourceFile), std::ref(Data));

            // LocalBuffer to copy bytes read to so we don't hold up the read thread.
            std::unique_ptr<unsigned char[]> LocalBuffer(new unsigned char[FILE_BUFFER_SIZE]);
            for (uint64_t BytesWritten = 0; BytesWritten < FileSize;)
            {
                // Need to save this for the end of the loop.
                uint32_t BytesRead = 0;

                // Scoped so the lock is released earlier.
                {
                    // Wait for signal buffer is full.
                    std::unique_lock<std::mutex> BufferLock(Data.BufferMutex);
                    Data.BufferCondition.wait(BufferLock, [&Data]() { return Data.BufferIsFull; });

                    // Record number of bytes read thread read and copy sharedbuffer contents to localbuffer.
                    BytesRead = Data.BytesRead;
                    std::memcpy(LocalBuffer.get(), Data.SharedBuffer.get(), BytesRead);

                    // Signal to other thread it can continue reading
                    Data.BufferIsFull = false;
                    Data.BufferCondition.notify_one();
                }

                // Write data to destination
                size_t WriteCount = DestinationFile.write(LocalBuffer.get(), BytesRead);
                if (WriteCount <= 0)
                {
                    Logger::Log("Error writing to file: %s", fslib::error::get_string());
                    // To do: Handle this somehow.
                }

                // Update count
                BytesWritten += WriteCount;

                // Update progress.
                if (Task) { Task->SetCurrent(static_cast<double>(BytesWritten)); }
            }

            // Join read thread
            ReadThread.join();

            // Close the destination file early just incase commit is required.
            DestinationFile.close();

            if (Commit && !fslib::control_device(FS::SAVE_MOUNT))
            {
                Logger::Log("Error committing save to device: %s", fslib::error::get_string());
            }
        }
    }
}

void FS::CopyDirectoryToZip(System::ProgressTask *Task, const fslib::Path &Source, zipFile Destination)
{
    fslib::Directory SourceDir(Source);
    if (!SourceDir.is_open())
    {
        Logger::Log("Error opening source directory: %s", fslib::error::get_string());
        return;
    }

    for (uint32_t i = 0; i < SourceDir.get_count(); i++)
    {
        if (SourceDir[i].is_directory())
        {
            fslib::Path NewSource = Source / SourceDir[i];
            FS::CopyDirectoryToZip(Task, NewSource, Destination);
        }
        else
        {
            fslib::Path FullSource = Source / SourceDir[i];
            fslib::File SourceFile(FullSource, FS_OPEN_READ);
            if (!SourceFile.is_open())
            {
                Logger::Log("Error opening source file for ZIP: %s", fslib::error::get_string());
                continue;
            }

            // Get time using C standard stuff cause I don't feel like doing it with ctrulib
            std::time_t Timer;
            std::time(&Timer);
            std::tm *LocalTime = localtime(&Timer);

            zip_fileinfo FileInfo = {.tmz_date    = {.tm_sec  = LocalTime->tm_sec,
                                                     .tm_min  = LocalTime->tm_min,
                                                     .tm_hour = LocalTime->tm_hour,
                                                     .tm_mday = LocalTime->tm_mday,
                                                     .tm_mon  = LocalTime->tm_mon,
                                                     .tm_year = 1900 + LocalTime->tm_year},
                                     .dosDate     = 0,
                                     .internal_fa = 0,
                                     .external_fa = 0};

            // We need to convert the UTF16 to UTF for the zip. This is a pain thanks to 3DS needing UTF-16 paths, but here we
            // go. Full path.
            fslib::Path ZipFileName = Source / SourceDir[i];
            // Get pointer to C string.
            const char16_t *PathPointer = ZipFileName.full_path();
            // Find where the path begins...
            const char16_t *PathBegin = std::char_traits<char16_t>::find(PathPointer, ZipFileName.get_length(), u'/') + 1;
            // Covert to UTF-8
            char UTF8Buffer[fslib::MAX_PATH] = {0};
            utf16_to_utf8(reinterpret_cast<uint8_t *>(UTF8Buffer),
                          reinterpret_cast<const uint16_t *>(PathBegin),
                          fslib::MAX_PATH);
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
                Task->SetStatus(Strings::GetStringByName(Strings::Names::AddingToZip, 0), UTF8Buffer);
                Task->Reset(static_cast<double>(SourceFile.get_size()));
            }

            uint32_t TotalCopied = 0;
            std::unique_ptr<unsigned char[]> FileBuffer(new unsigned char[FILE_BUFFER_SIZE]);
            while (TotalCopied < SourceFile.get_size())
            {
                uint32_t BytesRead = SourceFile.read(FileBuffer.get(), FILE_BUFFER_SIZE);

                ZipError = zipWriteInFileInZip(Destination, FileBuffer.get(), BytesRead);
                if (ZipError == Z_OK) { Logger::Log("%s", fslib::error::get_string()); }
                TotalCopied += BytesRead;

                if (Task) { Task->SetCurrent(TotalCopied); }
            }
            zipCloseFileInZip(Destination);
        }
    }
}

void FS::CopyZipToDirectory(System::ProgressTask *Task, unzFile Source, const fslib::Path &Destination, bool Commit)
{
    Logger::Log("CopyZipToDir");

    int UnzError = unzGoToFirstFile(Source);
    if (UnzError != UNZ_OK)
    {
        Logger::Log("Error opening file in zip for restore.");
        Task->Finish();
        return;
    }

    do {
        if (unzOpenCurrentFile(Source) != UNZ_OK)
        {
            Logger::Log("Error opening file in zip!");
            continue;
        }
        Logger::Log("unzOpenCurrent");
        // This really is all needed.
        unz_file_info FileInfo                  = {0};
        char FileNameUTF8[fslib::MAX_PATH]      = {0};
        char16_t FileNameUTF16[fslib::MAX_PATH] = {0};
        if (unzGetCurrentFileInfo(Source, &FileInfo, FileNameUTF8, fslib::MAX_PATH, NULL, 0, NULL, 0) != UNZ_OK)
        {
            Logger::Log("Error reading file info from zip!");
            continue;
        }
        Logger::Log("unzGetCurrentInfo");

        // Convert to UTF-16 for Path
        StringUtil::ToUTF16(FileNameUTF8, FileNameUTF16, fslib::MAX_PATH);
        fslib::Path DestinationPath = Destination / FileNameUTF16;
        Logger::Log("UTF-8 -> UTF-16");

        // Make sure we create entire path before file name.
        fslib::Path TargetDir = DestinationPath.sub_path(DestinationPath.find_last_of(u'/'));

        char DirBuffer[0x40] = {0};
        StringUtil::ToUTF8(TargetDir.full_path(), DirBuffer, 0x40);
        Logger::Log("TargetDir: %s", DirBuffer);

        // Check to make sure it isn't the root, it doesn't exist already and if it was created.
        if (!(std::char_traits<char16_t>::compare(TargetDir.full_path(), u"save:", 6) == 0) &&
            !fslib::directory_exists(TargetDir) && !fslib::create_directory_recursively(TargetDir))
        {
            Logger::Log("Error creating target directory for restore: %s", fslib::error::get_string());
            continue;
        }
        Logger::Log("Create target dir.");

        fslib::File DestinationFile(DestinationPath, FS_OPEN_CREATE | FS_OPEN_WRITE, FileInfo.uncompressed_size);
        if (!DestinationFile.is_open())
        {
            Logger::Log("Error opening destination file for writing: %s", fslib::error::get_string());
            continue;
        }
        Logger::Log("Destination file.");

        int ReadCount = 0, TotalCount = 0;
        std::unique_ptr<unsigned char[]> ReadBuffer(new unsigned char[FILE_BUFFER_SIZE]);
        Task->SetStatus(Strings::GetStringByName(Strings::Names::CopyingFile, 0), FileNameUTF8);
        while ((ReadCount = unzReadCurrentFile(Source, ReadBuffer.get(), FILE_BUFFER_SIZE)) > 0)
        {
            DestinationFile.write(ReadBuffer.get(), static_cast<size_t>(ReadCount));
            Task->SetCurrent((TotalCount += ReadCount));
        }

        if (Commit)
        {
            DestinationFile.close();
            fslib::control_device(FS::SAVE_MOUNT);
        }
    } while (unzGoToNextFile(Source) != UNZ_END_OF_LIST_OF_FILE);
}
