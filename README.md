# Auto Level Generator — v1.5.0

A procedural Geometry Dash editor mod that generates gameplay layouts from the **AI Gen** button.

## Presets

- **Regular** — broad procedural variety.
- **Hard** — tighter mixed gameplay with frequent precision checks.
- **Extreme** — sustained high difficulty with precision, ship, orb, speed and chokepoint sections.
- **Top 1 Style** — an escalating macro structure intended to create very difficult, precision-heavy layouts. It is a target/style preset, not a claim that every generated level is actually Top 1 or verified.
- **Impossible** — generates deliberately impassable wall sections. This is an intentionally impossible creative preset, not a human-verification or collision-proof guarantee.

## Changes in v1.5.0

- Adds a dedicated Impossible preset with deliberately blocked sections.
- Fixes the advanced difficulty ramp so each segment ramps from the configured base difficulty instead of compounding from the previous segment.

## Reliability improvements carried forward from v1.4.0

- Validates the editor layer before generation.
- Handles failed object creation without dereferencing a null object.
- Stops safely after repeated object-creation failures instead of continuing into an incompatible object set.
- Reports generation failure to the user instead of silently pretending the layout completed.
- Gives generated UI nodes stable mod-prefixed IDs.
- Keeps seeded generation reproducible when Seed is non-zero.
- Keeps the generation logic deterministic for a given seed/settings combination.
- Preserves the anti-repeat and difficulty-ramp systems from v1.3.0.

## Settings

- **Seed (0 = random)** — use a non-zero seed to reproduce a layout.
- **Number of segments** — controls generated length.
- **Difficulty (1-5)** — base difficulty for the regular/hard presets; advanced presets apply their own minimum/ramp.
- **Add decoration** — enables background/structural decoration.
- **Avoid repeating sections** — prevents adjacent sections from using the same generator type.

## Compatibility

The project is currently pinned to the **Geode 5.11.2 / Geometry Dash 2.2081** target because native 2.209 support is not yet available. Geode v5.11.2 contains some 2.209 forward-compatibility fixes but is still primarily a 2.2081 release.

When official 2.209 SDK/loader support is released, update `mod.json` and re-run the Android64 build. Do not label this package 2.209-compatible before that build has been compiled and tested on 2.209.

## Android64 build

The included GitHub Actions workflow builds an Android64 `.geode` remotely, so the project can be prepared from a Chromebook. The build action targets Android64 and uploads the resulting `.geode` as a workflow artifact.

## Testing checklist before release

1. Build against the exact supported Geode SDK.
2. Install on the matching Geometry Dash version.
3. Open the editor and verify the AI Gen button appears once.
4. Generate each preset on a blank level.
5. Generate with decoration on and off.
6. Test Seed 0 and a fixed non-zero seed.
7. Test the minimum and maximum segment settings.
8. Test generation after changing editor zoom/window size.
9. Confirm no crash when object creation fails or the target object set differs.
10. For 2.209, repeat the complete checklist after official 2.209 support lands.
