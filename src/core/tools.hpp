//
//  tools.hpp
//  copper
//
//  Created by Ninter6 on 2024/3/16.
//

#pragma once

#include "math_helper.h"

#include <chrono>

namespace cu {

enum class ColorFeature {
    R, G, B, A,
    SUM, LUM, GRS
};

float GetColorFeatureValue(const Color& color, ColorFeature feature);

struct FLatch {
    using tp = std::chrono::high_resolution_clock::time_point;
    using dt = std::chrono::high_resolution_clock::duration;

    FLatch(const dt& time);
    ~FLatch();

    tp end;
};

}
