#pragma once

namespace Data
{
    typedef enum
    {
        SaveTypeUser,
        SaveTypeExtData,
        SaveTypeSystem,
        SaveTypeBOSS,
        SaveTypeSharedExtData, // Nothing besides the system has this, but it's needed to grab paths.
        SaveTypeTotal,
    } SaveDataType;
}
