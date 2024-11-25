#pragma once

namespace Data
{
    typedef enum
    {
        SaveTypeUser,
        SaveTypeExtData,
        SaveTypeSystem,
        SaveTypeBOSS,
        SaveTypeSharedExtData, // This is kind of faked and tacked on to the data vector at the end.
        SaveTypeTotal,
    } SaveDataType;
}
