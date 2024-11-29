#pragma once
#include <string_view>

namespace Asset
{
    namespace Names
    {
        static constexpr std::string_view NOTO_SANS = "NotoSans";
        static constexpr std::string_view DIALOG_BOX = "DialogBoxCorners";
        static constexpr std::string_view BOUNDING_CORNERS = "BoundingBoxCorners";
    } // namespace Names

    namespace Paths
    {
        static constexpr std::string_view NOTO_SANS_PATH = "romfs:/NotoSansJP-ExtraBold.ttf";
        static constexpr std::string_view DIALOG_BOX_PATH = "romfs:/DialogCorners.png";
        static constexpr std::string_view BOUNDING_CORNERS_PATH = "romfs:/BoundingCorners.png";
    } // namespace Paths
} // namespace Asset
