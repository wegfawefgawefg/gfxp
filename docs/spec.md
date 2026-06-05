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

The initial scale is:

```text
1 pixel = 1024 subpixels
```

That gives about `0.0009765625` pixel precision. It is enough for normal
hand-authored gameplay constants in Splonks, where one to three decimal places
dominate.

The scale is fixed at compile time:

```cpp
gfxp::Fixed::frac_bits == 10
gfxp::Fixed::scale == 1024
```

## Storage

`Fixed` stores one signed 32-bit raw integer:

```cpp
actual_value = raw / 1024
```

Multiplication and division use signed 64-bit intermediates. Callers that need
larger worlds should audit raw range before using this type.

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
"1.5"   -> 1536 raw
"0.125" -> 128 raw
"-0.25" -> -256 raw
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
