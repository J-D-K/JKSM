#pragma once
#include <string>

namespace UI
{
    namespace Strings
    { // Loads external strings from file in romfs.
        void Intialize(void);
        // Loads a string according to the StringName provided. Returns NULL if string name isn't found.
        const char *GetStringByName(std::string_view StringName, uint8_t Index);
        // This is to ensure no typos and make stuff easier.
        namespace Names
        {
            static constexpr std::string_view DataLoadingText = "DataLoadingText";
            static constexpr std::string_view StateName = "StateName";
            static constexpr std::string_view StateInformation = "StateInformation";
            static constexpr std::string_view MediaType = "MediaType";
            static constexpr std::string_view SettingsDescription = "SettingsDescription";
            static constexpr std::string_view SettingsMenu = "SettingsMenu";
            static constexpr std::string_view SettingsDecriptions = "SettingsDescriptions";
        } // namespace Names
    } // namespace Strings
} // namespace UI
