#include <algorithm>
#include <chrono>
#include <cstdint>
#include <gfxp/gfxp.hpp>
#include <iostream>
#include <string_view>
#include <vector>

namespace {

template <typename Function> double time_seconds(Function&& function) {
    const auto start = std::chrono::steady_clock::now();
    function();
    const auto end = std::chrono::steady_clock::now();
    return std::chrono::duration<double>(end - start).count();
}

struct FloatEnt {
    float px;
    float py;
    float vx;
    float vy;
    float ax;
    float ay;
};

struct DoubleEnt {
    double px;
    double py;
    double vx;
    double vy;
    double ax;
    double ay;
};

struct FixedEnt {
    gfxp::Vec2 pos;
    gfxp::Vec2 vel;
    gfxp::Vec2 acc;
};

void print_result(std::string_view name, double seconds, std::uint64_t checksum, int entities,
                  int ticks) {
    const double updates = static_cast<double>(entities) * static_cast<double>(ticks);
    const double ns_per_update = seconds * 1'000'000'000.0 / updates;
    std::cout << name << ": " << seconds << "s, " << ns_per_update << " ns/entity-tick, checksum "
              << checksum << "\n";
}

void print_ops_result(std::string_view name, double seconds, std::uint64_t checksum,
                      int iterations) {
    const double ns_per_op = seconds * 1'000'000'000.0 / static_cast<double>(iterations);
    std::cout << name << ": " << seconds << "s, " << ns_per_op << " ns/op, checksum " << checksum
              << "\n";
}

} // namespace

int main() {
    constexpr int entities = 20000;
    constexpr int ticks = 2000;

    std::vector<FloatEnt> floats;
    std::vector<DoubleEnt> doubles;
    std::vector<FixedEnt> fixeds;
    floats.reserve(entities);
    doubles.reserve(entities);
    fixeds.reserve(entities);

    for (int i = 0; i < entities; ++i) {
        const float base = static_cast<float>(i % 251);
        floats.push_back(FloatEnt{base, base * 0.5F, 0.25F, -0.125F, 0.001F, 0.002F});
        doubles.push_back(DoubleEnt{base, base * 0.5, 0.25, -0.125, 0.001, 0.002});
        fixeds.push_back(FixedEnt{
            gfxp::Vec2{gfxp::Fixed::from_int(i % 251), gfxp::Fixed::from_ratio(i % 251, 2).value()},
            gfxp::Vec2{gfxp::Fixed::from_decimal("0.25").value(),
                       gfxp::Fixed::from_decimal("-0.125").value()},
            gfxp::Vec2{gfxp::Fixed::from_decimal("0.001").value(),
                       gfxp::Fixed::from_decimal("0.002").value()},
        });
    }

    std::uint64_t float_checksum = 0;
    const double float_seconds = time_seconds([&]() {
        for (int tick = 0; tick < ticks; ++tick) {
            for (FloatEnt& ent : floats) {
                ent.vx += ent.ax;
                ent.vy += ent.ay;
                ent.vx *= 0.999F;
                ent.vy *= 0.999F;
                ent.px += ent.vx;
                ent.py += ent.vy;
                if (ent.px < 0.0F || ent.px > 2048.0F)
                    ent.vx = -ent.vx;
                if (ent.py < 0.0F || ent.py > 2048.0F)
                    ent.vy = -ent.vy;
            }
        }
        for (const FloatEnt& ent : floats)
            float_checksum += static_cast<std::uint64_t>(ent.px * 1000.0F);
    });

    std::uint64_t double_checksum = 0;
    const double double_seconds = time_seconds([&]() {
        for (int tick = 0; tick < ticks; ++tick) {
            for (DoubleEnt& ent : doubles) {
                ent.vx += ent.ax;
                ent.vy += ent.ay;
                ent.vx *= 0.999;
                ent.vy *= 0.999;
                ent.px += ent.vx;
                ent.py += ent.vy;
                if (ent.px < 0.0 || ent.px > 2048.0)
                    ent.vx = -ent.vx;
                if (ent.py < 0.0 || ent.py > 2048.0)
                    ent.vy = -ent.vy;
            }
        }
        for (const DoubleEnt& ent : doubles)
            double_checksum += static_cast<std::uint64_t>(ent.px * 1000.0);
    });

    std::uint64_t fixed_checksum = 0;
    const gfxp::Fixed damping = gfxp::Fixed::from_decimal("0.999").value();
    const gfxp::Fixed low = gfxp::Fixed::zero();
    const gfxp::Fixed high = gfxp::Fixed::from_int(2048);
    const double fixed_seconds = time_seconds([&]() {
        for (int tick = 0; tick < ticks; ++tick) {
            for (FixedEnt& ent : fixeds) {
                ent.vel += ent.acc;
                ent.vel *= damping;
                ent.pos += ent.vel;
                if (ent.pos.x < low || ent.pos.x > high)
                    ent.vel.x = -ent.vel.x;
                if (ent.pos.y < low || ent.pos.y > high)
                    ent.vel.y = -ent.vel.y;
            }
        }
        for (const FixedEnt& ent : fixeds)
            fixed_checksum += static_cast<std::uint64_t>(ent.pos.x.raw_value());
    });

    std::cout << "entities: " << entities << ", ticks: " << ticks << "\n";
    std::cout << "fixed scale: " << gfxp::Fixed::scale << " subpixels/pixel\n";
    print_result("float", float_seconds, float_checksum, entities, ticks);
    print_result("double", double_seconds, double_checksum, entities, ticks);
    print_result("fixed", fixed_seconds, fixed_checksum, entities, ticks);

    constexpr int primitive_count = 32768;
    constexpr int primitive_passes = 2048;
    constexpr int primitive_ops = primitive_count * primitive_passes;
    std::cout << "primitive ops: " << primitive_ops << "\n";

    std::vector<float> float_values(static_cast<std::size_t>(primitive_count), 1.0F);
    std::vector<double> double_values(static_cast<std::size_t>(primitive_count), 1.0);
    std::vector<gfxp::Fixed> fixed_values(static_cast<std::size_t>(primitive_count),
                                          gfxp::Fixed::from_int(1));

    const auto checksum_floats = [&]() {
        std::uint64_t checksum = 0;
        for (float value : float_values)
            checksum += static_cast<std::uint64_t>(value * 1000.0F);
        return checksum;
    };
    const auto checksum_doubles = [&]() {
        std::uint64_t checksum = 0;
        for (double value : double_values)
            checksum += static_cast<std::uint64_t>(value * 1000.0);
        return checksum;
    };
    const auto checksum_fixed = [&]() {
        std::uint64_t checksum = 0;
        for (gfxp::Fixed value : fixed_values)
            checksum += static_cast<std::uint64_t>(value.raw_value());
        return checksum;
    };

    const double float_add_seconds = time_seconds([&]() {
        for (int pass = 0; pass < primitive_passes; ++pass) {
            for (float& value : float_values)
                value += 1.0F / static_cast<float>(gfxp::Fixed::scale);
        }
    });
    print_ops_result("float add", float_add_seconds, checksum_floats(), primitive_ops);

    const double double_add_seconds = time_seconds([&]() {
        for (int pass = 0; pass < primitive_passes; ++pass) {
            for (double& value : double_values)
                value += 1.0 / static_cast<double>(gfxp::Fixed::scale);
        }
    });
    print_ops_result("double add", double_add_seconds, checksum_doubles(), primitive_ops);

    const gfxp::Fixed fixed_one_raw = gfxp::Fixed::from_raw(1);
    const double fixed_add_seconds = time_seconds([&]() {
        for (int pass = 0; pass < primitive_passes; ++pass) {
            for (gfxp::Fixed& value : fixed_values)
                value += fixed_one_raw;
        }
    });
    print_ops_result("fixed add", fixed_add_seconds, checksum_fixed(), primitive_ops);

    std::fill(float_values.begin(), float_values.end(), 1000.0F);
    std::fill(double_values.begin(), double_values.end(), 1000.0);
    std::fill(fixed_values.begin(), fixed_values.end(), gfxp::Fixed::from_int(1000));

    const double float_mul_seconds = time_seconds([&]() {
        for (int pass = 0; pass < primitive_passes; ++pass) {
            for (float& value : float_values)
                value *= 0.999F;
        }
    });
    print_ops_result("float mul", float_mul_seconds, checksum_floats(), primitive_ops);

    const double double_mul_seconds = time_seconds([&]() {
        for (int pass = 0; pass < primitive_passes; ++pass) {
            for (double& value : double_values)
                value *= 0.999;
        }
    });
    print_ops_result("double mul", double_mul_seconds, checksum_doubles(), primitive_ops);

    const gfxp::Fixed fixed_mul_factor = gfxp::Fixed::from_decimal("0.999").value();
    const double fixed_mul_seconds = time_seconds([&]() {
        for (int pass = 0; pass < primitive_passes; ++pass) {
            for (gfxp::Fixed& value : fixed_values)
                value *= fixed_mul_factor;
        }
    });
    print_ops_result("fixed mul", fixed_mul_seconds, checksum_fixed(), primitive_ops);

    std::fill(float_values.begin(), float_values.end(), 1000.0F);
    std::fill(double_values.begin(), double_values.end(), 1000.0);
    std::fill(fixed_values.begin(), fixed_values.end(), gfxp::Fixed::from_int(1000));

    const double float_div_seconds = time_seconds([&]() {
        for (int pass = 0; pass < primitive_passes; ++pass) {
            for (float& value : float_values)
                value /= 1.001F;
        }
    });
    print_ops_result("float div", float_div_seconds, checksum_floats(), primitive_ops);

    const double double_div_seconds = time_seconds([&]() {
        for (int pass = 0; pass < primitive_passes; ++pass) {
            for (double& value : double_values)
                value /= 1.001;
        }
    });
    print_ops_result("double div", double_div_seconds, checksum_doubles(), primitive_ops);

    const gfxp::Fixed fixed_divisor = gfxp::Fixed::from_decimal("1.001").value();
    const double fixed_div_seconds = time_seconds([&]() {
        for (int pass = 0; pass < primitive_passes; ++pass) {
            for (gfxp::Fixed& value : fixed_values)
                value /= fixed_divisor;
        }
    });
    print_ops_result("fixed div", fixed_div_seconds, checksum_fixed(), primitive_ops);

    return 0;
}
