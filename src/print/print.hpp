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

#include "sethread.h"

#include "core.hpp"

namespace cu {

struct CharTable {
    virtual ~CharTable() = default;
    
    virtual char get(float v) const;
};

struct NormCharTable : public CharTable {
    virtual char get(float v) const override;
    size_t find(float v) const;
    NormCharTable& addSlipPoint(float begin, char c);
    
    std::vector<std::pair<float, char>> slip_points;
};

struct PrinterCreateInfo {
    Extent viewport;
    ColorFeature print_mode;
    CharTable* char_table;
    Sampler* sampler;
};

class Printer {
public:
    Printer(const PrinterCreateInfo& info);
    virtual ~Printer() = default;
    
    virtual void print(const Image& image) const;
    static void clrscr();

    Extent getViewport() const;
    ColorFeature getPrintMode() const;
    CharTable* getCharTable() const;
    Sampler* getSampler() const;
    
private:
    Extent viewport;
    ColorFeature mode;
    CharTable* char_table;
    Sampler* sampler;
    
};

class AsyncPrinter : public Printer {
public:
    AsyncPrinter(const PrinterCreateInfo& info, st::ThreadPool* tp);
    ~AsyncPrinter();
    
    virtual void print(const Image& image) const override;
    
private:
    st::ThreadPool* m_tp;
    void* m_buffer;

    mutable std::atomic_bool m_available = true;

    void createBuffer();
    void destroyBuffer();

    void printImage(const Image& image) const;
    void writeBuffer(const Image& image) const;
    void printBuffer() const;

};

}
