# TH08 Mods Implementation Plan

- Status: active
- Started: 2026-09-01
- Web baseline: `th08-web` commit `3f926db`
- Semantic source reference: `th08` commit `50077ac`

## Goal

Build a deterministic, source-level modifier system for Touhou Eiyashou that
ships on the Web first, remains continuously buildable on Linux, and can be
ported to TH06 and TH07 by replacing a thin game adapter instead of rewriting
each modifier.

The first useful release lets a player select modifiers before launch, freezes
that configuration for the run, shows the active modifier set, and preserves
enough metadata to replay the same rules later.

## Product decisions

- The reconstructed C++ game remains the simulation authority. JavaScript is a
  launcher and browser boundary, never a second gameplay implementation.
- Modifiers are statically linked. Dynamic libraries, a scripting VM, and an
  online plugin store are outside the first design.
- The authored C++98 source and the C++17 mod runtime communicate through a
  narrow C ABI made only from fixed-width values and plain data structures.
- A run configuration is immutable after gameplay begins.
- Every behavior-changing modifier has a versioned ruleset such as `HD@1`.
- The unmodified build path remains available and must not change the exact VC7
  reconstruction claims or ledgers.
- Original DAT files remain user-supplied, local, and excluded from artifacts
  and persistent browser storage.
- Vanilla score remains authoritative for vanilla game progression. Modifier
  score, badges, and eligibility are separate results.

## Architecture

```text
Web launcher / Linux CLI
          |
          | RunConfigV1
          v
Game-independent mod core
registry, composition, manifest, run statistics
          |
          | stable C ABI and POD snapshots
          v
TH08 adapter
input, projectile observation, overlays, scheduler, game rules
          |
          v
Reconstructed TH08 engine
```

### Mod core

The core owns configuration validation, modifier ordering and conflicts,
versioned defaults, deterministic helper algorithms, run metadata, and a
static built-in registry. It does not include TH08 headers or retain pointers
to game objects.

### TH08 adapter

The adapter translates stable hook calls into TH08 concepts. Initial hooks are
limited to run lifecycle, effective input, projectile draw observation, and a
playfield overlay. Later hooks add simulation scheduling, audio rate, miss
commit, collision policy, and starting resources only when a modifier proves
that it needs them.

### Host bridge

The Web launcher passes one validated configuration to an exported C function
before `main`. The Linux host accepts the same logical configuration through a
CLI or a file. Host code never advances simulation or evaluates modifier rules.

## Initial modifier contract

| Modifier | Version 1 behavior | Primary seam | Rank policy |
| --- | --- | --- | --- |
| Hidden (`HD`) | Bullets remain visible for a delay and then fade by active age. Lasers and transition states are included. | Projectile draw observation | Ranked later |
| Flashlight (`FL`) | A configurable circular visible area follows the player while the playfield outside it is obscured. Native Stage 2 darkness composes with it. | High-priority playfield overlay | Ranked later |
| Autoshot (`AT`) | Shoot is held during active gameplay; menus retain vanilla input. | Effective-input filter | Unranked |
| Mirror (`MR`) | The playfield is mirrored horizontally and left/right input is exchanged. HUD is not mirrored. | Playfield post-process and input filter | Ranked later |
| No Fail (`NF`) | Misses and death animation still occur, but the final game-over transition is suppressed. | Miss-commit policy | Unranked |
| Double Time (`DT`) | Simulation and authored audio advance at the configured rate while presentation stays display-paced. | Frame scheduler and mixer rate | Ranked later |
| Hard Rock (`HR`) | A frozen policy changes movement, hurtbox, graze margin, projectile speed, and resources together. | Multiple gameplay policies | Unranked until balanced |
| Easy (`EZ`) | A frozen assistance policy changes the same explicit dimensions in the easier direction. | Multiple gameplay policies | Unranked |

`HR` and `EZ` conflict in version 1. Ninety-degree rotation is not part of
`MR@1` because the playfield aspect ratio and HUD make it a different feature.

## Determinism and replay contract

- Input modifiers operate after replay input is loaded and before the player
  update. Recording stores the resulting effective input.
- Playback of an effective-input replay does not apply the same input modifier
  a second time.
- Every modified replay is bound to a canonical manifest containing the game
  version, engine revision, mod API version, modifier versions, and normalized
  options.
- Missing or incompatible manifests fail explicitly instead of attempting a
  potentially desynchronized playback.
- Observation-only modifiers must not change a per-tick simulation hash.
- Simulation modifiers must produce the same checkpoints on Web and Linux for
  the same manifest and effective input stream.

The existing replay file layout will not be changed in the first slice. A
versioned sidecar or envelope will be selected after the core configuration is
running and can be serialized canonically.

## Save isolation

Modified runs must not pollute a player's vanilla configuration, score, or
replay namespace. The Web build will use a separate IDBFS namespace. Native
ports will gain an explicit save directory rather than depending on the retail
data directory for writable state.

## Delivery phases

### Phase 0: integration foundation

- [x] Preserve the `th08-web` Git history and record upstream remotes.
- [x] Add a versioned, statically linked no-op mod core.
- [x] Add unit tests for configuration defaults, validation, ABI compatibility,
  and lifecycle freeze.
- [x] Compile the core independently with a modern host compiler.
- [x] Link the no-op boundary into Web and Linux builds behind
  `TH08_MOD_BUILD`.
- [ ] Prove that the default configuration leaves authored behavior untouched.
- [ ] Define a separate save namespace before modified state is persisted.

### Phase 1: first playable vertical slice

- [ ] Add pre-launch Web controls for Hidden, Flashlight, and Autoshot.
- [ ] Pass `RunConfigV1` into Wasm before the game entry point.
- [ ] Filter gameplay input for Autoshot without changing menu input.
- [ ] Apply Hidden at the projectile rendering boundary without mutating
  persistent animation state.
- [ ] Compose Flashlight with the native Stage 2 darkness overlay.
- [ ] Display the frozen active modifier set and canonical identifier.
- [ ] Run browser smoke coverage and Linux compile coverage.

### Phase 2: composition and replay identity

- [ ] Add horizontal Mirror through a playfield post-process and input remap.
- [ ] Freeze modifier ordering and conflict validation.
- [ ] Persist and validate a canonical replay manifest.
- [ ] Add cross-platform checkpoint and visual replay tests.

### Phase 3: time domain

- [ ] Add a display-paced simulation accumulator for Double Time.
- [ ] Advance sound queues per simulation tick and scale the shared mixer rate.
- [ ] Validate authored-tick/present ratios and audio duration on Chromium,
  Firefox, and Linux.
- [ ] Define an overload policy that never silently drops deterministic ticks.

### Phase 4: gameplay policies

- [ ] Add No Fail at the miss/game-over commit boundary.
- [ ] Freeze explicit `HR@1` and `EZ@1` policy values.
- [ ] Cover bullet transformations, lasers, collision, graze, movement, and
  resources before enabling composite difficulty modifiers.
- [ ] Balance score multipliers only after measured gameplay runs.

### Phase 5: additional games and native releases

- [ ] Rebase or adapt the hook layer onto the current semantic TH08 source and
  portable 64-bit Linux lane.
- [ ] Implement a TH07 adapter against the same mod core contract.
- [ ] Implement a TH06 adapter after the TH07 boundary has exposed any false
  TH08-specific abstractions.

## First prototype gate

### Decision at stake

Can low-coupling modifiers share one static core while the original C++98 game
continues to build for both Wasm and Linux?

### Risky assumption

If a fixed-width C boundary is sufficient for configuration, input filtering,
and projectile/overlay observation, then Hidden, Flashlight, and Autoshot can
run without exposing TH08 object layouts to the core or changing simulation
state when visual modifiers are enabled.

### Borrowed parts

- The complete `th08-web` C++ to Wasm build and launcher lifecycle.
- The authored calculation/draw priority chains and replay input ordering.
- Bullet active timers and the existing bullet draw path.
- The native Stage 2 darkness renderer and its high-priority draw position.
- The shared SDL/Linux compatibility layer used by Web and native builds.

### Smallest experiment and signals

Build a no-op core first, then activate one input modifier and two observation
modifiers. Proceed only if:

1. core unit tests pass with a normal host compiler;
2. the authored Web target links with the boundary enabled;
3. the Linux target compiles from the same core sources;
4. the default config produces no new hook-side state changes;
5. Hidden and Flashlight leave the simulation hash unchanged; and
6. an Autoshot effective-input stream can be recorded and replayed without a
   second transformation.

Presentation polish, score balancing, replay storage format, and additional
games are deliberately outside this experiment.

### Current evidence

On 2026-09-01, the first boundary checkpoint configured and built on the host
with:

```bash
cmake -S . -B build/mod-core -G Ninja \
  -DTH08_MODCORE_ONLY=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build/mod-core --parallel 2
ctest --test-dir build/mod-core --output-on-failure
```

The C++17 runtime tests passed. A separate executable compiled as strict C++98
also linked against and exercised the same C ABI, which is the first direct
signal that the authored game and modern core can remain language-separated.

The same boundary then linked into the complete 32-bit Linux game and both
Emscripten presentation variants. `scripts/build-modern-linux.sh` completed a
53-step build, and `scripts/build-web-game.sh` produced the Chromium and Firefox
Wasm artifacts before passing the Web provenance allowlist. Starting the Linux
binary without retail data printed the initialized API version and zero
modifier mask, then followed the expected missing-DAT error path. No gameplay
hook is registered at this checkpoint, so the runtime is structurally a no-op;
replay and screenshot parity remain a later runtime gate. A separate complete
Linux build with `-DTH08_MOD_BUILD=OFF` also linked successfully and omitted the
core and adapter targets.

### Prototype debt gate

Before calling the slice production-ready, replace temporary configuration
plumbing, add explicit validation errors, isolate writable state, settle replay
manifest persistence, cover lasers and Stage 2 darkness composition, and run
real Chromium, Firefox, and Linux gameplay checks.

## Verification matrix

| Contract | Minimum evidence |
| --- | --- |
| Mod-off parity | Vanilla replay checkpoints and representative screenshots match the baseline. |
| Visual-only purity | Per-tick simulation hashes match with `HD`, `FL`, and `MR` enabled. |
| Input determinism | Recorded effective input replays without double application. |
| Time determinism | Equal simulation-tick counts reach equal state at `1.0x` and `DT`; only wall-clock presentation differs. |
| Cross-platform behavior | Web and Linux checkpoints match for one manifest and replay. |
| Rendering coverage | Stage 2 darkness, dialogue, screen shake, route transitions, ending, and dense bullet scenes have visual checks. |
| Performance | Per-projectile policy is allocation-free and measured at the 1,536-bullet engine limit. |
| Reconstruction isolation | Building without `TH08_MOD_BUILD` does not select mod source or alter exact-match ledgers. |

## Commit policy

Work lands as small, runnable checkpoints. Documentation and scaffolding,
standalone core behavior, each engine seam, each modifier, and host UI should be
separate commits whenever they can be verified independently. A checkpoint
must include the commands run, observed result, known limitation, and next
bounded step in this document or the relevant handoff.

## Explicit non-goals for the first release

- Runtime loading of arbitrary third-party binaries or scripts.
- An in-game modifier menu that replaces the host launcher.
- Online leaderboards, accounts, anti-cheat, or server authority.
- A generic serialized view of the entire game state.
- Ninety-degree playfield rotation.
- Final score multipliers before mechanics are stable and measured.
