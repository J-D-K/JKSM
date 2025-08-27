#include "Data/SMDH.hpp"

#include "logging/logger.hpp"

namespace
{
    constexpr uint32_t FILE_PATH[] = {0x00, 0x00, 0x02, 0x6E6F6369, 0x00};
}

bool Data::LoadSMDH(uint64_t TitleID, FS_MediaType MediaType, SMDH &Out)
{
    // For path.
    uint32_t LowerID = TitleID & 0xFFFFFFFF;
    uint32_t UpperID = TitleID >> 32 & 0xFFFFFFFF;

    uint32_t ArchivePath[] = {LowerID, UpperID, MediaType, 0x00};

    FS_Path BinaryArchivePath = {PATH_BINARY, 0x10, ArchivePath};
    FS_Path BinaryFilePath    = {PATH_BINARY, 0x14, FILE_PATH};

    // I'm not going to make FsLib support this.
    Handle SDMHHandle;
    Result FsError =
        FSUSER_OpenFileDirectly(&SDMHHandle, ARCHIVE_SAVEDATA_AND_CONTENT, BinaryArchivePath, BinaryFilePath, FS_OPEN_READ, 0);
    if (R_FAILED(FsError)) { return false; }

    uint32_t BytesRead = 0;
    FsError            = FSFILE_Read(SDMHHandle, &BytesRead, 0, &Out, sizeof(SMDH));
    if (R_FAILED(FsError) || BytesRead != sizeof(SMDH))
    {
        FSFILE_Close(SDMHHandle);
        return false;
    }
    FSFILE_Close(SDMHHandle);
    return true;
}
