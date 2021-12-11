#pragma once

#include <string>
#include "type.h"

namespace ui
{
    void fldInit(const std::u16string& _path, const std::string& _uploadParent, funcPtr _cb, void *_args);
    void fldRefresh();
    void fldUpdate();
    void fldDraw();
}
