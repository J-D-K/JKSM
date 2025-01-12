#pragma once
#include <string_view>

// This is just the mount point JKSM uses for save data in general.
namespace FS
{
    static constexpr std::u16string_view SAVE_MOUNT = u"save";
    static constexpr std::u16string_view SAVE_ROOT = u"save:/";
} // namespace FS
