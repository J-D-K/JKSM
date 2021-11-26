#include <string>
#include <unordered_map>

#include "cfg.h"

std::unordered_map<std::string, bool> cfg::config;

void cfg::initToDefault()
{
    cfg::config["zip"] = false;
}

void cfg::load()
{
    FILE *cfgIn = fopen("/JKSV/cfg.bin", "rb");
    if(cfgIn)
    {
        bool getBool = false;
        fread(&getBool, sizeof(bool), 1, cfgIn);
        cfg::config["zip"] = getBool;

        fclose(cfgIn);
    }
}

void cfg::save()
{
    FILE *cfgOut = fopen("/JKSV/cfg.bin", "wb");
    if(cfgOut)
    {
        fwrite(&cfg::config["zip"], sizeof(bool), 1, cfgOut);
        fclose(cfgOut);
    }
}
