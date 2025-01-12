//
// Created by Ninter6 on 2025/1/11.
//

#include "ascii.hpp"

#include <numeric>
#include <algorithm>

namespace cu {

std::string AsciiFactory::process(const Image& img) {
    if (enabled_noise)
        return filter_pixels(cu::fetch_pixels_noised(img, noise_factor));
    else
        return filter_pixels(cu::fetch_pixels(img));
}

std::string AsciiFactory::process(const Texture& tex, Extent ext) {
    if (enabled_noise)
        return filter_pixels(cu::pick_pixels_noised(tex, ext, noise_factor));
    else
        return filter_pixels(cu::pick_pixels(tex, ext));
}

AsciiFactory& AsciiFactory::set_table(const std::string& tb) {
    table = tb + tb.back();
    return *this;
}

AsciiFactory& AsciiFactory::set_noise_enable(bool enable) {
    enabled_noise = enable;
    return *this;
}

AsciiFactory& AsciiFactory::set_noise_factor(float factor) {
    noise_factor = factor;
    return *this;
}

AsciiFactory& AsciiFactory::set_filter(const std::function<float(const Color&)>& filter) {
    this->filter = filter;
    return *this;
}

AsciiFactory& AsciiFactory::set_converter(const std::function<std::string(char)>& convert) {
    converter = convert;
    return *this;
}

char AsciiFactory::get_char(float v) const {
    const auto l = float(table.length() - 1);
    return table[(size_t)(v * l)];
}

template <class View>
std::string AsciiFactory::filter_pixels(const View& view) const {
    if (filter) {
        auto filtered = view | std::views::transform([this](auto&& col) -> char {
            auto f = filter(col);
            return get_char(std::clamp(f, 0.f, 1.f));
        });
        return convert_pixels(filtered);
    } else {
        auto filtered = view | std::views::transform([this](auto&& col) -> char {
            auto f = GetColorFeatureValue(col, ColorFeature::GRS); // default
            return get_char(std::clamp(std::pow(f, 2.2f), 0.f, 1.f));
        });
        return convert_pixels(filtered);
    }
}

template <class View>
std::string AsciiFactory::convert_pixels(const View& view) const {
    if (converter) {
        auto converted = view | std::views::transform([this](auto&& c) {
            return converter(c);
        });
        return std::accumulate(converted.begin(), converted.end(), std::string());
    } else {
        return std::accumulate(view.begin(), view.end(), std::string());
    }
}

}