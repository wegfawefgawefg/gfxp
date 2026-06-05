# gfxp

<p>
  <img src="assets/logo.svg" alt="gfxp logo" width="96" height="96">
</p>

`gfxp` is a small C++20 fixed-point math library for deterministic game
simulation.

The first target is simple 2D game physics that needs to behave the same on
different machines. `gfxp` stores values as signed integer subpixels. The default
scale is 4096 subpixels per pixel.

`gfxp` is not a broad math framework. It does not own vectors beyond the small
helpers needed for game-space positions and velocities, and it does not replace
rendering, camera, audio, UI, or visual interpolation floats.

## Targets

- `gfxp::gfxp`: header-only fixed-point values, vector helpers, rounding,
  parsing, hashing, and serialization helpers.

## Add To A Project

The intended integration path is vendored source with CMake `add_subdirectory`.

```text
third_party/
  gfxp/
```

```cmake
add_subdirectory(third_party/gfxp)
target_link_libraries(my_game PRIVATE gfxp::gfxp)
```

Because the core is header-only, a game can also copy `include/gfxp` directly.

The default fixed type is:

```cpp
using gfxp::Fixed; // int32_t raw, 12 fractional bits
```

Scale-specific aliases are available for experiments:

```cpp
gfxp::Fixed8;
gfxp::Fixed10;
gfxp::Fixed12;
gfxp::Fixed16;
```

## Basic Use

```cpp
gfxp::Fixed x = gfxp::Fixed::from_int(12);
gfxp::Fixed v = gfxp::Fixed::from_decimal("0.125").value();

x += v;

int pixel = x.floor_int();
int32_t stable_bits = x.raw_value();
```

Vector helpers are intentionally small:

```cpp
gfxp::Vec2 pos = gfxp::Vec2::from_pixels(10, 20);
gfxp::Vec2 vel{gfxp::Fixed::from_decimal("0.25").value(), gfxp::Fixed::zero()};

pos += vel;
```

## Build

```sh
./scripts/build.sh
```

The build script configures the default development preset and runs the core
tests through CTest.

## Benchmarks

```sh
./scripts/bench.sh
```

The benchmark compares float, double, and fixed-point arithmetic in a small
Splonks-like movement loop. It is a sanity check, not a final engine profiler.

See [docs/spec.md](docs/spec.md) for goals, rounding rules, serialization, and
determinism boundaries.
