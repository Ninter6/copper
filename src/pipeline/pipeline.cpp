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
    vertexShader(info.vertexShader),
    fragmentShader(info.fragmentShader),
    uniform(info.uniform),
    frame(info.frame),
    viewport(info.viewport),
    cullFace(info.cullFace),
    enableDepthTest(info.enable_depth_test),
    enableDepthWrite(info.enable_depth_write),
    depthFunc(info.depth_func),
    enableBlend(info.enable_blend),
    blendFunc(info.blend_func)
{
    init_rasterizer();
}

void Pipeline::draw_point(const Vertex& point) {
    auto v = point;

    auto w = call_vertex_shader(v);
    if (!point_frustum_culling(v, w)) return; // failed

    perspective_division(v, w);
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
    for (auto&& i : v) i.attr.var.other[0] = call_vertex_shader(i);

    if (!line_frustum_culling(v)) return;

    for (auto&& i : v) {
        perspective_division(i, i.attr.var.other[0]);
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

    rast_draw_line(v);
}

void Pipeline::draw_triangle(const std::array<Vertex, 3>& vertices) {
    auto v = vertices;
    for (auto&& i : v) i.attr.var.other[0] = call_vertex_shader(i);

    auto [succ, v2] = triangle_frustum_culling(v);
    if (!succ) return; // failed

    auto next = [this](auto&& v) {
        for (auto&& i : v) {
            perspective_division(i, i.attr.var.other[0]);
            viewport_transform(i);
        }

        if (!face_culling(v)) return; // failed

        for (auto&& i : v) i.rhw_init();

        rast_draw_triangle(v);
    };
    next(v);
    if (v2) next(*v2);
}

void Pipeline::draw_triangle_line(const std::array<Vertex, 3>& v) {
    draw_line({v[0], v[1]});
    draw_line({v[1], v[2]});
    draw_line({v[2], v[0]});
}

void Pipeline::draw_indexed_point(const VertexArray& array, std::span<const IndexGroup> indices) {
    for (auto&& i : array.getVertices(indices))
        draw_point(i);
}

void Pipeline::draw_indexed_line(const VertexArray& array, std::span<const IndexGroup> indices) {
    for (auto&& i : array.getLines(indices))
        draw_line(i);
}

void Pipeline::draw_indexed_triangle(const VertexArray& array, std::span<const IndexGroup> indices) {
    for (auto&& i : array.getTriangles(indices))
        draw_triangle(i);
}

void Pipeline::draw_array(const VertexArray& array, std::span<const IndexGroup> indices, Topology topo) {
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
            draw_array(array.getVertices(indices), topo);
            break;
    }
}

void Pipeline::draw_array(std::span<const Vertex> array, Topology topo) {
    switch (topo) {
        case Topology::point:
            for (const auto& i : array)
                draw_point(i);
            break;
        case Topology::line:
            for (int i = 1; i < array.size(); i += 2)
                draw_line({array[i - 1], array[i]});
            break;
        case Topology::triangle:
            for (int i = 2; i < array.size(); i += 3)
                draw_triangle({array[i - 2], array[i - 1], array[i]});
            break;
        case Topology::triangle_fan:
            for (int i = 2; i < array.size(); i += 3)
                draw_triangle({array[0], array[i - 1], array[i]});
            break;
        case Topology::triangle_line:
            for (int i = 2; i < array.size(); i += 3)
                draw_triangle_line({array[i - 2], array[i - 1], array[i]});
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

void Pipeline::set_vertex_shader(const VertexShader& vertex_shader) {
    this->vertexShader = vertex_shader;
}

void Pipeline::set_fragment_shader(const FragmentShader& fragment_shader) {
    this->fragmentShader = fragment_shader;
}

void Pipeline::set_cull_face(CullFace face) {
    cullFace = face;
}

void Pipeline::set_depth_test(bool enable) {
    enableBlend = enable;
}

void Pipeline::set_depth_write(bool enable) {
    enableDepthWrite = enable;
}

void Pipeline::set_depth_func(const DepthFunc& func) {
    depthFunc = func;
}

void Pipeline::set_blend(bool enable) {
    enableBlend = enable;
}

void Pipeline::set_blend_func(const BlendFunc& func) {
    blendFunc = func;
}

float Pipeline::call_vertex_shader(Vertex& v) const {
    assert(camera && uniform && vertexShader);
    const auto v1 = vertexShader(v, *uniform, *camera);
    v.pos = {v1.x, v1.y, v1.z};
    return v1.w;
}

void Pipeline::perspective_division(Vertex& v, float w) {
    v.pos.x /= w;
    v.pos.y /= w;
}

bool Pipeline::point_frustum_culling(const Vertex& p, float w) const {
    constexpr float zmax = -1e4f;
    if (p.pos.z >= -camera->frustum.near || p.pos.z < zmax) return false; // failed
    if (std::abs(p.pos.x) > w || std::abs(p.pos.y) > w) return false; // failed
    return true; // passed
}

bool Pipeline::line_frustum_culling(std::array<Vertex, 2>& l) {
    bool a[2]{};
    for (int i = 0; i < 2; i++)
        if (l[i].pos.z < -camera->frustum.near)
            a[i] = true;
    if (a[0] && a[1]) return true;
    if (!a[0] && !a[1]) return false;
    if (a[1]) cull_line(l[0], l[1]);
    else cull_line(l[1], l[0]);
    return true;
}

void Pipeline::cull_line(Vertex& a, const Vertex& b) {
    auto t = (a.pos.z + camera->frustum.near) / (a.pos.z - b.pos.z);
    a *= 1-t;
    a += b*t;
}

bool sphere_plane(const std::array<Vertex, 3>& v, float n) {
    const auto& A = v[0].pos;
    const auto& B = v[1].pos;
    const auto& C = v[2].pos;

    auto a = distance_quared(B, C),
         b = distance_quared(A, C),
         c = distance_quared(A, B);

    auto r2n = n*n + 1;
    vec3 cnt;
    if (a+b > c && b+c > a && c+a > b) {
        auto d21 = B-A;
        auto d31 = C-A;
        auto m23xy = d21.x*d31.y - d21.y*d31.x;
        auto m23yz = d21.y*d31.z - d21.z*d31.y;
        auto m23xz = d21.z*d31.x - d21.x*d31.z;
        if (m23yz*m23yz < 1e-7f)
            cnt = (A+B+C)/3.f;
        else {
            auto f21 = (B.length_squared() - A.length_squared())*.5f;
            auto f31 = (C.length_squared() - A.length_squared())*.5f;
            auto f23y = f31*d21.y - f21*d31.y;
            auto f23z = f21*d31.z - f31*d21.z;
            cnt.x = (m23yz * (A.x*m23yz + A.y*m23xz + A.z*m23xy) - m23xy*f23y - m23xz*f23z) / (m23xy*m23xy + m23yz*m23yz + m23xz*m23xz);
            cnt.y = (f23z + m23xz*cnt.x) / m23yz;
            cnt.z = (f23y + m23xy*cnt.x) / m23yz;
        }
        r2n *= a*b*c / (4*b*c - (b+c-a)*(b+c-a));
    } else {
        auto mx = argsort(vec3{a, b, c})[0];
        switch (mx) {
            case 0: cnt = (B+C)*.5f; r2n *= a*.25f; break;
            case 1: cnt = (A+C)*.5f; r2n *= b*.25f; break;
            case 2: cnt = (A+B)*.5f; r2n *= c*.25f; break;
            default: assert(false);
        }
    }

    const vec3 pn[] = {
        { 0,-n,-1},
        { 0, n,-1},
        {-n, 0,-1},
        { n, 0,-1}
    };
    return std::all_of(std::begin(pn), std::end(pn), [&](auto&& p) {
        auto d = dot(p, cnt);
        return d >= 0 || d*d < r2n;
    });
}

std::pair<bool, std::optional<std::array<Vertex, 3>>>
Pipeline::triangle_frustum_culling(std::array<Vertex, 3>& v) {
    if (!sphere_plane(v, camera->frustum.near))
        return {false, {}};

    int f = -1, p = -1, s = 0;
    for (int i = 0; i < 3; i++)
        if (v[i].pos.z < -camera->frustum.near) {
            p = i;
            s++;
        } else f = i;
    switch (s) {
        case 0: return {false, {}};
        case 1:
            cull_line(v[(p+1)%3], v[p]);
            cull_line(v[(p+2)%3], v[p]);
            return {true, {}};
        case 2: {
            auto t = v[f];
            cull_line(v[f], v[(f+1)%3]);
            cull_line(t, v[(f+2)%3]);
            return {true, std::array{t, v[f], v[(f+2)%3]}};
        }
        case 3: return {true, {}};
        default: assert(false);
    }
}

void Pipeline::viewport_transform(Vertex& v) const {
    auto [x, y] = viewport.translate(v.pos).asArray;
    v.pos.x = x;
    v.pos.y = y;
}

bool Pipeline::face_culling(const std::array<Vertex, 3>& v) const {
    if (cullFace == CullFace::none)
        return true; // passed
    if (cullFace == CullFace::both)
        return false; // failed

    auto n = cross(v[1].pos - v[0].pos, v[2].pos - v[1].pos);
    auto face = n.z < 0 ? CullFace::anticlockwise : CullFace::clockwise;

    return face != cullFace;
}

void Pipeline::fragment_shader_callback(const Vertex& v) {
    assert(camera && uniform && fragmentShader);

    ivec2 pos = {(int)v.pos.x, (int)v.pos.y};

    if (depth_test(pos, v.pos.z))
        call_fragment_shader(pos, v);
}

bool Pipeline::depth_test(ivec2 pos, float z) {
    if (!depth_test_enabled())
        return true; // haven't been enabled

    float depth = 1.f / z;
    if (check_depth(pos, depth))
        return false; // failed

    if (depth_write_enabled())
        write_depth(pos, depth);
    return true; // passed
}

void Pipeline::call_fragment_shader(ivec2 pos, const Vertex& v) {
    // nullopt if the fragment was discarded
    auto color = fragmentShader(v, *uniform, *camera);

    // write color
    if (color) {
        // blend
        if (enableBlend && blendFunc)
            blend_color(pos, *color);

        set_color(pos, *color);
    }
}

void Pipeline::blend_color(ivec2 pos, Color& color) {
    color = blendFunc(frame.color_image->get(pos), color);
}

void Pipeline::set_color(ivec2 pos, const Color& color) {
    frame.color_image->set(pos, color);
}

bool Pipeline::depth_test_enabled() const {
    return enableDepthTest && frame.depth_image;
}

bool Pipeline::depth_write_enabled() const {
    return enableDepthWrite;
}


bool Pipeline::check_depth(ivec2 pos, float z) const {
    return depthFunc(z, frame.depth_image->get(pos).z);
}

void Pipeline::write_depth(ivec2 pos, float z) const {
    frame.depth_image->set(pos, z);
}

void Pipeline::rast_draw_line(const std::array<Vertex, 2>& v) {
    rasterizer.draw_line(v);
}

void Pipeline::rast_draw_triangle(const std::array<Vertex, 3>& v) {
    rasterizer.draw_triangle(v, viewport);
}

void Pipeline::init_rasterizer() {
    rasterizer.callback = [this](auto&&v){fragment_shader_callback(v);};
}

}
