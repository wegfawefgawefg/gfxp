#pragma once

#include <cmath>
#include <cstdint>
#include <limits>
#include <optional>
#include <string_view>
#include <type_traits>

namespace gfxp {

enum class Rounding {
    TowardZero,
    Floor,
    Ceil,
    Nearest,
};

namespace detail {

constexpr bool is_digit(char c) {
    return c >= '0' && c <= '9';
}

constexpr std::optional<int64_t> div_round(int64_t numerator, int64_t denominator,
                                           Rounding rounding) {
    if (denominator == 0)
        return std::nullopt;

    if (denominator < 0) {
        numerator = -numerator;
        denominator = -denominator;
    }

    const int64_t quotient = numerator / denominator;
    const int64_t remainder = numerator % denominator;
    if (remainder == 0)
        return quotient;

    switch (rounding) {
    case Rounding::TowardZero:
        return quotient;
    case Rounding::Floor:
        return numerator < 0 ? quotient - 1 : quotient;
    case Rounding::Ceil:
        return numerator > 0 ? quotient + 1 : quotient;
    case Rounding::Nearest: {
        const int64_t abs_remainder = remainder < 0 ? -remainder : remainder;
        const bool round_up = abs_remainder * 2 >= denominator;
        if (!round_up)
            return quotient;
        return numerator > 0 ? quotient + 1 : quotient - 1;
    }
    }

    return std::nullopt;
}

constexpr std::optional<int32_t> checked_i32(int64_t value) {
    if (value < std::numeric_limits<int32_t>::min() ||
        value > std::numeric_limits<int32_t>::max()) {
        return std::nullopt;
    }
    return static_cast<int32_t>(value);
}

} // namespace detail

struct Fixed {
    int32_t raw = 0;

    static constexpr int frac_bits = 10;
    static constexpr int32_t scale = 1 << frac_bits;

    [[nodiscard]] static constexpr Fixed zero() {
        return Fixed{};
    }

    [[nodiscard]] static constexpr Fixed from_raw(int32_t raw_value) {
        return Fixed{raw_value};
    }

    [[nodiscard]] static constexpr Fixed from_int(int32_t value) {
        return Fixed{static_cast<int32_t>(value * scale)};
    }

    [[nodiscard]] static constexpr std::optional<Fixed> checked_from_int(int32_t value) {
        const int64_t raw_value = static_cast<int64_t>(value) * scale;
        const std::optional<int32_t> checked = detail::checked_i32(raw_value);
        if (!checked)
            return std::nullopt;
        return Fixed{*checked};
    }

    [[nodiscard]] static constexpr std::optional<Fixed>
    from_ratio(int64_t numerator, int64_t denominator, Rounding rounding = Rounding::Nearest) {
        const std::optional<int64_t> rounded =
            detail::div_round(numerator * static_cast<int64_t>(scale), denominator, rounding);
        if (!rounded)
            return std::nullopt;

        const std::optional<int32_t> checked = detail::checked_i32(*rounded);
        if (!checked)
            return std::nullopt;
        return Fixed{*checked};
    }

    [[nodiscard]] static constexpr std::optional<Fixed>
    from_decimal(std::string_view text, Rounding rounding = Rounding::Nearest) {
        if (text.empty())
            return std::nullopt;

        bool negative = false;
        std::size_t index = 0;
        if (text[index] == '+' || text[index] == '-') {
            negative = text[index] == '-';
            ++index;
        }

        bool saw_digit = false;
        int64_t whole = 0;
        while (index < text.size() && detail::is_digit(text[index])) {
            saw_digit = true;
            const int digit = text[index] - '0';
            if (whole > (std::numeric_limits<int64_t>::max() - digit) / 10)
                return std::nullopt;
            whole = whole * 10 + digit;
            ++index;
        }

        int64_t fractional = 0;
        int64_t fractional_denominator = 1;
        if (index < text.size() && text[index] == '.') {
            ++index;
            while (index < text.size() && detail::is_digit(text[index])) {
                saw_digit = true;
                const int digit = text[index] - '0';
                if (fractional_denominator > std::numeric_limits<int64_t>::max() / 10)
                    return std::nullopt;
                fractional_denominator *= 10;
                if (fractional > (std::numeric_limits<int64_t>::max() - digit) / 10)
                    return std::nullopt;
                fractional = fractional * 10 + digit;
                ++index;
            }
        }

        if (!saw_digit || index != text.size())
            return std::nullopt;

        if (whole > std::numeric_limits<int64_t>::max() / fractional_denominator)
            return std::nullopt;
        int64_t numerator = whole * fractional_denominator + fractional;
        if (negative)
            numerator = -numerator;

        return from_ratio(numerator, fractional_denominator, rounding);
    }

    [[nodiscard]] static Fixed from_float_for_boundary(float value,
                                                       Rounding rounding = Rounding::Nearest) {
        const float scaled = value * static_cast<float>(scale);
        switch (rounding) {
        case Rounding::TowardZero:
            return Fixed{static_cast<int32_t>(scaled)};
        case Rounding::Floor:
            return Fixed{static_cast<int32_t>(std::floor(scaled))};
        case Rounding::Ceil:
            return Fixed{static_cast<int32_t>(std::ceil(scaled))};
        case Rounding::Nearest:
            return Fixed{static_cast<int32_t>(std::round(scaled))};
        }
        return Fixed{};
    }

    [[nodiscard]] constexpr int32_t raw_value() const {
        return raw;
    }

    [[nodiscard]] constexpr float to_float() const {
        return static_cast<float>(raw) / static_cast<float>(scale);
    }

    [[nodiscard]] constexpr double to_double() const {
        return static_cast<double>(raw) / static_cast<double>(scale);
    }

    [[nodiscard]] constexpr int32_t trunc_int() const {
        return raw / scale;
    }

    [[nodiscard]] constexpr int32_t floor_int() const {
        const int32_t quotient = raw / scale;
        const int32_t remainder = raw % scale;
        if (remainder != 0 && raw < 0)
            return quotient - 1;
        return quotient;
    }

    [[nodiscard]] constexpr int32_t ceil_int() const {
        const int32_t quotient = raw / scale;
        const int32_t remainder = raw % scale;
        if (remainder != 0 && raw > 0)
            return quotient + 1;
        return quotient;
    }

    [[nodiscard]] constexpr int32_t round_int() const {
        const std::optional<int64_t> rounded = detail::div_round(raw, scale, Rounding::Nearest);
        return rounded ? static_cast<int32_t>(*rounded) : 0;
    }

    [[nodiscard]] constexpr Fixed abs() const {
        return raw < 0 ? Fixed{static_cast<int32_t>(-raw)} : *this;
    }

    [[nodiscard]] constexpr int sign() const {
        if (raw < 0)
            return -1;
        if (raw > 0)
            return 1;
        return 0;
    }

    constexpr Fixed& operator+=(Fixed other) {
        raw = static_cast<int32_t>(raw + other.raw);
        return *this;
    }

    constexpr Fixed& operator-=(Fixed other) {
        raw = static_cast<int32_t>(raw - other.raw);
        return *this;
    }

    constexpr Fixed& operator*=(Fixed other) {
        raw = static_cast<int32_t>((static_cast<int64_t>(raw) * other.raw) / scale);
        return *this;
    }

    constexpr Fixed& operator/=(Fixed other) {
        const std::optional<int64_t> divided =
            detail::div_round(static_cast<int64_t>(raw) * scale, other.raw, Rounding::TowardZero);
        raw = divided ? static_cast<int32_t>(*divided) : 0;
        return *this;
    }

    [[nodiscard]] constexpr Fixed operator-() const {
        return Fixed{static_cast<int32_t>(-raw)};
    }
};

[[nodiscard]] constexpr Fixed operator+(Fixed lhs, Fixed rhs) {
    lhs += rhs;
    return lhs;
}

[[nodiscard]] constexpr Fixed operator-(Fixed lhs, Fixed rhs) {
    lhs -= rhs;
    return lhs;
}

[[nodiscard]] constexpr Fixed operator*(Fixed lhs, Fixed rhs) {
    lhs *= rhs;
    return lhs;
}

[[nodiscard]] constexpr Fixed operator/(Fixed lhs, Fixed rhs) {
    lhs /= rhs;
    return lhs;
}

[[nodiscard]] constexpr Fixed operator*(Fixed lhs, int32_t rhs) {
    return Fixed{static_cast<int32_t>(lhs.raw * rhs)};
}

[[nodiscard]] constexpr Fixed operator*(int32_t lhs, Fixed rhs) {
    return rhs * lhs;
}

[[nodiscard]] constexpr Fixed operator/(Fixed lhs, int32_t rhs) {
    return Fixed{static_cast<int32_t>(lhs.raw / rhs)};
}

[[nodiscard]] constexpr bool operator==(Fixed lhs, Fixed rhs) {
    return lhs.raw == rhs.raw;
}

[[nodiscard]] constexpr bool operator!=(Fixed lhs, Fixed rhs) {
    return !(lhs == rhs);
}

[[nodiscard]] constexpr bool operator<(Fixed lhs, Fixed rhs) {
    return lhs.raw < rhs.raw;
}

[[nodiscard]] constexpr bool operator<=(Fixed lhs, Fixed rhs) {
    return lhs.raw <= rhs.raw;
}

[[nodiscard]] constexpr bool operator>(Fixed lhs, Fixed rhs) {
    return lhs.raw > rhs.raw;
}

[[nodiscard]] constexpr bool operator>=(Fixed lhs, Fixed rhs) {
    return lhs.raw >= rhs.raw;
}

[[nodiscard]] constexpr Fixed min(Fixed lhs, Fixed rhs) {
    return lhs < rhs ? lhs : rhs;
}

[[nodiscard]] constexpr Fixed max(Fixed lhs, Fixed rhs) {
    return lhs > rhs ? lhs : rhs;
}

[[nodiscard]] constexpr Fixed clamp(Fixed value, Fixed low, Fixed high) {
    return min(max(value, low), high);
}

} // namespace gfxp

namespace std {

template <> struct hash<gfxp::Fixed> {
    std::size_t operator()(gfxp::Fixed value) const noexcept {
        return std::hash<int32_t>{}(value.raw_value());
    }
};

} // namespace std
