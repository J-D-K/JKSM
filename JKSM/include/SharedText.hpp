#pragma once
#include "Data/SaveDataType.hpp"
#include <array>
#include <string_view>

// Both types of selection menu need these strings. Still trying to figure out where else the can even go?
namespace SharedText
{
    static constexpr std::array<std::string_view, Data::SaveTypeTotal> SaveTypeStrings = {"User Saves",
                                                                                          "ExtData Saves",
                                                                                          "System Saves",
                                                                                          "BOSS ExtData Saves",
                                                                                          "Shared ExtData"};

    static constexpr std::array<std::string_view, 3> MediaTypeStrings = {"NAND", "SD Card", "Game Card"};
} // namespace SharedText
