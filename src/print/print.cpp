//
//  printer.cpp
//  copper
//
//  Created by Ninter6 on 2024/3/16.
//

#include "print.hpp"

#include <ranges>
#include <iostream>

namespace cu {

BasicPrinter& operator<<(BasicPrinter& pr, std::string_view str) {
    pr.print(str);
    return pr;
}

Printer::Printer(const Extent& viewport)
    : viewport(viewport) {}

void Printer::print(std::string_view str) {
    std::ranges::copy(std::views::iota(0, viewport.y) | std::views::transform([&](auto&& i) {
        return str.substr(i * viewport.x, viewport.x);
    }), std::ostream_iterator<std::string_view>{std::cout, "\n"});
}

void Printer::clear() {
    std::cout << "\033[H";
}

}
