#pragma once

#include <string>
#include <unordered_map>

namespace cfg
{
    void initToDefault();

    void load();
    void save();

    extern std::unordered_map<std::string, bool> config;
}