#include "Strings.hpp"

#include "Config.hpp"
#include "JSON.hpp"
#include "logging/logger.hpp"

#include <3ds.h>
#include <cstdio>
#include <map>
#include <unordered_map>

namespace
{
    // This is the map where strings are mapped according to name and index. None of these are going to be anywhere close to
    // 0xFF.
    std::map<std::pair<std::string, uint8_t>, std::string> s_StringMap;
    // This is the map of CFG_Languages and what file they're mapped to.
    std::unordered_map<uint8_t, std::string_view> s_FileMap = {{CFG_LANGUAGE_JP, "JP.json"},
                                                               {CFG_LANGUAGE_EN, "EN.json"},
                                                               {CFG_LANGUAGE_FR, "FR.json"},
                                                               {CFG_LANGUAGE_DE, "DE.json"},
                                                               {CFG_LANGUAGE_IT, "IT.json"},
                                                               {CFG_LANGUAGE_ES, "ES.json"},
                                                               {CFG_LANGUAGE_ZH, "ZH.json"},
                                                               {CFG_LANGUAGE_KO, "KO.json"},
                                                               {CFG_LANGUAGE_NL, "NL.json"},
                                                               {CFG_LANGUAGE_PT, "PT.json"},
                                                               {CFG_LANGUAGE_RU, "RU.json"},
                                                               {CFG_LANGUAGE_TW, "TW.json"}};

} // namespace

// This just does some tests and returns the path to the file we're using.
inline std::string GetFilePath(void)
{
    uint8_t SystemLanguage = Config::GetSystemLanguage();

    // This will grab and append the file name from the FileMap
    std::string FilePath = "romfs:/Strings/" + std::string(s_FileMap[SystemLanguage].data());

    // This is to test to make sure the file exists. If we can't open it, just return the default English file.
    std::FILE *TestFile = std::fopen(FilePath.c_str(), "r");
    if (!TestFile) { return std::string("romfs:/Strings/" + std::string(s_FileMap[CFG_LANGUAGE_EN].data())); }
    // Close it and return the path generated earlier.
    std::fclose(TestFile);
    return FilePath;
}

void Strings::Intialize(void)
{
    // I'm being lazy with this.
    std::string FilePath = GetFilePath();

    // This allows me to wrap json-c objects in a smart pointer so I don't have to worry about memory leaks.
    JSON::Object StringFile = JSON::NewObject(json_object_from_file, FilePath.c_str());
    if (!StringFile)
    {
        logger::log("Error opening UI strings for reading!");
        return;
    }

    json_object_iterator CurrentObject = json_object_iter_begin(StringFile.get());
    json_object_iterator EndObject     = json_object_iter_end(StringFile.get());
    while (!json_object_iter_equal(&CurrentObject, &EndObject))
    {
        const char *ObjectName   = json_object_iter_peek_name(&CurrentObject);
        json_object *StringArray = json_object_iter_peek_value(&CurrentObject);

        size_t ArrayLength = json_object_array_length(StringArray);
        for (size_t i = 0; i < ArrayLength; i++)
        {
            json_object *CurrentArrayElement = json_object_array_get_idx(StringArray, i);

            s_StringMap[std::make_pair(ObjectName, i)] = json_object_get_string(CurrentArrayElement);
        }
        json_object_iter_next(&CurrentObject);
    }
}

const char *Strings::GetStringByName(std::string_view StringName, uint8_t Index)
{
    auto FindString = s_StringMap.find(std::make_pair(StringName.data(), Index));
    if (FindString == s_StringMap.end()) { return nullptr; }
    return FindString->second.c_str();
}
