#pragma once
#include <string_view>


namespace Asset
{
    namespace Names
    {
        static constexpr std::string_view NOTO_SANS = "NotoSans";
        static constexpr std::string_view NOTO_SANS_THIN = "NotoSansThing";
    } // namespace Names

    namespace Paths
    {
        static constexpr std::string_view NOTO_SANS_PATH = "romfs:/NotoSansJP-ExtraBold.ttf";
        static constexpr std::string_view NOTO_SANS_THIN_PATH = "romfs:/NotoSans-Thin.ttf";
    } // namespace Paths
} // namespace Asset
