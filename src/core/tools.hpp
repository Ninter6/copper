//
//  tools.hpp
//  copper
//
//  Created by Ninter6 on 2024/3/16.
//

#pragma once

#include "math_helper.h"

namespace cu {

enum class ColorFeature {
    R, G, B, A,
    LUM, SUM
};

float GetColorFeatureValue(const Color& color, ColorFeature feature);


}
