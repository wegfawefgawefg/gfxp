#include <gfxp/gfxp.hpp>
#include <iostream>

int main() {
    gfxp::Vec2 pos = gfxp::Vec2::from_pixels(10, 20);
    gfxp::Vec2 vel{gfxp::Fixed::from_decimal("0.25").value(), gfxp::Fixed::zero()};

    pos += vel;

    std::cout << "x raw: " << pos.x.raw_value() << "\n";
    std::cout << "x float boundary: " << pos.x.to_float() << "\n";
    return 0;
}
