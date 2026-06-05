#pragma once

#include "gfxp/fixed.hpp"

namespace gfxp {

struct Vec2 {
    Fixed x = Fixed::zero();
    Fixed y = Fixed::zero();

    [[nodiscard]] static constexpr Vec2 zero() {
        return Vec2{};
    }

    [[nodiscard]] static constexpr Vec2 from_raw(int32_t x_raw, int32_t y_raw) {
        return Vec2{Fixed::from_raw(x_raw), Fixed::from_raw(y_raw)};
    }

    [[nodiscard]] static constexpr Vec2 from_pixels(int32_t x_pixels, int32_t y_pixels) {
        return Vec2{Fixed::from_int(x_pixels), Fixed::from_int(y_pixels)};
    }

    constexpr Vec2& operator+=(Vec2 other) {
        x += other.x;
        y += other.y;
        return *this;
    }

    constexpr Vec2& operator-=(Vec2 other) {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    constexpr Vec2& operator*=(Fixed scalar) {
        x *= scalar;
        y *= scalar;
        return *this;
    }

    constexpr Vec2& operator/=(Fixed scalar) {
        x /= scalar;
        y /= scalar;
        return *this;
    }
};

[[nodiscard]] constexpr Vec2 operator+(Vec2 lhs, Vec2 rhs) {
    lhs += rhs;
    return lhs;
}

[[nodiscard]] constexpr Vec2 operator-(Vec2 lhs, Vec2 rhs) {
    lhs -= rhs;
    return lhs;
}

[[nodiscard]] constexpr Vec2 operator*(Vec2 lhs, Fixed scalar) {
    lhs *= scalar;
    return lhs;
}

[[nodiscard]] constexpr Vec2 operator*(Fixed scalar, Vec2 rhs) {
    return rhs * scalar;
}

[[nodiscard]] constexpr Vec2 operator/(Vec2 lhs, Fixed scalar) {
    lhs /= scalar;
    return lhs;
}

[[nodiscard]] constexpr bool operator==(Vec2 lhs, Vec2 rhs) {
    return lhs.x == rhs.x && lhs.y == rhs.y;
}

[[nodiscard]] constexpr bool operator!=(Vec2 lhs, Vec2 rhs) {
    return !(lhs == rhs);
}

} // namespace gfxp
