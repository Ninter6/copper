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
#include <unordered_map>

namespace cu {

struct Uniform {
    std::unordered_map<std::string, mat4> matrix;
    std::unordered_map<std::string, Texture> textures;
};

// shader functions
using VertexShader = std::function<vec4(const Vertex&, const Uniform&, const Camera&)>;
using FragmentShader = std::function<Color(const Vertex&, const Uniform&, const Camera&)>;

struct PipelineInitInfo {
    std::shared_ptr<Camera> camera          = nullptr;

    std::shared_ptr<Rasterizer> rasterizer  = nullptr;
    VertexShader vertexShader               = nullptr;
    FragmentShader fragmentShader           = nullptr;

    std::shared_ptr<Uniform> uniform        = nullptr;

    FrameBuffer frame;
    Viewport viewport;

    CullFace cullFace = CullFace::none;
};

class Pipeline {
public:
    Pipeline(const PipelineInitInfo& info);
    ~Pipeline();

    Pipeline(Pipeline&&) = delete;

    void draw_point(const Vertex& point);
    void draw_line(const std::array<Vertex, 2>& vertices);

    void draw_triangle(const std::array<Vertex, 3>& vertices);

    void draw_indexed_point(const VertexArray& array, std::span<IndexGroup> indices);
    void draw_indexed_line(const VertexArray& array, std::span<IndexGroup> indices);
    void draw_indexed_triangle(const VertexArray& array, std::span<IndexGroup> indices);
    void draw_array(const VertexArray& array, std::span<IndexGroup> indices, Topology topo);
    void draw_array(std::span<Vertex> array, Topology topo);

    void set_rasterizer(std::shared_ptr<Rasterizer> rasterizer);
    void set_vertex_shader(const VertexShader& vertex_shader);
    void set_fragment_shader(const FragmentShader& fragment_shader);
    void set_camera(std::shared_ptr<Camera> camera);
    void set_uniform(std::shared_ptr<Uniform> uniform);

private:
    std::shared_ptr<Camera> camera;

    std::shared_ptr<Rasterizer> rasterizer;
    VertexShader vertexShader;
    FragmentShader fragmentShader;

    std::shared_ptr<Uniform> uniform;

    FrameBuffer frame;
    Viewport viewport;

    CullFace cullFace;

    RastBindHandle bind_handle{};

    void call_vertex_shader(Vertex& v);
    static bool point_frustum_culling(const Vertex& v);
    static bool triangle_frustum_culling(const std::array<Vertex, 3>& v);
    void viewport_transform(Vertex& v);
    bool face_culling(const std::array<Vertex, 3>& v);

    void fragment_shader_callback(const Vertex&);

    void bind_rasterizer();
    void unbind_rasterizer();
};

}
