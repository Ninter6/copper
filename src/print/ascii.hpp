//
// Created by Ninter6 on 2025/1/11.
//

#pragma once

#include <ranges>
#include <functional>

#include "core.hpp"
#include "blue_noise.h"

namespace cu {

inline auto fetch_pixels(const Image& img) {
    auto [w, h] = img.size().asArray;
    return std::views::iota(0, w*h) | std::views::transform([&img, w](int i) {
        uivec2 pos(i % w, i / w);
        return img.get(pos);
    });
}

inline auto fetch_pixels_noised(const Image& img, float k = .5f) {
    auto [w, h] = img.size().asArray;
    return std::views::iota(0, w*h) | std::views::transform([&img, w, k](int i) {
        uivec2 pos(i % w, i / w);
        auto n = (float)get_blue_noise(pos.x, pos.y) / 255.f;
        return img.get(pos) + k * n;
    });
}

inline auto pick_pixels(const Texture& img, Extent size) {
    auto [w, h] = size.asArray;
    float rw = 1.f / w, rh = 1.f / h;
    return std::views::iota(0, w*h) | std::views::transform([=](int i) {
        auto e = (float)i * rw;
        auto f = floor(e);
        vec2 uv(e - f, f * rh);
        return img.get(uv);
    });
}

inline auto pick_pixels_noised(const Texture& img, Extent size, float k = .5f) {
    auto [w, h] = size.asArray;
    float rw = 1.f / w, rh = 1.f / h;
    return std::views::iota(0, w*h) | std::views::transform([=](int i) {
        auto e = (float)i * rw;
        auto f = floor(e);
        vec2 uv(e - f, f * rh);
        auto n = get_blue_noise(uv.x * (w-1), uv.y * (h-1)) / 255.f;
        return img.get(uv) + k * n;
    });
}

struct AsciiFactory {
    AsciiFactory() = default;

    std::string process(const Image& img);
    std::string process(const Texture& tex, Extent ext);

    AsciiFactory& set_table(const std::string& table);
    AsciiFactory& set_noise_enable(bool enable);
    AsciiFactory& set_noise_factor(float factor);
    AsciiFactory& set_filter(const std::function<float(const Color&)>& filter);
    AsciiFactory& set_converter(const std::function<std::string(char)>& convert);

    std::string table = " .:++";
    bool enabled_noise = false;
    float noise_factor = .5f;
    std::function<float(const Color&)> filter = nullptr;
    std::function<std::string(char)> converter = nullptr;

private:

    [[nodiscard]] char get_char(float v) const;

    template <class View>
    std::string filter_pixels(const View& view) const;

    template <class View>
    std::string convert_pixels(const View& view) const;
};

}
