#pragma once
#include "Data/SaveDataType.hpp"
#include "FsLib.hpp"

namespace FS
{
    // This is here purely for remaining backwards compatible. Current JKSM's FS is entirely FS lib.
    void Initialize(void);
    // This will return the base path depending on save type passed.
    FsLib::Path GetBasePath(Data::SaveDataType SaveType);
} // namespace FS
