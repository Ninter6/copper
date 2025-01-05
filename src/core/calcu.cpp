//
// Created by Ninter6 on 2025/1/4.
//

#include <algorithm>

#include "calcu.hpp"

namespace cu {

Attribute& Attribute::operator+=(const Attribute& o) {
    for (int i = 0; i < std::size(data); i++)
        data[i] += o.data[i];
    return *this;
}
Attribute Attribute::operator+(const Attribute& o) const {
    auto r = *this;
    return r += o;
}
Attribute& Attribute::operator-=(const Attribute& o) {
    for (int i = 0; i < std::size(data); i++)
        data[i] -= o.data[i];
    return *this;
}
Attribute Attribute::operator-(const Attribute& o) const {
    auto r = *this;
    return r -= o;
}
Attribute& Attribute::operator*=(float k) {
    for (auto&& i : data)
        i *= k;
    return *this;
}
Attribute Attribute::operator*(float k) const {
    auto r = *this;
    return r *= k;
}
Attribute& Attribute::operator/=(float k) {
    return *this *= (1.f / k);
}
Attribute Attribute::operator/(float k) const {
    return *this * (1.f / k);
}

}