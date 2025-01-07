//
// Created by Ninter6 on 2025/1/5.
//

#pragma once

#include "mathpls.h"

#if defined(__i386__) || defined(__x86_64__)
#   define CU_ENABLED_SIMD
#   include <immintrin.h>
#endif

namespace cu {

using namespace mathpls;

constexpr size_t attr_data_size = 16;

struct Attribute { // NOLINT(*-pro-type-member-init)
    union {
        struct {
            vec3 world_pos;
            vec3 normal;
            vec2 uv;
            vec4 color;
            vec4 other;
        } var;
        float data[attr_data_size]{};

#ifdef CU_ENABLED_SIMD
        static_assert(attr_data_size % 4 == 0);
        __m128 mm_data[attr_data_size / 4];
#endif
    };

    Attribute& operator+=(const Attribute&);
    Attribute& operator-=(const Attribute&);
    Attribute operator+(const Attribute&) const;
    Attribute operator-(const Attribute&) const;
    Attribute& operator*=(float);
    Attribute& operator/=(float);
    Attribute operator*(float) const;
    Attribute operator/(float) const;
};

}