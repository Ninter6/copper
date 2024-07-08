//
//  main.cpp
//  copper
//
//  Created by Ninter6 on 2024/3/16.
//

#include <iostream>
 
#include "print.hpp"
#include "pipeline.hpp"

#include "ext/gui.hpp"

int main(int argc, const char * argv[]) {
    constexpr cu::Extent ext{800, 500};

    cu::GuiCanvas::init({
        .window_size = ext,
        .title = "copper - debug"
    });

    cu::FrameBuffer fb {
        .color_image = cu::GuiCanvas::get_canvas(),
        .depth_image = nullptr
    };

    cu::NearestSampler spl{};

    auto cam = std::make_shared<cu::Camera>(
        cu::Frustum{.1f, (float)ext.x / ext.y, cu::radians(60.f)},
        cu::vec3{0.f}
        );

    auto uni = std::make_shared<cu::Uniform>();
    uni->matrix["model"] = {};

    auto rast = std::make_shared<cu::Rasterizer>();

    auto vs = [](auto&& v, auto&& uni, auto&& cam) -> cu::vec4 {
        return cam.proj_view() * uni.matrix.at("model") * cu::vec4{v.pos, 1.f};
    };
    auto fs = [](auto&& v, auto&& uni, auto&& cam) -> cu::Color {
        return {v.get_attr().color, 1.f};
    };

    cu::Pipeline pipe = {{
        .camera = cam,
        .rasterizer = rast,
        .vertexShader = vs,
        .fragmentShader = fs,
        .uniform = uni,
        .frame = fb,
        .viewport = {0, ext.y, ext.x, -ext.y},
        .cullFace = cu::CullFace::none
    }};

    std::array vertices = {
        cu::vec3{-1.f, 1.f, 0.f},
        cu::vec3{ 1.f, 1.f, 0.f},
        cu::vec3{ 0.f,-1.5f, 0.f}
    };

    std::array<cu::Vertex, 3> v;
    for (int i = 0; i < v.size(); i++) {
        v[i].pos = vertices[i];
        v[i].attr.color[i] = 1.f;
    }

    [[maybe_unused]] cu::Printer pr{{
        .viewport = {80, 50},
        .print_mode = cu::ColorFeature::LUM,
        .sampler = &spl
    }};

    int n = 0;
    while (!cu::GuiCanvas::window_should_close()) {
        uni->matrix["model"] = cu::translate(cu::vec3{0, 0, -2.f}) * cu::rotate<float>({0, 1.f, 0}, M_PI*n/180.f);
        pipe.draw_triangle(v);

        cu::GuiCanvas::get_canvas()->show();
        fb.color_image->clear({.5f, .5f, .5f, 1.f});

        n++;
    }

    return 0;
}
