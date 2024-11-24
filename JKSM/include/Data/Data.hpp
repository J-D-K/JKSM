#pragma once
#include "Data/TitleData.hpp"
#include <vector>

namespace Data
{
    bool Initialize(void);
    // Checks if a gamecard was inserted or removed. Returns true if one has been.
    bool GameCardUpdateCheck(void);
    // This gets a vector of the titles with the corresponding save type.
    void GetTitlesWithType(SaveDataType SaveType, std::vector<Data::TitleData *> &Out);
} // namespace Data
