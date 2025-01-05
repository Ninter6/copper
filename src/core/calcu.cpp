//
// Created by Ninter6 on 2025/1/4.
//

#include "calcu.hpp"

namespace cu {

Attribute& Attribute::operator+=(const Attribute& o) {
#ifndef CU_ENABLED_SIMD
    for (int i = 0; i < attr_data_size; i++)
        data[i] += o.data[i];
#else
    mm_data[0] = _mm_add_ps(mm_data[0], o.mm_data[0]);
    mm_data[1] = _mm_add_ps(mm_data[1], o.mm_data[1]);
    mm_data[2] = _mm_add_ps(mm_data[2], o.mm_data[2]);
    mm_data[3] = _mm_add_ps(mm_data[3], o.mm_data[3]);
#endif
    return *this;
}
Attribute Attribute::operator+(const Attribute& o) const {
    auto r = *this;
    return r += o;
}
Attribute& Attribute::operator-=(const Attribute& o) {
#ifndef CU_ENABLED_SIMD
    for (int i = 0; i < attr_data_size; i++)
        data[i] -= o.data[i];
#else
    mm_data[0] = _mm_sub_ps(mm_data[0], o.mm_data[0]);
    mm_data[1] = _mm_sub_ps(mm_data[1], o.mm_data[1]);
    mm_data[2] = _mm_sub_ps(mm_data[2], o.mm_data[2]);
    mm_data[3] = _mm_sub_ps(mm_data[3], o.mm_data[3]);
#endif
    return *this;
}
Attribute Attribute::operator-(const Attribute& o) const {
    auto r = *this;
    return r -= o;
}
Attribute& Attribute::operator*=(float k) {
#ifndef CU_ENABLED_SIMD
    for (auto&& i : data)
        i *= k;
#else
    auto mm_k = _mm_set1_ps(k);
    mm_data[0] = _mm_mul_ps(mm_data[0], mm_k);
    mm_data[1] = _mm_mul_ps(mm_data[1], mm_k);
    mm_data[2] = _mm_mul_ps(mm_data[2], mm_k);
    mm_data[3] = _mm_mul_ps(mm_data[3], mm_k);
#endif
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