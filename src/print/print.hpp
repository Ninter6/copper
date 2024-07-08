//
//  printer.hpp
//  copper
//
//  Created by Ninter6 on 2024/3/16.
//

#pragma once

#include <iostream>
#include <vector>
#include <memory>

#include "raylib.h"

#include "core.hpp"

namespace cu {

struct PrinterBase {
    virtual ~PrinterBase() = default;
    
    virtual void print(const Image& image) const = 0;
    virtual void clrscr() const = 0;

};

struct PrinterCreateInfo {
    Extent viewport;
    ColorFeature print_mode;
    Sampler* sampler;
    std::string char_tb = " .,-:^+cfawb%G&$#@";
};

struct Printer : public PrinterBase {
    Printer(const PrinterCreateInfo& info);
    ~Printer();

    Printer(Printer&&) = delete;

    void print(const Image& image) const override;
    void clrscr() const override;

    char get_char(const Color& c) const;

    char* m_buffer;

    Extent viewport;
    ColorFeature mode;
    Sampler* sampler;
    std::string char_tb;

    void createBuffer();
    void destroyBuffer() const;

    void writeBuffer(const Image& image) const;
    void printBuffer() const;

};

}
