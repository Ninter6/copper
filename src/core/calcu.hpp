//
// Created by Ninter6 on 2025/1/5.
//

#pragma once

#include "mathpls.h"

namespace cu {

using namespace mathpls;

struct Attribute { // NOLINT(*-pro-type-member-init)
    union {
        struct {
            vec3 world_pos;
            vec3 normal;
            vec2 uv;
            vec4 color;
            vec4 other;
        } var;
        float data[16]{};
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