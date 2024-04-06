//
//  main.cpp
//  copper
//
//  Created by Ninter6 on 2024/3/16.
//

#include <iostream>
 
#include "print.hpp"
#include "pipeline.hpp"

int main(int argc, const char * argv[]) {
    cu::ImageR8 image({100, 100});
    
    cu::FrameBuffer fb;
    fb.color_image = &image;
    
    cu::Rasterizer rast{{{0, 0, 100, 100}}};
    
    cu::CharTable ctable{};
    cu::LinearSampler spl;
    
    cu::PrinterCreateInfo info{
        {50, 42},
        cu::ColorFeature::R,
        &ctable,
        &spl
    };

    st::ThreadPool tp{2};
    cu::AsyncPrinter pr{info, &tp};

    cu::vec3 vertices[] {
        {0.f, -1.f, 0.f},
        {1.f, 1.f, 0.f},
        {-1.f, 1.f, 0.f}
    };

    cu::Camera cam {
        {
            .5,
            M_PI / 3,
            1
        },
        {0, 0, 5.f}
    };
    
    for (int i = 0; ; i += 2) {
        auto t = std::chrono::steady_clock::now();

        cu::vec3 v[3];
        for (int j = 0; j < 3; j++) {
            v[j] = cam.toNDC({vertices[j], 1.f}, cu::rotate<float>({0.f, M_PI*i/180, 0.f}, cu::xyz));
        }

        rast.drawTriangle(&fb, v[0], v[1], v[2]);
        pr.clrscr();
        pr.print(image);

        image.clear({});
        std::this_thread::sleep_until(t + std::chrono::microseconds{100000/6});
    }

//    rast.drawTriangle(&fb, vertices[0] * .5f, vertices[1] * .5f, vertices[2] * .5f);
//    pr.print(image);

    return 0;
}
