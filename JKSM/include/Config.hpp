#pragma once
#include <cstdint>
#include <string_view>

namespace Config
{
    // Loads JKSM's config and reads system's language since it's needed throughout JKSM.
    void Initialize(void);
    // Resets all keys to default value.
    void ResetToDefault(void);
    // Saves the config to JSON
    void Save(void);
    // Returns the value of Key. -1 is returned if the key isn't found.
    int8_t GetByKey(std::string_view Key);
    // Sets the value associated with Key
    void SetByKey(std::string_view Key, uint8_t Value);
    // Returns the language of the system.
    uint8_t GetSystemLanguage(void);
    // These are defined keys to make sure no typos can happen.
    namespace Keys
    {
        constexpr std::string_view TextMode = "TextMode";
        constexpr std::string_view ExportToZip = "ExportToZip";
        constexpr std::string_view ForceEnglish = "ForceEnglish";
    } // namespace Keys
} // namespace Config
