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

cu::VertexArray va {
    .positions = {
        cu::vec3{ 1, 1, 1},
        cu::vec3{ 1,-1, 1},
        cu::vec3{ 1,-1,-1},
        cu::vec3{ 1, 1,-1},
        cu::vec3{-1, 1, 1},
        cu::vec3{-1,-1, 1},
        cu::vec3{-1,-1,-1},
        cu::vec3{-1, 1,-1},
    },
    .colors = {
        cu::vec3{ 1, 1, 1},
        cu::vec3{ 1, 0, 1},
        cu::vec3{ 1, 0, 0},
        cu::vec3{ 1, 1, 0},
        cu::vec3{ 0, 1, 1},
        cu::vec3{ 0, 0, 1},
        cu::vec3{ 0, 0, 0},
        cu::vec3{ 0, 1, 0},
    }
};

std::vector<cu::IndexGroup> ig = {
    // 正面（逆时针）
    {0, 0}, {1, 1}, {3, 3}, {1, 1}, {2, 2}, {3, 3},
    // 背面（顺时针）
    {4, 4}, {7, 7}, {5, 5}, {7, 7}, {6, 6}, {5, 5},
    // 左面（逆时针）
    {4, 4}, {0, 0}, {7, 7}, {0, 0}, {3, 3}, {7, 7},
    // 右面（逆时针）
    {1, 1}, {5, 5}, {2, 2}, {5, 5}, {6, 6}, {2, 2},
    // 顶面（逆时针）
    {4, 4}, {5, 5}, {0, 0}, {5, 5}, {1, 1}, {0, 0},
    // 底面（逆时针）
    {3, 3}, {2, 2}, {7, 7}, {2, 2}, {6, 6}, {7, 7}
};

int main() {
    constexpr cu::Extent ext{800, 500};

    cu::GuiCanvas::init({
        .window_size = ext,
        .title = "copper - debug"
    });
    auto gui = cu::GuiCanvas::get_canvas();

    cu::FrameBuffer fb {
        .color_image = gui,
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
        return {v.get_attr().var.color, 1.f};
    };

    cu::Pipeline pipe = {{
        .camera = cam,
        .rasterizer = rast,
        .vertexShader = vs,
        .fragmentShader = fs,
        .uniform = uni,
        .frame = fb,
        .viewport = {0, ext.y, ext.x, -ext.y},
        .cullFace = cu::CullFace::anticlockwise
    }};

    std::array vertices = {
        cu::vec3{-1.f, 1.f, 0.f},
        cu::vec3{ 1.f, 1.f, 0.f},
        cu::vec3{ 0.f,-1.5f, 0.f}
    };

    std::array<cu::Vertex, 3> v;
    for (int i = 0; i < v.size(); i++) {
        v[i].pos = vertices[i];
        v[i].attr.var.color[i] = 1.f;
    }

    [[maybe_unused]] cu::Printer pr{{
        .viewport = {80, 50},
        .print_mode = cu::ColorFeature::LUM,
        .sampler = &spl
    }};

    auto vai = va.getVertices(ig);

    int n = 0;
    while (!gui->window_should_close()) {
        uni->matrix["model"] = cu::translate(cu::vec3{0, 0, -3.f}) * cu::rotate<float>(cu::EulerAngle{M_PI*n/180, M_PI*n/150, M_PI*n/210}, cu::xyz);
        pipe.draw_array(vai, cu::Topology::triangle);

        gui->show();
        gui->clear({.5f, .5f, .5f, 1.f});

        n++;
    }

    return 0;
}
