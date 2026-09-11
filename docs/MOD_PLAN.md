# TH08 Mods Implementation Plan

- Status: active
- Started: 2026-09-01
- Web baseline: `th08-web` commit `e3485ab` (the mods fork began at
  `3f926db` and later imported its runtime-parity and sustained-performance
  fixes)
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

### Modifier source layout

Modifier behavior is organized as a vertical slice instead of accumulating in
the runtime or game adapter:

```text
src/mod/modifiers/<modifier>/
    <Modifier>Policy.*          portable, game-independent rule
    th08/Th08<Modifier>.*       TH08 state and hook translation
tests/mod/modifiers/<modifier>/
    <Modifier>Tests.*           black-box policy contract
```

`ModRuntime.cpp` owns only configuration, lifecycle, and stable C ABI dispatch.
`Th08ModAdapter.cpp` owns only lifecycle translation and modifier registration.
A shared helper is promoted out of a modifier directory only after at least two
modifiers need the same contract. TH06 and TH07 add sibling adapter directories
inside each modifier without copying its portable policy.

### Host bridge

The Web launcher passes one validated configuration to an exported C function
before `main`. The Linux host accepts the same logical selection with
`--mods=HD,FL,AT,MR,NF,DT,HR,EZ,RX,BS,NB` and `--mirror=90`; `--mod-manifest`
prints its normalized identity without requiring retail data. Host code never
advances simulation or evaluates modifier rules.

## Initial modifier contract

| Modifier | Version 1 behavior | Primary seam | Rank policy |
| --- | --- | --- | --- |
| Hidden (`HD`) | Bullets remain visible for a delay and then fade by active age. Lasers and transition states are included. | Projectile draw observation | Ranked later |
| Flashlight (`FL`) | A configurable circular visible area follows the player while the playfield outside it is obscured. Native Stage 2 darkness composes with it. | High-priority playfield overlay | Ranked later |
| Autoshot (`AT`) | Shoot is held during active gameplay; menus retain vanilla input. | Effective-input filter | Unranked |
| Mirror (`MR`) | The playfield supports horizontal/vertical reflection and 90°/180°/270° rotation. Direction input remains screen-relative, quarter turns aspect-fit without cropping, and HUD/UI remain upright. | Scoped vertex transform and effective-input filter | Ranked later |
| No Fail (`NF`) | Misses, death count, penalties, and respawning remain active; at zero lives the final retry transition is suppressed without underflowing the life counter. | Miss-commit policy | Unranked |
| Double Time (`DT`) | Simulation and authored audio advance at the configured rate while presentation stays display-paced. | Frame scheduler and mixer rate | Ranked later |
| Hard Rock (`HR`) | Enemy projectile speed is `23/20`, player movement is `9/10`, the hurtbox is `5/4`, and the graze margin is `4/5`; resources and spell time remain vanilla. | Multiple gameplay policies | Unranked until balanced |
| Easy (`EZ`) | Enemy projectile speed is `17/20`, the hurtbox is `3/4`, the graze margin is `5/4`, starting bombs gain `+2`, and spell time is `5/4`; player movement remains vanilla. | Multiple gameplay policies | Unranked |
| Relax (`RX`) | Shoot and Focus are held during active gameplay; movement and bombing remain manual, while menus, dialogue, blocked UI, and replay playback retain vanilla input. | Effective-input filter | Unranked |
| Blind Spot (`BS`) | Bullets and lasers are fully visible at 128 pixels, fade linearly while approaching, and are invisible within 48 pixels of the player. | Projectile draw observation | Ranked later |
| No Bomb (`NB`) | Bomb is suppressed during live active gameplay; menus, dialogue, blocked UI, and replay playback retain vanilla input. | Effective-input filter | Unranked |

`HR` and `EZ` conflict in version 1. `AT` and `RX` also conflict because Relax
is a strict superset of Autoshot's assistance. `MR@1` records its selected
transform in the manifest; 90° and 270° use deterministic letterboxing inside
the original playfield rather than cropping or rotating the HUD.

Difficulty transforms cross the C ABI as exact integer ratios. Game adapters
apply those ratios at their native seams, avoiding platform-dependent policy
constants and keeping the same `HR@1` identity portable to TH06 and TH07.

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
- [x] Define a separate save namespace before modified state is persisted.

### Phase 1: first playable vertical slice

- [x] Add pre-launch Web controls for all implemented modifiers.
- [x] Pass `RunConfigV1` into Wasm before the game entry point.
- [x] Filter gameplay input for Autoshot without changing menu input.
- [x] Apply Hidden at the projectile rendering boundary without mutating
  persistent animation state.
- [x] Compose Flashlight with the native Stage 2 darkness overlay.
- [x] Display the frozen active modifier set.
- [x] Display the canonical manifest identifier.
- [x] Run Linux compile and CLI coverage.
- [x] Run Linux gameplay smoke coverage with retail data.
- [x] Run Chromium gameplay smoke coverage with retail data.
- [x] Run Firefox gameplay smoke coverage with retail data.

### Phase 2: composition and replay identity

- [x] Add all five Mirror modes through a scoped playfield transform and input
  remap.
- [x] Freeze modifier ordering and conflict validation.
- [ ] Persist and validate a canonical replay manifest.
- [ ] Add cross-platform checkpoint and visual replay tests.

### Phase 3: time domain

- [x] Add a display-paced simulation accumulator for Double Time.
- [x] Advance sound queues per simulation tick and scale the shared mixer rate.
- [x] Validate authored-tick/present ratios on Chromium and Linux.
- [x] Validate Linux mixer-rate transitions across gameplay and pause.
- [x] Validate the authored-tick/present ratio on Firefox.
- [ ] Validate audible audio duration/pitch on hardware.
- [x] Define an overload policy that never silently drops deterministic ticks.

### Phase 4: gameplay policies

- [x] Add No Fail at the miss/game-over commit boundary.
- [x] Freeze explicit `HR@1` and `EZ@1` policy values.
- [x] Cover transformed bullets, lasers, collision, graze, and movement at
  their TH08 commit boundaries.
- [x] Apply the Easy starting-bomb and spell-time resource policies.
- [x] Add Relax as a portable effective-input policy and expose it to Web and
  native hosts.
- [x] Add Blind Spot as a distance-based observation policy for bullets and
  active laser segments.
- [x] Add No Bomb as a portable action-suppression policy.
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

The C++17 runtime tests passed. Separate executables compiled as strict C++98
also exercise the public C ABI and native CLI parser. These three test targets
run in the repository's target-independent CI, providing a direct signal that
the authored game and modern core remain language-separated.

The same boundary then linked into the complete 32-bit Linux game and both
Emscripten presentation variants. `scripts/build-modern-linux.sh` completed a
53-step build, and `scripts/build-web-game.sh` produced the Chromium and Firefox
Wasm artifacts before passing the Web provenance allowlist. Starting the Linux
binary without retail data printed the initialized API version and zero
modifier mask, then followed the expected missing-DAT error path. No gameplay
hook was registered at that boundary checkpoint, so it was structurally a
no-op. A separate complete Linux build with `-DTH08_MOD_BUILD=OFF` also linked
successfully and omitted the core and adapter targets.

The first gameplay checkpoints now add Autoshot at calculation priority 8 and
Hidden at the final bullet/laser draw boundary. Autoshot excludes replay
playback, menus, retry UI, and dialogue, and writes the effective action to the
input stream later consumed by the replay recorder. Hidden uses an
allocation-free integer fade, observes bullet transitions and complete laser
phase age, and restores each `AnmVm` color immediately after drawing. Core,
strict C++98 ABI, mod-enabled Linux, mod-disabled Linux, Chromium Wasm, Firefox
Wasm, JavaScript syntax, and Web provenance checks pass. Controlled replay and
mod-off screenshot parity remain runtime gates;
retail data is intentionally absent from the repository and build artifacts.

Flashlight reuses the native four-rectangle mask and ANM script 105 at the same
Ascii high-priority draw point. It owns a separate mask VM and draws after the
native Stage 2 layer, so standard alpha blending composes both visibility
constraints without rewriting the stage-controlled radius or opacity fields.

The portable core now emits a canonical, human-readable manifest in fixed
modifier order. It identifies TH08 v1.00d, the modern engine and source
baseline, the C API version, each enabled modifier ruleset, and normalized
options. The Web launcher asks Wasm for this value on every pre-launch
selection change and displays the exact frozen value instead of maintaining a
parallel JavaScript manifest implementation.

The same selection is available to native hosts through the reusable C++98
CLI bridge. Its parser is covered independently from the game, the complete
32-bit Linux target links it, and `--mod-manifest` exercises configuration and
identity output before any retail-data check. Both final Wasm artifacts are
loaded under Node during the Web release build and must return the expected
manifests for no modifiers, every individual modifier, all five Mirror modes,
and complete compatible sets using either Autoshot or Relax through the same
exported functions used by the launcher. This Wasm host smoke is supplemented
by the bounded Chromium gameplay check below; Firefox modifier gameplay
remains pending.

Mirror is implemented as its own vertical slice. Its portable policy owns
direction remapping, mode validation, geometry, state, and manifest identity.
The TH08 translation brackets only draw priorities 6 through 14 with a final
vertex transform, leaving Spellcard presentation and the priority-17 HUD
upright. Horizontal, vertical, and 180° modes retain the native playfield size;
90° and 270° use a 6/7 scale inside the 384×448 region and draw deterministic
black letterbox bars. A priority-3 reset sentinel prevents transform state from
leaking across an early draw-chain break. The shared priority-8 input adapter
now serves both Autoshot and Mirror. On live runs it transforms only the newly
sampled input that the priority-17 recorder will publish as next frame's player
snapshot; it preserves that already-transformed snapshot instead of applying
an involutive Mirror mode twice. Playback and blocking UI remain unmodified.
Dialogue keeps screen-relative directional mapping while suppressing Autoshot.
Relax reuses the same effective-input seam, holding Shoot and Focus only during
live, unblocked gameplay. It is isolated in its own modifier directory; the
TH08 adapter merely registers the generic input hook when `RX` is selected.

The version-1 modifier registry is now the single owner of built-in bits,
codes, ruleset versions, canonical manifest order, and conflict masks. Runtime
validation rejects unknown bits and conflicts with distinct result codes, and
tests pin the published `HD+FL+AT+MR+NF+DT+HR+EZ` order, append `RX+BS+NB`
without renumbering earlier modifiers, and cover both conflict pairs.
Effective-input composition is also fixed for version 1: geometric direction
transforms run before Relax or Autoshot assistance actions are injected.
No Bomb suppression runs last, so `RX+NB` deterministically leaves automatic
Shoot and Focus while removing Bomb. Modifier-specific options and behavior
remain in their vertical slices instead of moving into the registry.

Blind Spot is the second projectile-observation modifier. Its portable policy
owns the 48-to-128-pixel alpha curve, while its TH08 slice measures bullet
distance and distance to the nearest point on an active laser segment. The
shared TH08 projectile observer applies temporal Hidden first and spatial Blind
Spot second, scopes the composed alpha to one draw call, and restores authored
VM color state immediately afterward. Both modifiers remain simulation no-ops.

No Fail is another complete vertical slice. Its portable policy returns two
separate decisions for a committed miss: whether the run continues and whether
a life is consumed. Positive-life behavior remains vanilla; at zero lives an
active `NF@1` run continues without decrementing below zero. The TH08
translation asks that policy only at the original power-drop and retry/respawn
branches, so spell failure, death statistics, time-orb loss, rank loss, death
animation, and bomb reset remain authored game behavior. The mod-disabled VC7
lane still matches `Player::FUN_0044cbf0 @ 0x0044CBF0` exactly at 1,373 of
1,373 bytes with all relocations, and the normal 52-object VC7 link also
passes.

Hard Rock and Easy keep their fixed ratios in separate portable modifier
slices. The shared TH08 adapter only translates those ratios at game-specific
commit boundaries: final player movement, initialized hurtbox and graze sizes,
laser graze expansion, every active and fade-state bullet displacement, and
laser length growth. Applying projectile speed to final displacement means the
same hook covers random speeds, acceleration, turning, and bounce transforms
without rewriting ECL records or letting a later transform restore vanilla
motion. Easy also translates each authored SHT starting-bomb reset to `base+2`
(capped at TH08's eight-bomb limit) and scales the real enemy spell timeout at
spell start, so the countdown, bonus decay, and timeout transition share the
same `5/4` duration. Replay stage restores remain untouched. The
target-independent policy and adapter tests pass, and the complete 32-bit Linux
game links with these hooks enabled.

Double Time is implemented as a separate vertical slice under
`src/mod/modifiers/doubletime/`. Its portable `DT@1` policy fixes the rate at
3/2 and advances a bounded integer accumulator in the deterministic cadence
`1,2,1,2,...`. Configuration, run begin/end, and non-gameplay scene boundaries
reset the accumulator, so pause duration never creates catch-up work. The
overload contract is deliberately small: one display-paced presentation runs
at most two complete authored ticks, ticks are never skipped inside that
cadence, and a slow host slows wall-clock progression instead of injecting an
unbounded backlog.

The TH08 translation wraps `RunCalcChain` and `SoundPlayer::ProcessQueues` in
that cadence while leaving the draw chain and `Present` at one call per host
presentation. `g_GameManager.flags.unk2` limits acceleration to active
gameplay, keeping title, loading, retry, and pause paths at 1x. The shared
Linux/Web mixer multiplies each PCM source cursor step by the same rational
rate under the SDL audio-device lock and returns to 1/1 at the same scene
boundary. The mod-disabled VC7 lane still matches `GameWindow::Render @
0x00441E70` exactly at 482 of 482 bytes with all 38 relocations.

### Retail Linux gameplay smoke

On 2026-09-01, a locally supplied Japanese retail archive was extracted into a
temporary directory outside the repository. Its `th08.exe` SHA-256 was
`330fbdbf58a710829d65277b4f312cfbb38d5448b3df523e79350b879213d924`,
matching the canonical 1.00d target. The two runtime DAT files had the expected
sizes of 46,838,025 and 449,961,024 bytes. Each run used a fresh temporary data
directory containing only symlinks to those DAT files, so generated config and
score state never touched the source archive.

The 32-bit Linux build ran through the title, difficulty, team, and Stage 1
paths under Xvfb with the software OpenGL renderer. All five Mirror modes were
observed in gameplay. Horizontal, vertical, and 180° retained the complete
playfield; 90° and 270° showed the expected aspect-fit bars; and the HUD stayed
upright in every case. Physical screen-direction probes also moved in the same
visible direction: Right for horizontal, 90°, and 180°, Down for vertical, and
Left for 270°.

The first horizontal probe exposed a two-frame input feedback bug: a sampled
Right became Left correctly, but the following frame treated that stored Left
as raw input and transformed it back to Right. A debugger trace pinned the
native values to `0x80 -> 0x40 -> 0x80`. The adapter now models the player and
recorder timelines explicitly, and a deterministic two-frame regression plus
the five runtime direction probes confirm the correction. A combined
`HD+FL+AT+MR` run also reached active Stage 1; Autoshot, Flashlight, and Mirror
were visibly active together. Hidden's time-based fade still needs a controlled
paired capture rather than relying on that composition smoke.

A separate No Fail A/B run used natural Stage 1 collisions rather than a
patched control-flow result. With modifiers disabled, the third miss reached
zero lives, incremented the death counter to 3, set `showRetryMenu` to 1, and
disabled active gameplay while the retry menu was visible. With `NF@1`
selected, the same unattended run reached 8 deaths while lives remained
exactly 0, `showRetryMenu` remained 0, gameplay updates remained active, and
bombs reset to 3 after respawn. This verifies repeated zero-life respawning and
the no-underflow contract, not just modifier selection.

A Double Time A/B sampled `g_Supervisor.calcCount` during the same uninterrupted
eight-second Stage 1 interval. The `NF@1` control advanced 490 complete authored
ticks; `NF@1+DT@1` advanced 736, a measured ratio of 1.502x. Presentation and
the HUD counter remained at 60 FPS. While paused, the counter advanced 371
ticks in six seconds at 1x; after resume it advanced 554 ticks in six seconds
at approximately 1.5x. Debugger snapshots also observed the shared mixer rate
switch from 3/2 during gameplay to 1/1 in the pause menu.

The same `NF@1+DT@1` run was repeated in a current headless Chromium against
the locally served pthread Wasm build. Over eight seconds it recorded 481
browser callbacks and 721 authored calculation frames, a ratio of 1.49896x.
The paused sample recorded 301 callbacks and 301 calculations exactly. This
run also exposed and fixed a launcher boundary bug: the manifest is now copied
out of Wasm shared memory before `TextDecoder` receives it. The selected retail
files remained browser-local, and the generated screenshot and profile stayed
under `/tmp`.

Firefox 153 exercised its dedicated main-thread proxy presentation artifact
with the same files and manifest. A ten-second Stage 1 sample recorded 591
callbacks and 882 authored calculations, a ratio of 1.49239x. Its paused sample
recorded 354 callbacks and 350 calculations, or 0.98870x; the small difference
is expected because four browser callbacks did not cross the authored 60 Hz
timestamp gate. The proxy canvas rendered the pause scene without a page,
worker, or Wasm error.

After the No Fail seam was linked, target-independent core/C++98 tests, both
mod-enabled and mod-disabled fixed-address Linux builds, the Linux layout
verifier, both Emscripten presentation variants, Wasm manifest smoke tests, and
the Web retail-data provenance check all passed.

All retail-derived screenshots and generated state stayed under `/tmp` and
were not added to Git. An audible hardware audio duration/pitch comparison
remains pending; Node/Wasm smoke continues to cover both browser artifacts'
selection, ABI, manifest, and provenance behavior.

### Prototype debt gate

Before calling the slice production-ready, replace temporary configuration
plumbing, add explicit validation errors, isolate writable state, settle replay
manifest persistence, cover lasers and Stage 2 darkness composition, and run
the remaining audible hardware audio check.

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
- Final score multipliers before mechanics are stable and measured.
