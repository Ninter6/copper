//
//  main.cpp
//  copper
//
//  Created by Ninter6 on 2024/3/16.
//

#include <iostream>
#include <numeric>
 
#include "print.hpp"
#include "async.hpp"
#include "ascii.hpp"

#ifdef COPPER_INCLUDE_EXT
#   include "ext/gui.hpp"
#endif

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
        cu::vec4{1, 1, 1, 1},
        cu::vec4{1, 0, 1, 1},
        cu::vec4{1, 0, 0, 1},
        cu::vec4{1, 1, 0, 1},
        cu::vec4{0, 1, 1, 1},
        cu::vec4{0, 0, 1, 1},
        cu::vec4{0, 0, 0, 1},
        cu::vec4{0, 1, 0, 1},
    }
};

std::vector<cu::IndexGroup> ig = {
    {.pos=0, .col=0}, {.pos=1, .col=1}, {.pos=3, .col=3}, {.pos=1, .col=1}, {.pos=2, .col=2}, {.pos=3, .col=3}, // 正面
    {.pos=4, .col=4}, {.pos=7, .col=7}, {.pos=5, .col=5}, {.pos=7, .col=7}, {.pos=6, .col=6}, {.pos=5, .col=5}, // 背面
    {.pos=4, .col=4}, {.pos=0, .col=0}, {.pos=7, .col=7}, {.pos=0, .col=0}, {.pos=3, .col=3}, {.pos=7, .col=7}, // 左面
    {.pos=1, .col=1}, {.pos=5, .col=5}, {.pos=2, .col=2}, {.pos=5, .col=5}, {.pos=6, .col=6}, {.pos=2, .col=2}, // 右面
    {.pos=4, .col=4}, {.pos=5, .col=5}, {.pos=0, .col=0}, {.pos=5, .col=5}, {.pos=1, .col=1}, {.pos=0, .col=0}, // 顶面
    {.pos=3, .col=3}, {.pos=2, .col=2}, {.pos=7, .col=7}, {.pos=2, .col=2}, {.pos=6, .col=6}, {.pos=7, .col=7}  // 底面
};

int main() {
    constexpr cu::Extent ext{800, 500};

//    std::ios::sync_with_stdio(false);

//    cu::GuiCanvas::init({
//        .window_size = ext,
//        .title = "copper - debug"
//    });
//    auto gui = cu::GuiCanvas::get_canvas();

    cu::FrameBuffer fb {
        .color_image = std::make_shared<cu::ImageRGBA8>(ext),
        .depth_image = nullptr
    };

    cu::NearestSampler spl{};
    cu::Texture tex{fb.color_image.get(), &spl};

    auto cam = std::make_shared<cu::Camera>(
        cu::Frustum{.1f, (float)ext.x / ext.y, cu::radians(60.f)},
        cu::vec3{0.f}
    );

    auto uni = std::make_shared<cu::Uniform>();
    uni->matrix["model"] = {};

    auto vs = [](auto&& v, auto&& uni, auto&& cam) -> cu::vec4 {
        return cam.proj_view() * uni.matrix.at("model") * cu::vec4{v.pos, 1.f};
    };
    auto fs = [](auto&& v, auto&& uni, auto&& cam) -> std::optional<cu::Color> {
        auto color = v.get_attr().var.color;
        color.a = .5f;
        cu::vec3 pos = color;
        pos = pos * 2 - 1;
        if (pos.length_squared() > 1.64f)
            return std::nullopt;
        return color;
    };

    cu::Pipeline pipe = {{
        .camera = cam,
        .vertexShader = vs,
        .fragmentShader = fs,
        .uniform = uni,
        .frame = fb,
        .viewport = {0, ext.y, ext.x, -ext.y},
        .cullFace = cu::CullFace::none,
        .enable_blend = true
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

    auto vai = va.getVertices(ig);

    cu::Extent term_ext{160, 50};
    [[maybe_unused]] cu::Printer pr{term_ext};

    cu::AsciiFactory ascii;
    ascii.set_noise_enable(true);

    int n = 0;
    while (n < INT_MAX) {
        auto f = cu::FLatch{std::chrono::milliseconds{16}};

        uni->matrix["model"] = cu::translate(cu::vec3{0, 0, -3.f}) * cu::rotate<float>(cu::EulerAngle{M_PI*n/180, M_PI*n/150, M_PI*n/210}, cu::xyz);
        pipe.draw_array(vai, cu::Topology::triangle);
//        pipe.finish();

        cam->position.z = sinf(M_PI*n/180)*2.f;

        pr << ascii.process(tex, pr.viewport);

        fb.color_image->clear({.5f, .5f, .5f, 1.f});
        pr.clear();

//        gui->show();
//        gui->clear({.5f, .5f, .5f, 1.f});

        n++;
    }

    return 0;
}
