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

template <typename FixedT> struct FixedEnt {
    FixedT px;
    FixedT py;
    FixedT vx;
    FixedT vy;
    FixedT ax;
    FixedT ay;
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

template <typename FixedT>
std::pair<double, std::uint64_t> run_fixed_movement_bench(int entities, int ticks) {
    std::vector<FixedEnt<FixedT>> fixeds;
    fixeds.reserve(static_cast<std::size_t>(entities));

    for (int i = 0; i < entities; ++i) {
        fixeds.push_back(FixedEnt<FixedT>{
            FixedT::from_int(i % 251),
            FixedT::from_ratio(i % 251, 2).value(),
            FixedT::from_decimal("0.25").value(),
            FixedT::from_decimal("-0.125").value(),
            FixedT::from_decimal("0.001").value(),
            FixedT::from_decimal("0.002").value(),
        });
    }

    std::uint64_t checksum = 0;
    const FixedT damping = FixedT::from_decimal("0.999").value();
    const FixedT low = FixedT::zero();
    const FixedT high = FixedT::from_int(2048);
    const double seconds = time_seconds([&]() {
        for (int tick = 0; tick < ticks; ++tick) {
            for (FixedEnt<FixedT>& ent : fixeds) {
                ent.vx += ent.ax;
                ent.vy += ent.ay;
                ent.vx *= damping;
                ent.vy *= damping;
                ent.px += ent.vx;
                ent.py += ent.vy;
                if (ent.px < low || ent.px > high)
                    ent.vx = -ent.vx;
                if (ent.py < low || ent.py > high)
                    ent.vy = -ent.vy;
            }
        }
        for (const FixedEnt<FixedT>& ent : fixeds)
            checksum += static_cast<std::uint64_t>(ent.px.raw_value());
    });

    return {seconds, checksum};
}

} // namespace

int main() {
    constexpr int entities = 20000;
    constexpr int ticks = 2000;

    std::vector<FloatEnt> floats;
    std::vector<DoubleEnt> doubles;
    floats.reserve(entities);
    doubles.reserve(entities);

    for (int i = 0; i < entities; ++i) {
        const float base = static_cast<float>(i % 251);
        floats.push_back(FloatEnt{base, base * 0.5F, 0.25F, -0.125F, 0.001F, 0.002F});
        doubles.push_back(DoubleEnt{base, base * 0.5, 0.25, -0.125, 0.001, 0.002});
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

    const auto [fixed8_seconds, fixed8_checksum] =
        run_fixed_movement_bench<gfxp::Fixed8>(entities, ticks);
    const auto [fixed10_seconds, fixed10_checksum] =
        run_fixed_movement_bench<gfxp::Fixed10>(entities, ticks);
    const auto [fixed12_seconds, fixed12_checksum] =
        run_fixed_movement_bench<gfxp::Fixed12>(entities, ticks);
    const auto [fixed16_seconds, fixed16_checksum] =
        run_fixed_movement_bench<gfxp::Fixed16>(entities, ticks);

    std::cout << "entities: " << entities << ", ticks: " << ticks << "\n";
    std::cout << "default fixed scale: " << gfxp::Fixed::scale << " subpixels/pixel\n";
    print_result("float", float_seconds, float_checksum, entities, ticks);
    print_result("double", double_seconds, double_checksum, entities, ticks);
    print_result("fixed8", fixed8_seconds, fixed8_checksum, entities, ticks);
    print_result("fixed10", fixed10_seconds, fixed10_checksum, entities, ticks);
    print_result("fixed12", fixed12_seconds, fixed12_checksum, entities, ticks);
    print_result("fixed16", fixed16_seconds, fixed16_checksum, entities, ticks);

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
