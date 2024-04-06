//
//  printer.cpp
//  copper
//
//  Created by Ninter6 on 2024/3/16.
//

#include "print.hpp"

#include "se_tools.h"

namespace cu {

char CharTable::get(float v) const {
    return " .,-:^+cfawb%G&$#@"[int(v*v*17.999f)];
}

char NormCharTable::get(float v) const {
    return slip_points[find(v)].second;
}

size_t NormCharTable::find(float v) const {
    if (v < slip_points.front().first)
        return 0;
    
    size_t l = 0, r = slip_points.size();
    size_t m = (l + r)/2;
    
    while (!(slip_points[m].first <= v && (m == slip_points.size() - 1 || slip_points[m+1].first > v))) {
        if (slip_points[m].first > v)
            r = m;
        else
            l = m;
        m = (l + r)/2;
    }
    
    return m;
}

NormCharTable& NormCharTable::addSlipPoint(float begin, char c) {
    size_t i = 0;
    if (slip_points.size() != 0)
        i = find(begin) + 1;
    slip_points.emplace(slip_points.begin() + i, begin, c);
    
    return *this;
}

Printer::Printer(const PrinterCreateInfo& info)
:   viewport(info.viewport),
    mode(info.print_mode), 
    char_table(info.char_table),
    sampler(info.sampler) {}

void Printer::print(const Image &image) const {
    const auto& width = viewport.x;
    const auto& height = viewport.y;
    
    char p[3]{0};
    for (uint32_t j = 0; j < height; ++j) {
        for (uint32_t i = 0; i < width; ++i) {
            vec2 uv(i / (float)(width - 1), j / (float)(height - 1));
            auto color = sampler->get(image, uv);
            auto f = GetColorFeatureValue(color, mode);
            auto c = char_table->get(f);
            
            std::memset(p, c, 2);
            std::fputs(p, stdout);
        }
        std::fputc('\n', stdout);
    }
}

void Printer::clrscr() {
    std::fputs("\E[2J\E[H", stdout);
}

Extent Printer::getViewport() const {
    return viewport;
}

ColorFeature Printer::getPrintMode() const {
    return mode;
}

CharTable* Printer::getCharTable() const {
    return char_table;
}

Sampler* Printer::getSampler() const {
    return sampler;
}

AsyncPrinter::AsyncPrinter(const cu::PrinterCreateInfo &info, st::ThreadPool *tp)
:   Printer(info),
    m_tp(tp) {
    createBuffer();
}

AsyncPrinter::~AsyncPrinter() {
    while (!m_available)
        std::this_thread::yield();
    destroyBuffer();
}

void AsyncPrinter::createBuffer() {
    auto ext = getViewport();
    size_t buf_size = (2 * ext.x + 1) * ext.y;

    auto buf = new char[buf_size]{};
    m_buffer = buf;

    for (size_t i = 0; i < ext.y - 1; ++i)
        buf[i * (2 * ext.x + 1) + 2 * ext.x] = '\n';
}

void AsyncPrinter::destroyBuffer() {
    delete[] (char*)m_buffer;
}

void AsyncPrinter::print(const cu::Image &image) const {
    if (!m_available) return;
//    m_tp->addTask([&]{
//        m_available = false;
//        printImage(image);
//        m_available = true;
//    });
    printImage(image);
}

void AsyncPrinter::printImage(const cu::Image &image) const {
    writeBuffer(image);
    printBuffer();
}

void AsyncPrinter::writeBuffer(const cu::Image &image) const {
    auto buf = static_cast<char*>(m_buffer);
    std::atomic_int num_t = 0;

    auto ext = getViewport();
    m_tp->dispatch(ext.y, [=, this, &num_t, &image](uint32_t b, uint32_t e) {
        ++num_t;
        for (uint32_t j = b; j < e; ++j) {
            for (uint32_t i = 0; i < ext.x; ++i) {
                vec2 uv(i / (float)(ext.x - 1), j / (float)(ext.y - 1));
                auto color = getSampler()->get(image, uv);
                auto f = GetColorFeatureValue(color, getPrintMode());
                auto c = getCharTable()->get(f);
                std::memset(buf + j * (2 * ext.x + 1) + 2 * i, c, 2);
            }
        }
        --num_t;
    });

    while (num_t != 0)
        std::this_thread::yield();
}

void AsyncPrinter::printBuffer() const {
    fputs((char*)m_buffer, stdout);
    fflush(stdout);
}

}
