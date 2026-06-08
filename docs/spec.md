# gfxp Specification

## Purpose

`gfxp` is a tiny fixed-point math library for deterministic game simulation.

The motivating use case is lockstep multiplayer: every machine receives the
same inputs and must simulate the same gameplay state. In that context,
positions, velocities, collision decisions, and hashes need stable integer
semantics instead of platform-sensitive floating-point drift.

The library should stay boring:

- Plain value types.
- Explicit rounding.
- Explicit float interop.
- Integer raw storage for hashing and serialization.
- No renderer, engine, SDL, ImGui, networking, or allocator dependency.

## Responsibilities

`gfxp` is responsible for:

- Fixed-point scalar values.
- Small 2D vector helpers built from fixed-point values.
- Small 2D AABB helpers built from fixed-point vectors.
- Deterministic decimal parsing for authored constants.
- Explicit rounding to integer pixels.
- Stable raw-value access for serialization and hashing.
- A representative benchmark for game-like movement loops.

## Non-Responsibilities

`gfxp` is not responsible for:

- Rendering math.
- Camera smoothing.
- UI layout.
- Audio.
- Visual particle interpolation.
- Broad linear algebra.
- Trigonometry-heavy gameplay.
- Hidden global scale configuration.
- Physics engine ownership.

Host games decide which gameplay fields need fixed-point semantics.

## Scale

The default scale is:

```text
1 pixel = 4096 subpixels
```

That gives about `0.000244140625` pixel precision. It is enough for normal
hand-authored gameplay constants in Splonks, where one to three decimal places
dominate.

The scale is fixed at compile time:

```cpp
gfxp::Fixed::frac_bits == 12
gfxp::Fixed::scale == 4096
```

Scale-specific aliases are available for tests and benchmarks:

```cpp
gfxp::Fixed8;   // 1/256 px
gfxp::Fixed10;  // 1/1024 px
gfxp::Fixed12;  // 1/4096 px, default
gfxp::Fixed16;  // 1/65536 px
```

## Storage

`Fixed` stores one signed 32-bit raw integer:

```cpp
actual_value = raw / 4096
```

Multiplication and division use signed 64-bit intermediates. Callers that need
larger worlds should audit raw range before using this type.

## Conversion

Converting between fixed scales is explicit:

```cpp
gfxp::Fixed12 value = gfxp::Fixed12::from_decimal("1.5").value();
gfxp::Fixed8 compact = gfxp::fixed_cast<gfxp::Fixed8>(value);
```

Named rounding is available when reducing precision:

```cpp
gfxp::fixed_cast<gfxp::Fixed8>(value, gfxp::Rounding::TowardZero);
gfxp::fixed_cast<gfxp::Fixed8>(value, gfxp::Rounding::Nearest);
```

Checked variants return `std::nullopt` on overflow.

## Vector Helpers

`BasicVec2<FixedT>` provides small deterministic vector helpers:

```cpp
gfxp::Vec2 pos = gfxp::Vec2::from_pixels(10, 20);
gfxp::Vec2_8 compact = gfxp::vec2_cast<gfxp::Vec2_8>(pos);
```

Available aliases:

```cpp
gfxp::Vec2;     // BasicVec2<Fixed>
gfxp::Vec2_8;   // BasicVec2<Fixed8>
gfxp::Vec2_10;  // BasicVec2<Fixed10>
gfxp::Vec2_12;  // BasicVec2<Fixed12>
gfxp::Vec2_16;  // BasicVec2<Fixed16>
```

Helpers include `dot`, `length_sq`, `manhattan_length`, component `min`, `max`,
`clamp`, and `abs`. There is no square root or normalize helper yet.

## AABB Helpers

`BasicAabb<FixedT>` provides deterministic axis-aligned bounds helpers for
game-space collision and query code:

```cpp
gfxp::Aabb body = gfxp::Aabb::from_corners(
    gfxp::Vec2::from_pixels(0, 0),
    gfxp::Vec2::from_pixels(8, 8));

bool hit = gfxp::aabbs_intersect(body, other);
gfxp::Vec2 gap = gfxp::min_displacement(body, other);
```

Available aliases mirror the vector aliases:

```cpp
gfxp::Aabb;     // BasicAabb<Fixed>
gfxp::Aabb_8;   // BasicAabb<Fixed8>
gfxp::Aabb_10;  // BasicAabb<Fixed10>
gfxp::Aabb_12;  // BasicAabb<Fixed12>
gfxp::Aabb_16;  // BasicAabb<Fixed16>
```

Helpers include `translate`, `aabbs_intersect`, `min_displacement`, `size`,
`center`, and explicit scale casts.

## Rounding

Rounding is explicit:

- `TowardZero`
- `Floor`
- `Ceil`
- `Nearest`

`Nearest` rounds half away from zero. This is simple to reason about and does not
depend on processor floating-point modes.

## Decimal Parsing

`Fixed::from_decimal` parses decimal text with integer math. It does not convert
through `float` or `double`.

Examples:

```text
"1.5"   -> 6144 raw
"0.125" -> 512 raw
"-0.25" -> -1024 raw
```

Invalid input returns `std::nullopt`.

## Float Interop

Float conversion exists for debug/render boundaries:

```cpp
float draw_x = pos.x.to_float();
```

Constructing from float is explicit and should not be used for authoritative
gameplay constants unless the caller has accepted that conversion boundary.

Authoritative constants should prefer integers, ratios, or decimal strings.

## Hashing And Serialization

The stable representation is `raw_value()`.

Network hashes and serialized gameplay state should use the raw integer, not
`to_float()`.

## Splonks Usage Direction

Splonks should not integrate `gfxp` until this repository has working tests and
benchmarks.

The first likely Splonks migration targets are:

- entity position
- entity velocity
- entity acceleration
- collision stepping
- raw fixed-point hashes in desync fingerprints

Rendering should convert fixed-point positions to float at the render boundary.
