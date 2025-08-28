#pragma once
#include <string>

namespace Strings
{ // Loads external strings from file in romfs.
    void Intialize();
    // Loads a string according to the StringName provided. Returns NULL if string name isn't found.
    const char *GetStringByName(std::string_view StringName, uint8_t Index);
    // This is to ensure no typos and make stuff easier.
    namespace Names
    {
        static constexpr std::string_view LR                       = "LR";
        static constexpr std::string_view DataLoadingText          = "DataLoadingText";
        static constexpr std::string_view StateName                = "StateName";
        static constexpr std::string_view StateInformation         = "StateInformation";
        static constexpr std::string_view MediaType                = "MediaType";
        static constexpr std::string_view SettingsDescription      = "SettingsDescription";
        static constexpr std::string_view SettingsMenu             = "SettingsMenu";
        static constexpr std::string_view SettingsDescriptions     = "SettingsDescriptions";
        static constexpr std::string_view FolderMenuNew            = "FolderMenuNew";
        static constexpr std::string_view BackupMenuCurrentBackups = "BackupMenuCurrentBackups";
        static constexpr std::string_view CopyingFile              = "CopyingFile";
        static constexpr std::string_view AddingToZip              = "AddingToZip";
        static constexpr std::string_view DeletingBackup           = "DeletingBackup";
        static constexpr std::string_view KeyboardButtons          = "KeyboardButtons";
        static constexpr std::string_view YesNo                    = "YesNo";
        static constexpr std::string_view HoldingText              = "HoldingText";
        static constexpr std::string_view OK                       = "OK";
        static constexpr std::string_view BackupMenuConfirmations  = "BackupMenuConfirmations";
        static constexpr std::string_view TitleOptions             = "TitleOptions";
        static constexpr std::string_view TitleOptionConfirmations = "TitleOptionConfirmations";
        static constexpr std::string_view TitleOptionTaskStatus    = "TitleOptionTaskStatus";
        static constexpr std::string_view TitleOptionMessages      = "TitleOptionMessages";
        static constexpr std::string_view PlayCoinsMessages        = "PlayCoinsMessages";
    } // namespace Names
} // namespace Strings
