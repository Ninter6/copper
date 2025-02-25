//
// Created by Ninter6 on 2024/7/7.
//

#pragma once

#include "core.hpp"

#include <functional>

namespace cu {

namespace algo {

struct Scanline;
struct Trapezoid;

}

using FragmentShaderCallback = std::function<void(const Vertex&)>;

struct RasterizerInitInfo {};

class Rasterizer {
public:
    Rasterizer() = default;

    void draw_point(const Vertex&);
    void draw_line(const std::array<Vertex, 2>&);
    void draw_triangle(const std::array<Vertex, 3>&, const Viewport&);

    void draw_scanline(const algo::Scanline&, const Viewport&);
    void draw_trapezoid(const algo::Trapezoid&, const Viewport&);

private:

    FragmentShaderCallback callback;

    friend class Pipeline;
};

}
