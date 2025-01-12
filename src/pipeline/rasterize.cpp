//
// Created by Ninter6 on 2024/7/7.
//

#include "rasterize.hpp"

#include <ranges>

namespace cu {

namespace algo {

struct LineDrawer {
    LineDrawer(ivec2 begin, ivec2 end) : p(begin), end(end) {
        dx = std::abs(end.x - begin.x);
        dy = std::abs(end.y - begin.y);
        sx = begin.x < end.x ? 1 : -1;
        sy = begin.y < end.y ? 1 : -1;
        err = dx - dy;
    }

    std::optional<ivec2> advance() {
        if (p == end) return std::nullopt;

        int e2 = err << 1;
        if (e2 > -dy) {
            err -= dy;
            p.x += sx;
        }
        if (e2 < dx) {
            err += dx;
            p.y += sy;
        }
        return p;
    }

    int dx, dy;
    int sx, sy;
    int err;

    ivec2 p, end;
};

// scan line algo
struct Edge {
    Vertex A, B;

    [[nodiscard]] float y2x(float y) const {
        float dx = B.pos.x - A.pos.x;
        float dy = B.pos.y - A.pos.y;
        float ey = B.pos.y - y;
        return B.pos.x - dx / dy * ey;
    }

    [[nodiscard]] float y2t(float y) const {
        float dy = B.pos.y - A.pos.y;
        float ty = y - A.pos.y;
        return ty / dy;
    }
};

struct Trapezoid {
    float bottom;
    float top;
    Edge left;
    Edge right;
};

std::array<std::optional<Trapezoid>, 2> triangle2trapezoid(std::array<Vertex, 3> vertices) {
    constexpr auto equal = [&](float a, float b) {
        return std::abs(a - b) <= std::numeric_limits<float>::epsilon();
    };

    auto p = std::views::transform(vertices, [](auto &&i) { return i.pos; });

    // determine whether the three points are collinear
    if (equal(std::abs((p[1].x - p[0].x) * (p[2].y - p[0].y) - (p[1].y - p[0].y) * (p[2].x - p[0].x)), 0))
        return {std::nullopt, std::nullopt};

    std::sort(vertices.begin(), vertices.end(), [](auto &&a, auto &&b) { return a.pos.y < b.pos.y; });

    if (equal(p[0].y, p[1].y)) {
        if (p[0].x > p[1].x)
            std::swap(vertices[0], vertices[1]);
        return {Trapezoid{p[0].y, p[2].y, Edge{vertices[2], vertices[0]}, Edge{vertices[2], vertices[1]}},
                std::nullopt};
    } else if (equal(p[1].y, p[2].y)) {
        if (p[1].x > p[2].x)
            std::swap(vertices[1], vertices[2]);
        return {Trapezoid{p[0].y, p[1].y, Edge{vertices[0], vertices[1]}, Edge{vertices[0], vertices[2]}},
                std::nullopt};
    }

    Edge e01 = {vertices[0], vertices[1]}, e02 = {vertices[0], vertices[2]}, e12 = {vertices[1], vertices[2]};

    if (p[1].x > e02.y2x(p[1].y))
        return {Trapezoid{p[0].y, p[1].y, e02, e01},
                Trapezoid{p[1].y, p[2].y, e02, e12}};
    else
        return {Trapezoid{p[0].y, p[1].y, e01, e02},
                Trapezoid{p[1].y, p[2].y, e12, e02}};
}

std::optional<Trapezoid> trapezoid_clip(const Trapezoid &trap, float ymin, float ymax) {
    if (trap.bottom > ymax || trap.top < ymin)
        return std::nullopt;
    auto rst = trap;
    rst.top = std::min(ymax, trap.top);
    rst.bottom = std::max(ymin, trap.bottom);
    return rst;
}

struct Scanline {
    Scanline() = default;

    Scanline(const Vertex &vertex, const Vertex &step, float width)
    : vertex(vertex), step(step), width((int)round(width)) {}

    // trapezoid2scanline
    Scanline(const Trapezoid &trap, float y) {
        vertex = lerp(trap.left.A, trap.left.B, trap.left.y2t(y));
        vertex.pos.x = round(vertex.pos.x), vertex.pos.y = y;
        step = lerp(trap.right.A, trap.right.B, trap.right.y2t(y));
        step.pos.x = round(step.pos.x), step.pos.y = y;
        width = (int)step.pos.x - (int)vertex.pos.x;
        if (width > 0) step = (step - vertex) / (float)width;
    }

    std::optional<Vertex> advance() {
        if (--width < 0) return std::nullopt;
        return (vertex += step);
    }

    Vertex vertex{}; // left vertex
    Vertex step{}; // step of scanline
    int width{}; // width of scanline
};

std::optional<Scanline> scanline_clip(const Scanline &scanline, float xmin, float xmax) {
    auto l = scanline.vertex.pos.x;
    auto r = l + (float)scanline.width;
    if (auto w = (float)scanline.width + xmax - xmin - std::max(r, xmax) + std::min(l, xmin); w >= 0) {
        if (l >= xmin) return {{scanline.vertex, scanline.step, w}};
        float e = xmin - l;
        return {{scanline.vertex + scanline.step * e, scanline.step, w}};
    }
    return std::nullopt;
}

}

//Rasterizer::Rasterizer(const RasterizerInitInfo&) {}

void Rasterizer::draw_point(const Vertex& v) {
    callback(v);
}

void Rasterizer::draw_line(const std::array<Vertex, 2>& v) {
    algo::LineDrawer drawer{(vec2)v[0].pos, (vec2)v[1].pos};
    algo::Edge edge{v[0], v[1]};
    while (auto p = drawer.advance()) {
        auto t = edge.y2t((float)p->y);
        draw_point(lerp(v[0], v[1], t));
    }
}

void Rasterizer::draw_triangle(const std::array<Vertex, 3>& v, const Viewport& viewport) {
    for (auto&& i : algo::triangle2trapezoid(v))
        if (i) draw_trapezoid(*i, viewport);
}

void Rasterizer::draw_scanline(const algo::Scanline& scanline, const Viewport& viewport) {
    auto xmin = (float)viewport.x, xmax = (float)(viewport.x + viewport.w);
    if (xmin > xmax) std::swap(xmin, xmax);
    if (auto clipped = algo::scanline_clip(scanline, xmin - 1, xmax - 1)) // it offers (min, max], but we want [min, max)
        while (auto v = clipped->advance())
            draw_point(*v);
}

void Rasterizer::draw_trapezoid(const algo::Trapezoid& trap, const Viewport& viewport) {
    auto ymin = (float)viewport.y, ymax = (float)(viewport.y + viewport.h);
    if (ymin > ymax) std::swap(ymin, ymax);
    if (auto clipped = algo::trapezoid_clip(trap, ymin, ymax)) {
        for (int y = ceil(clipped->bottom), end = ceil(clipped->top); y < end; ++y) {
            auto scanline = algo::Scanline(*clipped, (float)y);
            draw_scanline(scanline, viewport);
        }
    }
}

}