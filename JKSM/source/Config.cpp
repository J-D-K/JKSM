#include "Config.hpp"
#include "FsLib.hpp"
#include "JSON.hpp"
#include "Logger.hpp"
#include <cstring>
#include <string>
#include <unordered_map>

namespace
{
    // Map of config values. This needs to be string instead of string_view.
    std::unordered_map<std::string, uint8_t> s_ConfigMap;
    // Path where config should be.
    constexpr std::string_view CONFIG_FILE_PATH = "sdmc:/config/JKSM/JKSM.json";
    // The system language. I decided to put it here since it's part of CFGU. I also don't think calling it over and over is a good idea.
    uint8_t s_SystemLanguage = 0;
} // namespace

void Config::Initialize(void)
{
    // Read system language first.
    Result CFGUError = CFGU_GetSystemLanguage(&s_SystemLanguage);
    if (R_FAILED(CFGUError))
    {
        Logger::Log("Error getting system language. Defaulting to English.");
        s_SystemLanguage = CFG_LANGUAGE_EN;
    }

    JSON::Object ConfigJSON = JSON::NewObject(json_object_from_file, CONFIG_FILE_PATH.data());
    if (!ConfigJSON)
    {
        Logger::Log("Error opening config for reading.");
        Config::ResetToDefault();
        return;
    }

    json_object_iterator CurrentConfigValue = json_object_iter_begin(ConfigJSON.get());
    json_object_iterator ConfigEnd = json_object_iter_end(ConfigJSON.get());
    while (!json_object_iter_equal(&CurrentConfigValue, &ConfigEnd))
    {
        const char *ObjectName = json_object_iter_peek_name(&CurrentConfigValue);
        json_object *ObjectValue = json_object_iter_peek_value(&CurrentConfigValue);

        s_ConfigMap[ObjectName] = json_object_get_uint64(ObjectValue) & 0xFF;

        json_object_iter_next(&CurrentConfigValue);
    }
}

void Config::ResetToDefault(void)
{
    s_ConfigMap[Config::Keys::TextMode.data()] = 0;
    s_ConfigMap[Config::Keys::ExportToZip.data()] = 1;
    s_ConfigMap[Config::Keys::ForceEnglish.data()] = 0;

    Config::Save();
}

void Config::Save(void)
{
    JSON::Object ConfigJSON = JSON::NewObject(json_object_new_object);
    if (!ConfigJSON)
    {
        Logger::Log("Error allocating JSON::Object for config writing.");
        return;
    }

    for (auto &[Key, Value] : s_ConfigMap)
    {
        json_object *ConfigValue = json_object_new_uint64(Value);
        json_object_object_add(ConfigJSON.get(), Key.c_str(), ConfigValue);
    }

    FsLib::File JSONOut(u"sdmc:/config/JKSM/JKSM.json", FS_OPEN_CREATE | FS_OPEN_WRITE);
    if (!JSONOut.IsOpen())
    {
        Logger::Log("Error opening config file for writing: %s", FsLib::GetErrorString());
        return;
    }
    JSONOut << json_object_get_string(ConfigJSON.get());
}

int8_t Config::GetByKey(std::string_view Key)
{
    if (s_ConfigMap.find(Key.data()) == s_ConfigMap.end())
    {
        return -1;
    }
    return s_ConfigMap.at(Key.data());
}

uint8_t Config::GetSystemLanguage(void)
{
    if (Config::GetByKey(Config::Keys::ForceEnglish))
    {
        return CFG_LANGUAGE_EN;
    }
    return s_SystemLanguage;
}
