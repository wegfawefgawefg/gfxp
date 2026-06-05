#include <cstdint>
#include <gfxp/gfxp.hpp>
#include <iostream>

#if defined(_MSC_VER)
#define GFXP_NOINLINE __declspec(noinline)
#else
#define GFXP_NOINLINE __attribute__((noinline))
#endif

namespace {

volatile std::int32_t g_a_raw = 50176;
volatile std::int32_t g_b_raw = 2048;

GFXP_NOINLINE gfxp::Fixed12 add12(gfxp::Fixed12 lhs, gfxp::Fixed12 rhs) {
    return lhs + rhs;
}

GFXP_NOINLINE gfxp::Fixed12 mul12(gfxp::Fixed12 lhs, gfxp::Fixed12 rhs) {
    return lhs * rhs;
}

GFXP_NOINLINE gfxp::Fixed12 div12(gfxp::Fixed12 lhs, gfxp::Fixed12 rhs) {
    return lhs / rhs;
}

GFXP_NOINLINE int floor12(gfxp::Fixed12 value) {
    return value.floor_int();
}

} // namespace

int main() {
    const gfxp::Fixed12 a = gfxp::Fixed12::from_raw(g_a_raw);
    const gfxp::Fixed12 b = gfxp::Fixed12::from_raw(g_b_raw);
    const gfxp::Fixed12 c = add12(a, b) + mul12(a, b) + div12(a, b);
    std::cout << c.raw_value() << " " << floor12(c) << "\n";
    return 0;
}
