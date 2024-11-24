#include "FS/FS.hpp"
#include "Logger.hpp"
#include <array>
#include <string_view>

namespace
{
    /*
        These are the various paths JKSM has used over the years. This source file exists entirely just to convert everything.
    */
    // When folder was JKSV
    constexpr std::array<std::u16string_view, 6> s_JKSVFolderLocations =
        {u"sdmc:/JKSV/Saves", u"sdmc:/JKSV/ExtData", u"sdmc:/JKSV/SysSave", u"sdmc:/JKSV/Boss", u"sdmc:/JKSV/Shared", u"sdmc:/JKSV"};

    // These are the new, permanant JKSM folders.
    constexpr std::array<std::u16string_view, 6> s_JKSMFolderLocations = {u"sdmc:/JKSM/User_Saves",
                                                                          u"sdmc:/JKSM/Extra_Data",
                                                                          u"sdmc:/JKSM/System_Saves",
                                                                          u"sdmc:/JKSM/BOSS_Extra_Data",
                                                                          u"sdmc:/JKSM/Shared_Extra_Data",
                                                                          u"sdmc:/JKSM"};

    // This is new and doesn't need to be converted.
    constexpr std::u16string_view CONFIG_FOLDER = u"sdmc:/config/JKSM";


} // namespace

void FS::Initialize(void)
{
    // This loop will update locations for the user.
    for (int i = 0; i < 6; i++)
    {
        if (FsLib::DirectoryExists(s_JKSVFolderLocations[i]) && !FsLib::RenameDirectory(s_JKSVFolderLocations[i], s_JKSMFolderLocations[i]))
        {
            Logger::Log("Error updating folder locations for JKSM update: %s.", FsLib::GetErrorString());
        }
    }

    // This loop will create the others if they don't already exist.
    for (int i = 0; i < 6; i++)
    {
        if (!FsLib::DirectoryExists(s_JKSMFolderLocations[i]) && !FsLib::CreateDirectoriesRecursively(s_JKSMFolderLocations[i]))
        {
            Logger::Log(FsLib::GetErrorString());
        }
    }
}

FsLib::Path FS::GetBasePath(Data::SaveDataType SaveType)
{
    // Make sure it's not out of bounds.
    if (SaveType + 1 >= Data::SaveTypeTotal)
    {
        return FsLib::Path(u"");
    }

    // This needs to be offset by one tp account for the root path in the array.
    return FsLib::Path(s_JKSMFolderLocations[SaveType]);
}
