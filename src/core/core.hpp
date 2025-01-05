//
//  core.hpp
//  copper
//
//  Created by Ninter6 on 2024/3/16.
//

#pragma once

#include <span>
#include <array>
#include <memory>
#include <vector>
#include <cstdint>
#include <optional>

#include "mathpls.h"
#include "math_helper.h"
#include "calcu.hpp"
#include "tools.hpp"

namespace cu {

using namespace::mathpls; // directly introducing mathematical libraries

struct Vertex {
    vec3 pos{};
    Attribute attr{};

    void rhw_init() {
        pos.z = 1.f / pos.z;
        attr *= pos.z;
    }

    [[nodiscard]] Attribute get_attr() const {
        return attr * (1.f / pos.z);
    }

    Vertex& operator+=(const Vertex&);
    Vertex& operator-=(const Vertex&);
    Vertex operator+(const Vertex&) const;
    Vertex operator-(const Vertex&) const;
    Vertex& operator*=(float);
    Vertex& operator/=(float);
    Vertex operator*(float) const;
    Vertex operator/(float) const;
};

struct IndexGroup {
    uint32_t pos;
    std::optional<uint32_t> nor;
    std::optional<uint32_t> uv;
    std::optional<uint32_t> col;
};

struct VertexArray {
    std::vector<vec3> positions;
    std::vector<vec3> normals;
    std::vector<vec2> uvs;
    std::vector<vec4> colors;

    [[nodiscard]] Vertex get(const IndexGroup&) const;

    [[nodiscard]] std::vector<Vertex> getVertices(std::span<IndexGroup> indices) const;
    [[nodiscard]] std::vector<std::array<Vertex, 2>> getLines(std::span<IndexGroup> indices) const;
    [[nodiscard]] std::vector<std::array<Vertex, 3>> getTriangles(std::span<IndexGroup> indices) const;
};

struct Image {
    virtual ~Image() = default;
    [[nodiscard]] virtual Image* clone() const = 0;
    
    [[nodiscard]] virtual Extent size() const = 0;
    [[nodiscard]] virtual Color get(uivec2 pos) const = 0;
    virtual void set(uivec2 pos, const Color& color) = 0;
    virtual void clear(const Color& clear_color) = 0;
};

struct ImageR8 : Image {
    explicit ImageR8(Extent size, ColorFeature mode = ColorFeature::R);
    ~ImageR8() override;
    [[nodiscard]] Image* clone() const override;

    ImageR8(ImageR8&&) = delete;
    
    [[nodiscard]] Extent size() const override;
    [[nodiscard]] Color get(uivec2 pos) const override;
    void set(uivec2 pos, const Color& color) override;
    void clear(const Color& clear_color) override;
    
    Extent size_;
    uint8_t* data_;
    ColorFeature mode_;
};

struct ImageRGBA8 : Image {
    explicit ImageRGBA8(Extent size);
    ~ImageRGBA8() override;
    [[nodiscard]] Image* clone() const override;

    ImageRGBA8(ImageRGBA8&&) = delete;

    [[nodiscard]] Extent size() const override;
    [[nodiscard]] Color get(uivec2 pos) const override;
    void set(uivec2 pos, const Color& color) override;
    void clear(const Color& clear_color) override;
    
    Extent size_;
    ColorU32* data_;
};

struct Sampler {
    virtual ~Sampler() = default;
    [[nodiscard]] virtual Color get(const Image& image, const vec2& uv) const = 0;
};

struct NearestSampler : Sampler {
    [[nodiscard]] Color get(const Image& image, const vec2& uv) const override;
};

struct LinearSampler : Sampler {
    [[nodiscard]] Color get(const Image& image, const vec2& uv) const override;
};

struct FrameBuffer {
    std::shared_ptr<Image> color_image = nullptr;
    std::shared_ptr<Image> depth_image = nullptr;
};

struct Frustum {
    Frustum() = default;
    Frustum(float near, float aspect, float fovy);

    float near{};
    float aspect{};
    float fovy{};
    mat4 mat{};
};

struct Camera {
    Camera() = default;
    Camera(const Frustum& frustum, const vec3& pos);

    Frustum frustum{};
    vec3 position{};
    vec3 forward{0.f, 0.f, -1.f};
    vec3 up{0.f, 1.f, 0.f};

    [[nodiscard]] mat4 proj_view() const;
};

struct Viewport {
    int x, y;
    int w, h;
    [[nodiscard]] vec2 translate(const vec2&) const;
};

enum class Topology {
    point,
    line,
    triangle
};

enum class CullFace {
    none,
    clockwise,
    anticlockwise,
    both
};

struct Texture {
    Texture() = default;
    Texture(Image*, Sampler*);

    [[nodiscard]] Color get(const vec2& uv) const;

    Image* image = nullptr;
    Sampler* sampler = nullptr;
};

}
