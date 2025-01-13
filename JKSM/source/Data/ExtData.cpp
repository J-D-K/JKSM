#include "Data/ExtData.hpp"

uint32_t Data::ExtDataRedirect(uint64_t TitleID)
{
    uint32_t LowerID = static_cast<uint32_t>(TitleID & 0xFFFFFFFF);
    switch (LowerID)
    {
        // Pokemon Y
        case 0x00055E00:
        {
            return 0x0000055D;
        }
        break;

        // Pokemon OR
        case 0x0011C400:
        {
            return 0x000011C5;
        }
        break;

        // Pokemon Moon
        case 0x001B5100:
        {
            return 0x00001B50;
        }
        break;

        // Fire Emblem Fates Conquest & Special Edition USA/NA
        case 0x00179600:
        case 0x00179800:
        {
            return 0x00001794;
        }
        break;

        // Fire Emblem Conquest Euro
        case 0x00179700:
        case 0x0017A800:
        {
            return 0x00001795;
        }
        break;

        // Fire Emblem If Japan
        case 0x0012DD00:
        case 0x0012DE00:
        {
            return 0x000012DC;
        }
        break;

        default:
        {
            return LowerID >> 8;
        }
        break;
    }
    return 0;
}
