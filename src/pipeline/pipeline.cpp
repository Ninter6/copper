//
//  pipeline.cpp
//  copper
//
//  Created by Ninter6 on 2024/3/16.
//

#include "pipeline.hpp"

#include <algorithm>

namespace cu {

Rasterizer::Rasterizer(const RasterizerCreateInfo& info)
: m_viewport(info.viewport) {
    
}

void Rasterizer::drawPoint(FrameBuffer* frame, int32_t x, int32_t y, float z) {
    testAndWrite(frame, {x, y}, z);
}

void Rasterizer::drawPoint(FrameBuffer* frame, const vec3& point) {
    auto uv = toViewport(point);
    testAndWrite(frame, uv, point.z);
}

void Rasterizer::drawLine(FrameBuffer *frame, int32_t x_begin, int32_t y_begin, int32_t x_end, int32_t y_end, float z_begin, float z_end) {
    
    int sx = x_end < x_begin ? -1 : 1;
    int sy = y_end < y_begin ? -1 : 1;
    
    long dx = sx > 0 ? x_end - x_begin : x_begin - x_end;
    long dy = sy > 0 ? y_end - y_begin : y_begin - y_end;
    
    auto x = x_begin;
    auto y = y_begin;
    
    bool steep = dx < dy;
    
    uint32_t final_x;
    if (steep) {
        final_x = y_end;
        std::swap(x, y);
        std::swap(sx, sy);
        std::swap(dx, dy);
    } else {
        final_x = x_end;
    }
    
    long e = -dx;
    long step = 2 * dy;
    long desc =-2 * dx;

    float z = z_begin;
    float sz = (z_end - z_begin) / (dx - 1.f);
    
    while (x != final_x) {
        if (steep)
            drawPoint(frame, y, x, z);
        else
            drawPoint(frame, x, y, z);
        
        e += step;
        if (e >= 0) {
            y += sy;
            e += desc;
        }
        x += sx;
        z += sz;
    }
}

void Rasterizer::drawLine(FrameBuffer* frame, const vec3& begin, const vec3& end) {
    vec2 b = begin, e = end;
    float db = begin.z, de = end.z;
    
    if (CohenSutherlandLineClip(b, e, -1.f, 1.f)
//        && !(db < 0 && de < 0)
//        && !(db > 1 && de > 1)
        ) {
        auto vb = toViewport(b);
        auto ve = toViewport(e);
        drawLine(frame, vb.x, vb.y, ve.x, ve.y, db, de);
    }
}

void Rasterizer::drawTriangleLine(FrameBuffer* frame, const vec3& a, const vec3& b, const vec3& c) {
    drawLine(frame, a, b);
    drawLine(frame, b, c);
    drawLine(frame, c, a);
}

void Rasterizer::drawTriangle(FrameBuffer* frame, const vec3& a, const vec3& b, const vec3& c) {
    vec3 v[] = {a, b, c};
    std::sort(v, v + 3, [](auto&&a, auto&&b) {
        return a.y < b.y;
    });
    ivec2 vs[] = {toViewport(v[0]), toViewport(v[1]), toViewport(v[2])};

    int sx[] = {vs[2].x < vs[0].x ? -1 : 1, vs[1].x < vs[0].x ? -1 : 1, vs[2].x < vs[1].x ? -1 : 1};
    int sy[] = {vs[2].y < vs[0].y ? -1 : 1, vs[1].y < vs[0].y ? -1 : 1, vs[2].y < vs[1].y ? -1 : 1};

    long dx[] = {
        sx[0] > 0 ? vs[2].x - vs[0].x : vs[0].x - vs[2].x,
        sx[1] > 0 ? vs[1].x - vs[0].x : vs[0].x - vs[1].x,
        sx[2] > 0 ? vs[2].x - vs[1].x : vs[1].x - vs[2].x
    };
    long dy[] = {
        vs[2].y - vs[0].y,
        vs[1].y - vs[0].y,
        vs[2].y - vs[1].y
    };

    bool steep[] = {
        dx[0] < dy[0],
        dx[1] < dy[1],
        dx[2] < dy[2]
    };

    int32_t x[] = {
        vs[0].x,
        vs[0].x,
        vs[1].x
    };
    int32_t y[] = {
        vs[0].y,
        vs[0].y,
        vs[1].y
    };

    int32_t final_x[3];

    for (uint32_t i = 0; i <3; ++i) {
        if (steep[i]) {
            final_x[i] = i == 1 ? vs[1].y : vs[2].y;
            std::swap(x[i], y[i]);
            std::swap(sx[i], sy[i]);
            std::swap(dx[i], dy[i]);
        } else {
            final_x[i] = i == 1 ? vs[1].x : vs[2].x;
        }
    }

    long e[] = {-dx[0], -dx[1], -dx[2]};
    long step[] = {2 * dy[0], 2 * dy[1], 2 * dy[2]};
    long desc[] = {-2 * dx[0], -2 * dx[1], -2 * dx[2]};

    float z[] = {v[0].z, v[0].z, v[1].z};
    float sz[] = {
        (v[2].z - v[0].z) / (dx[0] - 1.f),
        (v[1].z - v[0].z) / (dx[1] - 1.f),
        (v[2].z - v[1].z) / (dx[2] - 1.f)
    };

    int g = 1;
    if (!steep[1] && step[1] <= 0)
        g = 2;
    drawLine(frame, v[0], v[1]);
    drawLine(frame, v[1], v[2]);
    drawPoint(frame, v[2]);

    while (x[0] != final_x[0]) {
        if (x[g] == final_x[g])
            g = 2;

        while ((steep[0] ? x[0] : y[0]) > (steep[g] ? x[g] : y[g]) && x[g] != final_x[g]) {
            e[g] += step[g];
            if (e[g] >= 0) {
                y[g] += sy[g];
                e[g] += desc[g];
            }
            x[g] += sx[g];
            z[g] += sz[g];
        }

        if (steep[0]) {
            if (steep[g]) {
                drawLine(frame, y[0], x[0], y[g], x[g], z[0], z[g]);
            } else {
                drawLine(frame, y[0], x[0], x[g], y[g], z[0], z[g]);
            }
        } else {
            if (steep[g]) {
                drawLine(frame, x[0], y[0], y[g], x[g], z[0], z[g]);
            } else {
                drawLine(frame, x[0], y[0], x[g], y[g], z[0], z[g]);
            }
        }

        e[0] += step[0];
        if (e[0] >= 0) {
            y[0] += sy[0];
            e[0] += desc[0];
        }
        x[0] += sx[0];
        z[0] += sz[0];
    }
}

ivec2 Rasterizer::toViewport(const vec2& v) {
    auto u = (v + 1.f) * .5f;
    return ivec2(u.x * m_viewport.w + m_viewport.x, u.y * m_viewport.h + m_viewport.y);
}

bool Rasterizer::depthTest(FrameBuffer* frame, uivec2 uv, float depth) {
    return true;
}

void Rasterizer::writeFlag(FrameBuffer* frame, uivec2 uv) {
    frame->color_image->set(uv, {1.f});
}

void Rasterizer::testAndWrite(FrameBuffer* frame, ivec2 uv, float depth) {
    auto imageSize = frame->color_image->size();
    
    if (uv.x < imageSize.x && uv.x >= 0 &&
        uv.y < imageSize.y && uv.y >= 0 &&
        depthTest(frame, uivec2(uv.x, uv.y), depth))
        writeFlag(frame, uivec2(uv.x, uv.y));
}

bool Rasterizer::CohenSutherlandLineClip(vec2& p0, vec2& p1, const vec2& min, const vec2& max) {
    constexpr uint8_t INSIDE = 0; // 0000
    constexpr uint8_t LEFT = 1;   // 0001
    constexpr uint8_t RIGHT = 2;  // 0010
    constexpr uint8_t BOTTOM = 4; // 0100
    constexpr uint8_t TOP = 8;    // 1000
    
    auto ComputeOutCode = [&](vec2& p) {
        uint8_t code = INSIDE;  // initialised as being inside of clip window
        
        if (p.x < min.x)           // to the left of clip window
            code |= LEFT;
        else if (p.x > max.x)      // to the right of clip window
            code |= RIGHT;
        if (p.y < min.y)           // below the clip window
            code |= BOTTOM;
        else if (p.y > max.y)      // above the clip window
            code |= TOP;
        
        return code;
    };
    
    uint8_t outcode0 = ComputeOutCode(p0);
    uint8_t outcode1 = ComputeOutCode(p1);
    bool accept = false;

    while (true) {
        if (!(outcode0 | outcode1)) {
            accept = true;
            break;
        } else if (outcode0 & outcode1) {
            break;
        } else {
            vec2 p;

            int outcodeOut = outcode1 > outcode0 ? outcode1 : outcode0;
            
            if (outcodeOut & TOP) {
                p.x = p0.x + (p1.x - p0.x) * (max.y - p0.y) / (p1.y - p0.y);
                p.y = max.y;
            } else if (outcodeOut & BOTTOM) {
                p.x = p0.x + (p1.x - p0.x) * (min.y - p0.y) / (p1.y - p0.y);
                p.y = min.y;
            } else if (outcodeOut & RIGHT) {  // point is to the right of clip window
                p.y = p0.y + (p1.y - p0.y) * (max.x - p0.x) / (p1.x - p0.x);
                p.x = max.x;
            } else if (outcodeOut & LEFT) {   // point is to the left of clip window
                p.y = p0.y + (p1.y - p0.y) * (min.x - p0.x) / (p1.x - p0.x);
                p.x = min.x;
            }
            
            if (outcodeOut == outcode0) {
                p0.x = p.x;
                p0.y = p.y;
                outcode0 = ComputeOutCode(p0);
            } else {
                p1.x = p.x;
                p1.y = p.y;
                outcode1 = ComputeOutCode(p1);
            }
        }
    }
    return accept;
}

}
