#pragma once
#include "Data/TitleData.hpp"
#include <vector>

namespace Data
{
    // Loads data from cache or system if cache isn't present.
    bool Initialize(void);
    // This gets a vector of the titles with the corresponding save type.
    void GetTitlesWithType(SaveDataType SaveType, std::vector<Data::TitleData *> &Out);
} // namespace Data
