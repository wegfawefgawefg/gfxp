# gfxp Ergonomics Plan

`gfxp` should be easy to copy into a game without making fixed-point migration
feel like a type-system tax.

The library still needs to stay small. Every helper should either make
determinism clearer, reduce conversion mistakes, or remove boilerplate from
ordinary game physics.

## Goals

- Keep the default type obvious: `gfxp::Fixed` is `Fixed12`.
- Let benchmarks and experiments use `Fixed8`, `Fixed10`, `Fixed12`, and
  `Fixed16` without duplicating vector code.
- Make conversion between fixed scales explicit.
- Make pixel conversion names read like game code.
- Keep raw access obvious for hashing and serialization.
- Add deterministic vector helpers needed by platformer physics.
- Avoid adding approximate trig/length behavior before we design it carefully.

## Tasks

- [x] Add `BasicVec2<FixedT>`.
- [x] Keep `Vec2` as `BasicVec2<Fixed>`.
- [x] Add fixed-to-fixed casting with explicit rounding.
- [x] Add pixel-named helpers.
- [x] Add vector dot, length-squared, component min/max/clamp, and abs/sign
      helpers.
- [x] Add tests for all conversion and vector helpers.
- [x] Keep render-boundary float conversion explicit.

## Non-Goals

- No `sqrt` or normalize yet.
- No trig tables yet.
- No saturation policy yet.
- No Splonks integration in this repo pass.
