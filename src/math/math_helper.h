//
//  math_helper.h
//  copper
//
//  Created by Ninter6 on 2024/3/16.
//

#pragma once

#include "mathpls.h"

#include <cstdint>
#include <algorithm>

namespace cu {

using Extent = mathpls::ivec2;

using Color = mathpls::vec4;
struct ColorU32 : mathpls::vec<uint8_t, 4> {
    using vec::vec;
    ColorU32(const Color& color) : vec(mathpls::clamp(color, 0.f, 1.f) * 255.f) {}
    explicit operator uint32_t() const {return std::bit_cast<uint32_t>(*this);}
};

}
