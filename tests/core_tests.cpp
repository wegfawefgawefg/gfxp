#include <cstdlib>
#include <functional>
#include <gfxp/gfxp.hpp>
#include <iostream>
#include <string>

namespace {

int g_failures = 0;

void check(bool condition, const std::string& message) {
    if (condition)
        return;
    ++g_failures;
    std::cerr << "FAIL: " << message << "\n";
}

void test_basic_arithmetic() {
    const gfxp::Fixed one = gfxp::Fixed::from_int(1);
    const gfxp::Fixed half = gfxp::Fixed::from_decimal("0.5").value();
    const gfxp::Fixed quarter = gfxp::Fixed::from_decimal("0.25").value();

    check(one.raw_value() == 4096, "one raw");
    check(half.raw_value() == 2048, "half raw");
    check((one + half).raw_value() == 6144, "add");
    check((one - half).raw_value() == 2048, "sub");
    check((half * half).raw_value() == quarter.raw_value(), "mul");
    check((half / gfxp::Fixed::from_int(2)).raw_value() == quarter.raw_value(), "div");
}

void test_decimal_parsing() {
    check(gfxp::Fixed::from_decimal("0").value().raw_value() == 0, "parse zero");
    check(gfxp::Fixed::from_decimal("1.5").value().raw_value() == 6144, "parse 1.5");
    check(gfxp::Fixed::from_decimal("-0.25").value().raw_value() == -1024, "parse -0.25");
    check(gfxp::Fixed::from_decimal(".125").value().raw_value() == 512, "parse .125");
    check(gfxp::Fixed::from_decimal("0.001").value().raw_value() == 4, "parse 0.001");
    check(gfxp::Fixed::from_decimal("0.0004").value().raw_value() == 2, "parse 0.0004");
    check(gfxp::Fixed::from_decimal("0.0005").value().raw_value() == 2, "parse 0.0005");
    check(!gfxp::Fixed::from_decimal("").has_value(), "reject empty");
    check(!gfxp::Fixed::from_decimal("abc").has_value(), "reject alpha");
    check(!gfxp::Fixed::from_decimal("1.2.3").has_value(), "reject double dot");
}

void test_rounding() {
    const gfxp::Fixed plus = gfxp::Fixed::from_raw(6144);
    const gfxp::Fixed minus = gfxp::Fixed::from_raw(-6144);
    const gfxp::Fixed tiny_negative = gfxp::Fixed::from_raw(-1);

    check(plus.trunc_int() == 1, "plus trunc");
    check(plus.floor_int() == 1, "plus floor");
    check(plus.ceil_int() == 2, "plus ceil");
    check(plus.round_int() == 2, "plus round");

    check(minus.trunc_int() == -1, "minus trunc");
    check(minus.floor_int() == -2, "minus floor");
    check(minus.ceil_int() == -1, "minus ceil");
    check(minus.round_int() == -2, "minus round");

    check(tiny_negative.floor_int() == -1, "tiny negative floor");
    check(tiny_negative.ceil_int() == 0, "tiny negative ceil");
}

void test_vec2_integration() {
    gfxp::Vec2 pos = gfxp::Vec2::from_pixels(10, 20);
    gfxp::Vec2 vel{gfxp::Fixed::from_decimal("0.25").value(),
                   gfxp::Fixed::from_decimal("-0.125").value()};
    const gfxp::Vec2 acc{gfxp::Fixed::from_decimal("0.001").value(),
                         gfxp::Fixed::from_decimal("0.002").value()};

    for (int i = 0; i < 1000; ++i) {
        vel += acc;
        pos += vel;
    }

    check(pos.x.raw_value() == 3066960, "integrated x raw");
    check(pos.y.raw_value() == 3573920, "integrated y raw");
}

void test_hash_and_raw_roundtrip() {
    const gfxp::Fixed value = gfxp::Fixed::from_decimal("12.375").value();
    const int32_t raw = value.raw_value();
    const gfxp::Fixed restored = gfxp::Fixed::from_raw(raw);
    check(value == restored, "raw roundtrip");
    check(std::hash<gfxp::Fixed>{}(value) == std::hash<int32_t>{}(raw), "hash raw");
}

void test_fixed_casts_and_pixel_helpers() {
    const gfxp::Fixed12 one_and_half = gfxp::Fixed12::from_decimal("1.5").value();
    const gfxp::Fixed8 down = gfxp::fixed_cast<gfxp::Fixed8>(one_and_half);
    const gfxp::Fixed16 up = gfxp::fixed_cast<gfxp::Fixed16>(one_and_half);

    check(down.raw_value() == 384, "fixed12 to fixed8");
    check(up.raw_value() == 98304, "fixed12 to fixed16");
    check(one_and_half.to_pixels_floor() == 1, "pixels floor");
    check(one_and_half.to_pixels_ceil() == 2, "pixels ceil");
    check(one_and_half.to_pixels_round() == 2, "pixels round");
    check(one_and_half.to_pixels_trunc() == 1, "pixels trunc");

    const gfxp::Fixed12 tiny = gfxp::Fixed12::from_raw(7);
    check(gfxp::fixed_cast<gfxp::Fixed8>(tiny, gfxp::Rounding::TowardZero).raw_value() == 0,
          "fixed cast toward zero");
    check(gfxp::fixed_cast<gfxp::Fixed8>(tiny, gfxp::Rounding::Nearest).raw_value() == 0,
          "fixed cast nearest down");
    check(gfxp::fixed_cast<gfxp::Fixed8>(gfxp::Fixed12::from_raw(8), gfxp::Rounding::Nearest)
                  .raw_value() == 1,
          "fixed cast nearest up");
}

void test_vec2_helpers() {
    const gfxp::Vec2 a = gfxp::Vec2::from_pixels(3, -4);
    const gfxp::Vec2 b{
        gfxp::Fixed::from_decimal("0.5").value(),
        gfxp::Fixed::from_decimal("2.0").value(),
    };

    check((a + b).x.raw_value() == 14336, "vec add x");
    check((a + b).y.raw_value() == -8192, "vec add y");
    check(gfxp::dot(a, a).raw_value() == gfxp::Fixed::from_int(25).raw_value(), "vec dot");
    check(gfxp::length_sq(a).raw_value() == gfxp::Fixed::from_int(25).raw_value(), "vec length sq");
    check(gfxp::manhattan_length(a).raw_value() == gfxp::Fixed::from_int(7).raw_value(),
          "vec manhattan");
    check(gfxp::abs(a) == gfxp::Vec2::from_pixels(3, 4), "vec abs");

    const gfxp::Vec2_8 a8 = gfxp::vec2_cast<gfxp::Vec2_8>(a);
    check(a8.x.raw_value() == 768, "vec cast x");
    check(a8.y.raw_value() == -1024, "vec cast y");
}

void test_aabb_helpers() {
    const gfxp::Aabb a = gfxp::Aabb::from_corners(gfxp::Vec2::from_pixels(0, 0),
                                                  gfxp::Vec2::from_pixels(8, 8));
    const gfxp::Aabb b = gfxp::Aabb::from_pos_size(gfxp::Vec2::from_pixels(7, 4),
                                                   gfxp::Vec2::from_pixels(4, 4));
    const gfxp::Aabb c = gfxp::translate(a, gfxp::Vec2::from_pixels(16, 0));

    check(gfxp::aabbs_intersect(a, b), "aabb intersect");
    check(!gfxp::aabbs_intersect(a, c), "aabb separated");
    check(a.size() == gfxp::Vec2::from_pixels(8, 8), "aabb size");
    check(a.center() == gfxp::Vec2::from_pixels(4, 4), "aabb center");
    check(gfxp::min_displacement(a, c) == gfxp::Vec2::from_pixels(8, 0),
          "aabb min displacement");

    const gfxp::Aabb_8 packed = gfxp::aabb_cast<gfxp::Aabb_8>(a);
    check(packed.br.x.raw_value() == 2048, "aabb cast br x");
    check(packed.br.y.raw_value() == 2048, "aabb cast br y");
}

} // namespace

int main() {
    test_basic_arithmetic();
    test_decimal_parsing();
    test_rounding();
    test_vec2_integration();
    test_hash_and_raw_roundtrip();
    test_fixed_casts_and_pixel_helpers();
    test_vec2_helpers();
    test_aabb_helpers();

    if (g_failures != 0) {
        std::cerr << g_failures << " test failure(s)\n";
        return EXIT_FAILURE;
    }

    std::cout << "gfxp core tests passed\n";
    return EXIT_SUCCESS;
}
