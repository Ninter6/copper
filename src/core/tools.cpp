//
//  tools.cpp
//  copper
//
//  Created by Ninter6 on 2024/3/16.
//

#include "tools.hpp"

#include <thread>

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
        case ColorFeature::SUM:
            return mathpls::dot<float, 3>(1.f, color);
        case ColorFeature::LUM:
            return mathpls::dot<float, 3>({0.2126f, 0.7152f, 0.0722f}, color);
        case ColorFeature::GRS:
            return mathpls::dot<float, 3>({0.299f, 0.587f, 0.114f}, color);
        default:
            return 0;
    }
}

FLatch::FLatch(const cu::FLatch::dt &time)
    : end(std::chrono::high_resolution_clock::now() + time) {}

FLatch::~FLatch() {
    std::this_thread::sleep_until(end);
}

}
