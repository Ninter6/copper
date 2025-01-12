//
//  printer.hpp
//  copper
//
//  Created by Ninter6 on 2024/3/16.
//

#pragma once

#include "core.hpp"

namespace cu {

struct BasicPrinter {
    virtual ~BasicPrinter() = default;
    
    virtual void print(std::string_view str) = 0;
    virtual void clear() = 0;

};

BasicPrinter& operator<<(BasicPrinter& pr, std::string_view str);

struct Printer : public BasicPrinter {
    Printer() = default;
    Printer(const Extent& viewport);

    void print(std::string_view str) override;
    void clear() override;

    Extent viewport{80, 22}; // nothing special
};

}
