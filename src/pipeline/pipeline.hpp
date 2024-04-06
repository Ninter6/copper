//
//  pipeline.hpp
//  copper
//
//  Created by Ninter6 on 2024/3/16.
//

#pragma once

#include "core.hpp"

namespace cu {

struct Viewport {
    uint32_t x, y;
    uint32_t w, h;
};

struct RasterizerCreateInfo {
    Viewport viewport;
};

class Rasterizer {
public:
    Rasterizer(const RasterizerCreateInfo& info);
    
    /**
     * All functions that use integers as parameters will render to the corresponding position of the image
     * The vector parameters are expected from NDC
     */

    void drawPoint(FrameBuffer* frame, int32_t x, int32_t y, float z = 0);
    void drawPoint(FrameBuffer* frame, const vec3& point);

    void drawLine(FrameBuffer* frame, int32_t x_begin, int32_t y_begin, int32_t x_end, int32_t y_end, float z_begin = 0, float z_end = 0);
    void drawLine(FrameBuffer* frame, const vec3& begin, const vec3& end);

    void drawTriangleLine(FrameBuffer* frame, const vec3& a, const vec3& b, const vec3& c);

    void drawTriangle(FrameBuffer* frame, const vec3& a, const vec3& b, const vec3& c);
    
private:
    ivec2 toViewport(const vec2& v);
    
    bool depthTest(FrameBuffer* frame, uivec2 uv, float depth);
    
    void writeFlag(FrameBuffer* frame, uivec2 uv);
    void testAndWrite(FrameBuffer* frame, ivec2 uv, float depth = 0);
    
    // [Cohen–Sutherland Algorithm](https://en.wikipedia.org/wiki/Cohen–Sutherland_algorithm)
    static bool CohenSutherlandLineClip(vec2& p0, vec2& p1, const vec2& min, const vec2& max);

private:
    Viewport m_viewport;

};

}
