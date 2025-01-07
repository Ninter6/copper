//
//  pipeline.hpp
//  copper
//
//  Created by Ninter6 on 2024/3/16.
//

#pragma once

#include "core.hpp"
#include "rasterize.hpp"

#include <span>
#include <array>
#include <memory>
#include <bitset>
#include <unordered_map>

namespace cu {

struct Uniform {
    std::unordered_map<std::string, mat4> matrix;
    std::unordered_map<std::string, Texture> textures;
};

// shader functions
using VertexShader = std::function<vec4(const Vertex&, const Uniform&, const Camera&)>;
using FragmentShader = std::function<std::optional<Color>(const Vertex&, const Uniform&, const Camera&)>;

using DepthFunc = std::function<bool(float, float)>;

using BlendFunc = std::function<Color(const Color&, const Color&)>;
constexpr auto default_blend_func = [](const Color& src, const Color& dst) -> Color {
    return lerp(src, dst, dst.a);
};

struct PipelineInitInfo {
    std::shared_ptr<Camera> camera          = nullptr;

    VertexShader vertexShader               = nullptr;
    FragmentShader fragmentShader           = nullptr;

    std::shared_ptr<Uniform> uniform        = nullptr;

    FrameBuffer frame;
    Viewport viewport;

    CullFace cullFace = CullFace::none;

    bool enable_depth_test = false;
    bool enable_depth_write = false;
    DepthFunc depth_func = std::less{};

    bool enable_blend = false;
    BlendFunc blend_func = default_blend_func;
};

class Pipeline {
public:
    Pipeline(const PipelineInitInfo& info);
    virtual ~Pipeline() = default;

    Pipeline(Pipeline&&) = delete;

    virtual void draw_point(const Vertex& point);
    virtual void draw_line(const std::array<Vertex, 2>& vertices);
    virtual void draw_triangle(const std::array<Vertex, 3>& vertices);

    void draw_triangle_line(const std::array<Vertex, 3>& vertices);

    void draw_indexed_point(const VertexArray& array, std::span<const IndexGroup> indices);
    void draw_indexed_line(const VertexArray& array, std::span<const IndexGroup> indices);
    void draw_indexed_triangle(const VertexArray& array, std::span<const IndexGroup> indices);
    void draw_array(const VertexArray& array, std::span<const IndexGroup> indices, Topology topo);
    void draw_array(std::span<const Vertex> array, Topology topo);

    void set_vertex_shader(const VertexShader& vertex_shader);
    void set_fragment_shader(const FragmentShader& fragment_shader);
    void set_camera(std::shared_ptr<Camera> camera);
    void set_uniform(std::shared_ptr<Uniform> uniform);
    void set_cull_face(CullFace face);
    void set_depth_test(bool enable);
    void set_depth_write(bool enable);
    void set_depth_func(const DepthFunc& func);
    void set_blend(bool enable);
    void set_blend_func(const BlendFunc& func);

protected:
    void fragment_shader_callback(const Vertex&);

    void rast_draw_line(const std::array<Vertex, 2>& vertices);
    void rast_draw_triangle(const std::array<Vertex, 3>& vertices);

    [[nodiscard]] virtual bool depth_test(ivec2 pos, float z);
    virtual void blend_color(ivec2 pos, Color& color);
    virtual void set_color(ivec2 pos, const Color& color);
    void call_fragment_shader(ivec2 pos, const Vertex& v);

    [[nodiscard]] bool depth_test_enabled() const;
    [[nodiscard]] bool depth_write_enabled() const;
    [[nodiscard]] bool check_depth(ivec2 pos, float z) const;
    void write_depth(ivec2 pos, float z) const;

private:
    std::shared_ptr<Camera> camera;

    Rasterizer rasterizer;
    VertexShader vertexShader;
    FragmentShader fragmentShader;

    std::shared_ptr<Uniform> uniform;

    FrameBuffer frame;
    Viewport viewport;

    CullFace cullFace;

    bool enableDepthTest;
    bool enableDepthWrite;
    DepthFunc depthFunc;

    bool enableBlend;
    BlendFunc blendFunc;

    float call_vertex_shader(Vertex& v) const;
    static void perspective_division(Vertex& v, float w);
    void viewport_transform(Vertex& v) const;
    [[nodiscard]] bool face_culling(const std::array<Vertex, 3>& v) const;

    [[nodiscard]] bool point_frustum_culling(const Vertex& v, float w) const;
    [[nodiscard]] bool line_frustum_culling(std::array<Vertex, 2>& l);
    void cull_line(Vertex& a, const Vertex& b);
    [[nodiscard]] std::pair<bool, std::optional<std::array<Vertex, 3>>>
    triangle_frustum_culling(std::array<Vertex, 3>& v);

    void init_rasterizer();
};

}
