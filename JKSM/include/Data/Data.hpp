#pragma once
#include "Data/TitleData.hpp"
#include "System/ProgressTask.hpp"
#include <vector>

namespace Data
{
    // This is threaded so we can update the screen with whats going on.
    void Initialize(System::ProgressTask *Task);
    // Checks if a gamecard was inserted or removed. Returns true if one has been.
    bool GameCardUpdateCheck(void);
    // This gets a vector of the titles with the corresponding save type.
    void GetTitlesWithType(SaveDataType SaveType, std::vector<Data::TitleData *> &Out);
} // namespace Data
