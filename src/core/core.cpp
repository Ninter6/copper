//
//  core.cpp
//  copper
//
//  Created by Ninter6 on 2024/3/16.
//

#include "core.hpp"

#include <cstring>

namespace cu {

ImageR8::ImageR8(Extent size, ColorFeature mode)
: size_(size), mode_(mode) {
    data_ = new uint8_t[size.x * size.y]{};
}

ImageR8::~ImageR8() {
    delete[] data_;
}

Extent ImageR8::size() const {
    return size_;
}

vec4 ImageR8::get(uivec2 pos) const {
    return vec4(*(data_ + pos.x + pos.y * size_.x) / 255.f, 0, 0, 0);
}

void ImageR8::set(uivec2 pos, const vec4& color) {
    *(data_ + pos.x + pos.y * size_.x) =
        std::clamp(GetColorFeatureValue(color, mode_), 0.f, 1.f) * 255;
}

void ImageR8::clear(const Color& clear_color) {
    int c = std::clamp(GetColorFeatureValue(clear_color, mode_), 0.f, 1.f) * 255;
    std::memset(data_, c, size_.x * size_.y);
}

Color NearestSampler::get(const Image &image, const vec2 &uv) const {
    auto ext = image.size() - 1;
    return image.get({
        static_cast<unsigned int>(std::round(uv.x * ext.x)),
        static_cast<unsigned int>(std::round(uv.y * ext.y))
    });
}

Color LinearSampler::get(const Image &image, const vec2 &uv) const {
    auto ext = image.size() - 1;
    
    auto x = ext.x * uv.x;
    auto y = ext.y * uv.y;
    
    uint32_t lx = std::floor(x), ux = std::ceil(x);
    uint32_t ly = std::floor(y), uy = std::ceil(y);
    
    float fu = x - lx, fv = y - ly;
    float gu = 1 - fu, gv = 1 - fv;
    
    vec4 K{
        gu * gv,
        fu * gv,
        gu * fv,
        fu * fv
    };
    
    mat4 CM{
        image.get({lx, ly}),
        image.get({ux, ly}),
        image.get({lx, uy}),
        image.get({ux, uy})
    };
    
    return CM * K;
}

mat4 Frustum::projMat() const {
    auto _n = 1.f / near;
    auto _ntan_ = 1.f / tan(fovy/2) * _n;
    return {
        vec4(_ntan_, 0.f, 0.f, 0.f),
        vec4(0.f, _ntan_ * aspect, 0.f, 0.f),
        vec4(0.f, 0.f, 1.f, -_n),
        vec4(0.f, 0.f, 0.f, 0.f),
    };
}

vec3 Camera::toNDC(const vec3& p) const {
    return toNDC(vec4{p, 1.f});
}

vec3 Camera::toNDC(const vec4& p) const {
    auto v = frustum.projMat() * lookAt(position, position + forward, up) * p;
    return vec3(v) / v.w;
}

vec3 Camera::toNDC(const vec4& p, const mat4& modelMat) const {
    auto v = frustum.projMat() * lookAt(position, position + forward, up) * modelMat * p;
    return vec3(v) / v.w;
}

}
