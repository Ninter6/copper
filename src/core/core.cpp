//
//  core.cpp
//  copper
//
//  Created by Ninter6 on 2024/3/16.
//

#include "core.hpp"

#include <cassert>
#include <cstring>
#include <algorithm>

namespace cu {

Vertex& Vertex::operator+=(const Vertex& o) {
    pos += o.pos;
    attr += o.attr;
    return *this;
}
Vertex Vertex::operator+(const Vertex& o) const {
    Vertex r = *this;
    return r += o;
}
Vertex& Vertex::operator-=(const Vertex& o) {
    pos -= o.pos;
    attr -= o.attr;
    return *this;
}
Vertex Vertex::operator-(const Vertex& o) const {
    Vertex r = *this;
    return r -= o;
}
Vertex& Vertex::operator*=(float k) {
    pos *= k;
    attr *= k;
    return *this;
}
Vertex Vertex::operator*(float k) const {
    Vertex r = *this;
    return r *= k;
}
Vertex& Vertex::operator/=(float k) {
    return *this *= (1.f / k);
}
Vertex Vertex::operator/(float k) const {
    return *this * (1.f / k);
}

Vertex VertexArray::get(const IndexGroup& index) const {
    Vertex v;
    v.pos = positions.at(index.pos);

    if (index.nor) v.attr.var.normal = normals.at(*index.nor);
    if (index.uv)       v.attr.var.uv = uvs.at(*index.uv);
    if (index.col) v.attr.var.color = colors.at(*index.col);

    return v;
}

std::vector<Vertex> VertexArray::getVertices(std::span<const IndexGroup> indices) const {
    std::vector<Vertex> v;
    v.reserve(indices.size());
    for (auto&& i : indices) v.push_back(get(i));
    return v;
}

std::vector<std::array<Vertex, 2>> VertexArray::getLines(std::span<const IndexGroup> indices) const {
    std::vector<std::array<Vertex, 2>> v(indices.size() / 2);
    for (size_t i = 0; i < indices.size(); i += 2)
        v.push_back({get(indices[i]), get(indices[i+1])});
    return v;
}

std::vector<std::array<Vertex, 3>> VertexArray::getTriangles(std::span<const IndexGroup> indices) const {
    std::vector<std::array<Vertex, 3>> v(indices.size() / 3);
    for (size_t i = 0; i < indices.size(); i += 3)
        v.push_back({get(indices[i]), get(indices[i+1]), get(indices[i+2])});
    return v;
}

ImageR8::ImageR8(Extent size, ColorFeature mode)
: size_(size), mode_(mode) {
    data_ = new uint8_t[size.x * size.y]{};
}

ImageR8::~ImageR8() {
    delete[] data_;
}

Image* ImageR8::clone() const {
    auto r = new ImageR8{size_, mode_};
    memcpy(r->data_, data_, size_.x * size_.y);
    return r;
}

Extent ImageR8::size() const {
    return size_;
}

vec4 ImageR8::get(uivec2 pos) const {
    return {(float)*(data_ + pos.x + pos.y * size_.x) / 255.f, 0, 0, 0};
}

void ImageR8::set(uivec2 pos, const Color& color) {
    *(data_ + pos.x + pos.y * size_.x) =
        static_cast<uint8_t>(std::clamp(GetColorFeatureValue(color, mode_), 0.f, 1.f) * 255);
}

void ImageR8::clear(const Color& clear_color) {
    int c = static_cast<int>(std::clamp(GetColorFeatureValue(clear_color, mode_), 0.f, 1.f) * 255);
    std::memset(data_, c, size_.x * size_.y);
}

ImageRGBA8::ImageRGBA8(Extent size) : size_(size) {
    data_ = new ColorU32[size_.x * size_.y]{};
}

ImageRGBA8::~ImageRGBA8() {
    delete[] data_;
}

Image* ImageRGBA8::clone() const {
    auto r = new ImageRGBA8{size_};
    memcpy(r->data_, data_, size_.x * size_.y * sizeof(ColorU32));
    return r;
}

Extent ImageRGBA8::size() const {
    return size_;
}

Color ImageRGBA8::get(uivec2 pos) const {
    Color c = *(data_ + pos.x + pos.y * size_.x);
    return c / 255.f;
}

void ImageRGBA8::set(uivec2 pos, const Color& color) {
    *(data_ + pos.x + pos.y * size_.x) = color;
}

void ImageRGBA8::clear(const Color& clear_color) {
    auto c = ColorU32(clear_color);
    std::uninitialized_fill_n(data_, size_.x * size_.y, c);
}

Color NearestSampler::get(const Image &image, const vec2 &uv) const {
    auto ext = image.size() - 1;
    return image.get({
        static_cast<unsigned>(std::round(uv.x * (float)ext.x)),
        static_cast<unsigned>(std::round(uv.y * (float)ext.y))
    });
}

Color LinearSampler::get(const Image &image, const vec2 &uv) const {
    auto ext = image.size() - 1;
    
    auto x = (float)ext.x * uv.x;
    auto y = (float)ext.y * uv.y;
    
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

Frustum::Frustum(float near, float aspect, float fovy) : near(near), aspect(aspect), fovy(fovy) {
    auto a = 1.f / (near * tan(fovy));
    mat = {
        vec4(a, 0.f, 0.f, 0.f),
        vec4(0.f, aspect * a, 0.f, 0.f),
        vec4(0.f, 0.f, 1.f, -1.f / near),
        vec4(0.f, 0.f, 0.f, 0.f)
    };
}

Camera::Camera(const Frustum& f, const vec3& p) : frustum(f), position(p) {}

mat4 Camera::proj_view() const {
    return frustum.mat * lookAt(position, position + forward, up);
}

vec2 Viewport::translate(const vec2& v) const {
    return {((float)x + (v.x + 1.f) * .5f * ((float)w - 1.f)),
            ((float)y + (v.y + 1.f) * .5f * ((float)h - 1.f))};
}

Texture::Texture(Image* image, Sampler* sampler) : image(image), sampler(sampler) {}

Color Texture::get(const vec2& uv) const {
    assert(image && sampler);
    return sampler->get(*image, uv);
}

Color Texture::fetch(const uivec2& pos) const {
    assert(image);
    return image->get(pos);
}

}
