//
//  printer.cpp
//  copper
//
//  Created by Ninter6 on 2024/3/16.
//

#include "print.hpp"

#include <cstring>

namespace cu {

Printer::Printer(const cu::PrinterCreateInfo &info)
:   viewport(info.viewport),
    mode(info.print_mode),
    sampler(info.sampler),
    char_tb(info.char_tb)
{
    puts("\033[2J");
    createBuffer();
}

Printer::~Printer() {
    destroyBuffer();
}

void Printer::createBuffer() {
    auto& ext = viewport;
    size_t buf_size = (2 * ext.x + 1) * ext.y;

    auto buf = new char[buf_size]{};
    m_buffer = buf;

    for (size_t i = 0; i < ext.y - 1; ++i)
        buf[i * (2 * ext.x + 1) + 2 * ext.x] = '\n';
}

void Printer::destroyBuffer() const {
    delete[] (char*)m_buffer;
}

char Printer::get_char(const Color& c) const {
    auto f = GetColorFeatureValue(c, mode);
    auto l = char_tb.length() - 1e-6f;
    return char_tb[(int)(f*f*l)];
}

void Printer::print(const cu::Image &image) const {
    writeBuffer(image);
    printBuffer();
}

void Printer::clrscr() const {
    puts("\033[H");
}

void Printer::writeBuffer(const cu::Image &image) const {
    auto buf = m_buffer;
    auto& ext = viewport;
    for (uint32_t j = 0; j < ext.y; ++j) {
        for (uint32_t i = 0; i < ext.x; ++i) {
            vec2 uv(i / (float)(ext.x - 1), j / (float)(ext.y - 1));
            auto color = sampler->get(image, uv);
            auto f = GetColorFeatureValue(color, mode);
            auto c = get_char(color);
            std::memset(buf + j * (2 * ext.x + 1) + 2 * i, c, 2);
        }
    }
}

void Printer::printBuffer() const {
    puts(m_buffer);
}

}
