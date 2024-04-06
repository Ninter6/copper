//
//  tools.cpp
//  copper
//
//  Created by Ninter6 on 2024/3/16.
//

#include "tools.hpp"

namespace cu {

float GetColorFeatureValue(const mathpls::vec4& color, ColorFeature feature) {
    switch (feature) {
        case ColorFeature::R:
            return color.r;
        case ColorFeature::G:
            return color.g;
        case ColorFeature::B:
            return color.b;
        case ColorFeature::A:
            return color.a;
        case ColorFeature::LUM:
            return mathpls::dot<float, 3>({0.2126f, 0.7152f, 0.0722f}, color);
        case ColorFeature::SUM:
            return mathpls::dot<float, 3>(1.f, color);
        default:
            return 0;
    }
}

}
