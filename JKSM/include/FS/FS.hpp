#pragma once
#include "Data/SaveDataType.hpp"
#include "fslib.hpp"

#include <array>
#include <string_view>

namespace FS
{
    // This is here purely for remaining backwards compatible. Current JKSM's FS is entirely FsLib.
    void Initialize(void);
    // This will return the base path depending on save type passed.
    fslib::Path GetBasePath(Data::SaveDataType SaveType);
    // Attempts to delete secure value for unique ID passed.
    bool DeleteSecureValue(uint32_t UniqueID);
} // namespace FS
