//
//  core.hpp
//  copper
//
//  Created by Ninter6 on 2024/3/16.
//

#pragma once

#include <optional>
#include <vector>
#include <cstdint>
#include <cmath>

#include "mathpls.h"
#include "math_helper.h"
#include "tools.hpp"

namespace cu {

using namespace::mathpls; // directly introducing mathematical libraries

struct Vertex {
    std::optional<vec3> position;
    std::optional<vec3> normal;
    std::optional<vec2> uv;
    std::optional<vec3> color;
};

struct IndexGroup {
    uint32_t position;
    uint32_t normal;
    uint32_t uv;
    uint32_t color;
};

struct VertexBuffer {
    std::vector<vec3> positions;
    std::vector<vec3> normals;
    std::vector<vec2> uvs;
    std::vector<vec3> colors;
    std::vector<IndexGroup> indices;
    
    std::vector<Vertex> getVertices() const;
};

struct Image {
    virtual ~Image() = default;
    
    virtual Extent size() const = 0;
    virtual Color get(uivec2 pos) const = 0;
    virtual void set(uivec2 pos, const Color& color) = 0;
    virtual void clear(const Color& clear_color) = 0;
};

struct ImageR8 : Image {
    ImageR8(Extent size, ColorFeature mode = ColorFeature::R);
    virtual ~ImageR8() override;
    
    virtual Extent size() const override;
    virtual Color get(uivec2 pos) const override;
    virtual void set(uivec2 pos, const Color& color) override;
    virtual void clear(const Color& clear_color) override;
    
    Extent size_;
    uint8_t* data_;
    ColorFeature mode_;
};

struct ImageRGBA32 : Image {
    ImageRGBA32(Extent size);
    virtual ~ImageRGBA32() override;
    
    virtual Extent size() const override;
    virtual Color get(uivec2 pos) const override;
    virtual void set(uivec2 pos, const Color& color) override;
    virtual void clear(const Color& clear_color) override;
    
    Extent size_;
    Color* data_;
};

struct Sampler {
    virtual ~Sampler() = default;
    virtual Color get(const Image& image, const vec2& uv) const = 0;
};

struct NearestSampler : Sampler {
    virtual Color get(const Image& image, const vec2& uv) const override;
};

struct LinearSampler : Sampler {
    virtual Color get(const Image& image, const vec2& uv) const override;
};

struct FrameBuffer {
    Image* color_image = nullptr;
    Image* depth_image = nullptr;
};

struct Frustum {
    float near;
    float fovy;
    float aspect;

    mat4 projMat() const;
};

struct Camera {
    Frustum frustum;
    vec3 position{};
    vec3 forward{0.f, 0.f, -1.f};
    vec3 up{0.f, 1.f, 0.f};

    vec3 toNDC(const vec3& p) const;
    vec3 toNDC(const vec4& p) const;
    vec3 toNDC(const vec4& p, const mat4& modelMat) const;
};

enum class Topology {
    point,
    line,
    triangle
};

}
