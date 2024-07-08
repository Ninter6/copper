//
//  pipeline.cpp
//  copper
//
//  Created by Ninter6 on 2024/3/16.
//

#include "pipeline.hpp"

#include <algorithm>
#include <utility>

#include "se_tools.h"

namespace cu {

Pipeline::Pipeline(const PipelineInitInfo& info) :
    camera(info.camera),
    rasterizer(info.rasterizer),
    vertexShader(info.vertexShader),
    fragmentShader(info.fragmentShader),
    uniform(info.uniform),
    frame(info.frame),
    viewport(info.viewport),
    cullFace(info.cullFace)
{
    bind_rasterizer();
}

Pipeline::~Pipeline() {
    unbind_rasterizer();
}

void Pipeline::draw_point(const Vertex& point) {
    auto v = point;
    call_vertex_shader(v);

    if (!point_frustum_culling(v)) return; // failed

    viewport_transform(v);
    v.rhw_init();

    fragment_shader_callback(v); // no need to rasterize
}

// [Cohen–Sutherland algorithm](https://en.wikipedia.org/wiki/Cohen–Sutherland_algorithm)
std::optional<std::pair<vec2, vec2>>
line_clip(const vec2 &b, const vec2 &e, const vec2 &min = {-1.f}, const vec2 &max = {1.f}) {
    constexpr uint8_t INSIDE = 0;
    constexpr uint8_t LEFT = 1;
    constexpr uint8_t RIGHT = 2;
    constexpr uint8_t BOTTOM = 4;
    constexpr uint8_t TOP = 8;

    auto out_code = [&](auto &&p) {
        uint8_t code = INSIDE;

        if (p.x < min.x)
            code |= LEFT;
        else if (p.x > max.x)
            code |= RIGHT;
        if (p.y < min.y)
            code |= BOTTOM;
        else if (p.y > max.y)
            code |= TOP;

        return code;
    };

    auto p0 = b, p1 = e;
    auto outcode0 = out_code(p0);
    auto outcode1 = out_code(p1);

    while (true) {
        if (!(outcode0 | outcode1)) {
            return {{p0, p1}};
        } else if (outcode0 & outcode1) {
            return std::nullopt;
        } else {
            float x{}, y{};
            auto outcodeOut = std::max(outcode0, outcode1);

            if (outcodeOut & TOP) {           // point is above the clip window
                x = p0.x + (p1.x - p0.x) * (max.y - p0.y) / (p1.y - p0.y);
                y = max.y;
            } else if (outcodeOut & BOTTOM) { // point is below the clip window
                x = p0.x + (p1.x - p0.x) * (min.y - p0.y) / (p1.y - p0.y);
                y = min.y;
            } else if (outcodeOut & RIGHT) {  // point is to the right of clip window
                y = p0.y + (p1.y - p0.y) * (max.x - p0.x) / (p1.x - p0.x);
                x = max.x;
            } else if (outcodeOut & LEFT) {   // point is to the left of clip window
                y = p0.y + (p1.y - p0.y) * (min.x - p0.x) / (p1.x - p0.x);
                x = min.x;
            }

            if (outcodeOut == outcode0) {
                p0 = {x, y};
                outcode0 = out_code(p0);
            } else {
                p1 = {x, y};
                outcode1 = out_code(p1);
            }
        }
    }
}

void Pipeline::draw_line(const std::array<Vertex, 2>& vertices) {
    auto v = vertices;
    for (auto&& i : v) {
        call_vertex_shader(i);
        i.rhw_init();
    }

    if (auto clipped = line_clip(v[0].pos, v[1].pos)) {
        auto&& [a, b] = *clipped;
        float dy = v[1].pos.y - v[0].pos.y;
        float eya = a.y - v[0].pos.y;
        float eyb = b.y - v[0].pos.y;
        float ta = eya / dy;
        float tb = eyb / dy;
        v[0] = lerp(v[0], v[1], ta);
        v[1] = lerp(v[0], v[1], tb);
    } else return;

    for (auto&& i : v) viewport_transform(i);

    rasterizer->draw_line(v);
}

void Pipeline::draw_triangle(const std::array<Vertex, 3>& vertices) {
    auto v = vertices;
    for (auto&& i : v) call_vertex_shader(i);

    if (!triangle_frustum_culling(v)) return; // failed

    for (auto&& i : v) viewport_transform(i);

    if (!face_culling(v)) return; // failed

    for (auto&& i : v) i.rhw_init();

    rasterizer->draw_triangle(v, viewport);
}


void Pipeline::draw_indexed_point(const VertexArray& array, std::span<IndexGroup> indices) {
    for (auto&& i : array.getVertices(indices))
        draw_point(i);
}

void Pipeline::draw_indexed_line(const VertexArray& array, std::span<IndexGroup> indices) {
    for (auto&& i : array.getLines(indices))
        draw_line(i);
}

void Pipeline::draw_indexed_triangle(const VertexArray& array, std::span<IndexGroup> indices) {
    for (auto&& i : array.getTriangles(indices))
        draw_triangle(i);
}

void Pipeline::draw_array(const VertexArray& array, std::span<IndexGroup> indices, Topology topo) {
    switch (topo) {
        case Topology::point:
            draw_indexed_point(array, indices);
            break;
        case Topology::line:
            draw_indexed_line(array, indices);
            break;
        case Topology::triangle:
            draw_indexed_triangle(array, indices);
            break;
        default:
            break;
    }
}

void Pipeline::set_uniform(std::shared_ptr<Uniform> u) {
    this->uniform = std::move(u);
}

void Pipeline::set_camera(std::shared_ptr<Camera> cam) {
    this->camera = std::move(cam);
}

void Pipeline::set_rasterizer(std::shared_ptr<Rasterizer> rast) {
    unbind_rasterizer();
    this->rasterizer = std::move(rast);
    bind_rasterizer();
}

void Pipeline::set_vertex_shader(const VertexShader& vertex_shader) {
    this->vertexShader = vertex_shader;
}

void Pipeline::set_fragment_shader(const FragmentShader& fragment_shader) {
    this->fragmentShader = fragment_shader;
}

void Pipeline::call_vertex_shader(Vertex& v) {
    assert(camera && uniform && vertexShader);
    auto v1 = vertexShader(v, *uniform, *camera);
    v.pos = {v1.x / v1.w, v1.y / v1.w, v1.z};
}

bool Pipeline::point_frustum_culling(const Vertex& v) {
    constexpr float zmax = 1e4f;
    if (std::abs(v.pos.x) > 1 || std::abs(v.pos.y) > 1 || std::isnan(v.pos.z) || v.pos.z > zmax)
        return false; // failed
    return true; // passed
}

bool Pipeline::triangle_frustum_culling(const std::array<Vertex, 3>& v) {
    return std::all_of(v.begin(), v.end(), [](auto&&i){
        return point_frustum_culling(i);
    });
}

void Pipeline::viewport_transform(Vertex& v) {
    auto [x, y] = viewport.translate(v.pos).asArray;
    v.pos.x = x;
    v.pos.y = y;
}

bool Pipeline::face_culling(const std::array<Vertex, 3>& v) {
    if (cullFace == CullFace::none)
        return true; // passed
    if (cullFace == CullFace::both)
        return false; // failed

    auto n = cross(v[1].pos - v[0].pos, v[2].pos - v[1].pos);
    auto face = n.z > 0 ? CullFace::anticlockwise : CullFace::clockwise;

    return face != cullFace;
}

void Pipeline::fragment_shader_callback(const Vertex& v) {
    assert(camera && uniform && fragmentShader);

    ivec2 pos = vec2{v.pos};

    if (frame.depth_image) {
        float z = 1.f / v.pos.z;
        if (frame.depth_image->get(pos).z > z)
            return; // failed
    } // depth test

    auto color = fragmentShader(v, *uniform, *camera);

    // write color
    frame.color_image->set(pos, color);
}

void Pipeline::bind_rasterizer() {
    bind_handle = rasterizer->bind_pipline([this](auto&&v){fragment_shader_callback(v);});
}

void Pipeline::unbind_rasterizer() {
    rasterizer->unbind_pipeline(bind_handle);
}

}
