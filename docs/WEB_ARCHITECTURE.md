# Web port architecture and status

## Outcome

TH08 Web is built from the reconstructed C++ game sources with Emscripten and
runs in a desktop browser. JavaScript is limited to the browser boundary: the
launcher, local file selection, keyboard events, Blob range reads, canvas
startup, and status reporting. Gameplay, archive parsing, simulation,
rendering calls, and game state remain in the authored C++ implementation.

This is neither a TypeScript reimplementation nor an x86 emulator. The exact
VC7 build remains the reconstruction evidence product. The Web build is a
separate modern port from the same sources and makes no binary-exact claim.

The current flow is:

```text
browser page (main thread)
  file picker + keyboard + Web Audio
           |               ^
           | local File    | small synchronous proxies
           v               |
pthread worker running wasm32 authored game
  PBG/archive logic + simulation + D3D8-shaped renderer
           |                  |
           |                  v
           |            OffscreenCanvas / WebGL 2
           |
           +-- th08.dat: one volatile 46.8 MB session-memory copy
           +-- thbgm.dat: byte-range reads from the browser Blob
```

## Provenance boundary

The repository and deployable Web artifact contain only project source,
HTML/JavaScript glue, WebAssembly, and clearly licensed project-owned assets.
They must never contain `th08.dat`, `thbgm.dat`, an original executable, or
files extracted from the retail archives.

At runtime, the user selects `th08.dat` and `thbgm.dat` from a legally obtained
local installation. The launcher does not contain an upload API. It keeps the
two browser `File` objects in the page, copies `th08.dat` into volatile Wasm
memory, and services `thbgm.dat` reads from Blob slices. Empty MEMFS directory
entries provide only the filenames expected by original path and `stat` code;
they contain no retail bytes. Reloading or closing the page discards the
session.

The different strategies are intentional. Game archive reads are synchronous,
frequent, and small enough for a one-time memory copy. The roughly 450 MB music
archive must not consume the fixed Wasm heap, so the compatibility layer sends
range requests to the browser main thread and atomically waits in the game
worker. Neither archive is fetched over HTTP, uploaded, bundled, or written to
browser persistence.

`scripts/check-web-provenance.py` rejects forbidden payloads from both tracked
source and a staged artifact directory. It is a minimum automated gate, not a
license detector. Release automation must publish an allowlisted build
directory, never the repository or a data directory.

## Runtime design

### Threads and browser isolation

The port preserves the authored frame body and Win32-shaped startup/BGM
threads. Emscripten `PROXY_TO_PTHREAD` moves `main()` off the browser UI thread,
and a three-thread pool supports the existing startup jobs. The outer blocking
loop is Web-only adapted into one authored frame per `emscripten_set_main_loop`
callback so the worker yields to the browser after every frame. This requires
`SharedArrayBuffer` and therefore a secure, cross-origin-isolated page.

`scripts/serve-web.py` supplies the required COOP and COEP headers. Localhost is
a valid development secure context. A remote machine should use HTTPS; the raw
LAN or Tailscale HTTP address can display the launcher but cannot start this
pthread build in a conforming browser.

### Data bridge

The C++ Win32 file compatibility layer recognizes only the two retail archive
basenames as browser-owned files. `CreateFile`, `ReadFile`, seeking, size, and
close operations retain their synchronous authored-facing behavior.

- `th08.dat` is allocated with `malloc`, filled through the exported Wasm heap,
  and read with ordinary memory copies.
- `thbgm.dat` stays a browser `File`. Reads use `File.slice().arrayBuffer()` on
  the main runtime thread, copy only the requested range into Wasm memory, and
  wake the blocked worker through an atomic word.
- All configuration, score, replay, and backup writes currently use volatile
  MEMFS. Persistence is a separate milestone and must never include retail
  data.

### Rendering and frame pacing

The existing `IDirect3D8`/`IDirect3DDevice8` compatibility surface remains the
portability seam. The Web backend creates a WebGL 2 context directly on the
transferred `OffscreenCanvas` and translates the reconstructed D3D8 call stream
into an explicit GLSL ES 3.0 shader, vertex array, and rotating vertex buffers.
The CPU converts the small D3D flexible-vertex-format family used by TH08,
queues compatible draw commands, uploads the frame's vertices once, and applies
cached blend, depth, sampler, scissor, alpha-test, fog, and texture-stage state.
Backbuffer and dialogue-snapshot copies use a separate shader blit. No legacy
immediate-mode or fixed-function GL emulation remains on the Web hot path.

The context uses implicit swap control: returning from each Emscripten
main-loop callback lets the browser compositor present the completed frame and
provides browser-side pacing. A short active-gameplay sample recorded 297
browser callbacks and 297 authored calculation frames in five seconds. The
renderer separately measured approximately 0.08--0.15 ms of CPU game
submission and 0.01--0.03 ms of blit work per frame in representative scenes.
Those bounded observations demonstrate that the previous renderer bottleneck
is gone; they are not a substitute for full-route browser endurance testing.

An earlier blocking-loop experiment rendered correctly into the WebGL default
framebuffer but remained black on screen. In Emscripten 6, the native
OffscreenCanvas `emscripten_webgl_commit_frame()` path is a no-op because modern
browsers present implicitly. Moving the outer loop to yielding callbacks fixed
the actual display boundary without changing authored frame behavior.

Instrumentation continues to separate browser main-loop callbacks from
authored calculation frames. This distinction caught the earlier legacy
renderer regression, where callbacks held at 60 Hz while calculation fell to
about 40 FPS, and remains more reliable than the displayed in-game counter
alone.

### Input and audio

Browser keyboard events are translated to the Win32 virtual-key set used by
the compatibility layer and stored in shared atomic state. Each authored input
poll merges that state with SDL's keyboard state. A key-down edge remains
latched until one authored poll consumes it, preventing short Z/X taps from
falling entirely between 60 Hz polls. Blur clears held and pending keys to avoid
stuck movement or firing.

SDL's Web Audio device must be opened, paused, and closed on the main browser
runtime thread. Small synchronous proxies preserve the DirectSound-shaped C++
interface, while mixing remains in the shared Linux audio implementation. BGM
streaming reads `thbgm.dat` through the Blob range bridge.

### Wasm ABI adaptations

WebAssembly validates indirect call signatures more strictly than native x86.
The port uses Web-only typed adapters for Win32 thread entry points and callback
tables whose reconstructed x86 calls intentionally ignore a non-void return.
The VC7/native branches remain unchanged. These are ABI boundary adaptations,
not gameplay replacements.

Normal Wasm globals start at 32 MiB so existing low raw-address views remain
available. The pthread build currently uses a fixed 256 MiB initial shared
memory and a 4 MiB stack. The music archive is not part of that memory budget.

The original PE also gives several named globals overlapping identities inside
larger manager objects. A native Linux linker script can preserve those
addresses, but Wasm globals are relocatable. Web-only references therefore bind
ECL state, player/gauge fields, effect and GUI tables, and callback lifetimes to
their real aggregate owners. This is correctness-critical: split callback
storage previously left old spell jobs alive across stage reloads, producing
missing effects, unstable scores, and an eventual out-of-bounds trap during a
result transition. Runtime diagnostics verify the most failure-prone aliases
before endurance tests.

## Build and run

Docker is the only Emscripten prerequisite. The build image is pinned by tag
and digest:

```bash
scripts/build-web-game.sh
python3 scripts/check-web-provenance.py --artifact build/web-dist
scripts/serve-web.py --bind 127.0.0.1 --port 8000
```

Open `http://127.0.0.1:8000/`, select local files named exactly `th08.dat` and
`thbgm.dat`, and choose **Start TH08**. Keyboard controls are listed in the
launcher. The generated static artifact consists of `th08-web.html`,
`th08-web.js`, `th08-web.wasm`, and the project-owned `th08-web-icon.png` copied
from the Linux port. CMake metadata stays in `build/web-game`; only the
allowlisted files are staged in `build/web-dist`.

The normal script builds `Release`; the staged JavaScript and Wasm are
approximately 232 KiB and 1.4 MiB respectively. The old Debug Wasm was about
21 MiB and is not the public build path.

For another device, place the same server behind HTTPS and preserve the COOP,
COEP, CORP, and no-store headers. Static hosts that cannot provide
cross-origin isolation cannot run this pthread build.

## Reproducible evidence

All Web builds use
`emscripten/emsdk:6.0.8@sha256:f174124ff798a3ead1abef247d9a849c270b642d552fea500a42565ff210f765`.

- The authored compiler gate builds all 44 shared game/PBG translation units
  as wasm32 objects.
- The full target links the authored objects with the existing Linux platform
  adapters and the Web-specific browser boundaries.
- With user-selected retail archives of 46,838,025 and 449,961,024 bytes, the
  title loaded, rendered, and accepted keyboard input through difficulty and
  character/team selection. Stage setup then completed without a Wasm trap.
- A separate data probe read the first and last bytes of both browser-local
  files. Browser resource inspection showed only HTML, JavaScript, Worker, and
  Wasm requests; neither DAT appeared as a network resource.
- Browser screenshots show the full title, menus, Japanese dialogue, the Music
  Room, and gameplay on the worker-owned OffscreenCanvas. After the direct
  WebGL 2 renderer landed, bounded samples held authored calculation frames in
  lockstep with browser callbacks near 60 Hz. Stage 2 Normal registered
  spell-card numbers 14, 18, 22, 26, and 29 when its documented Last Spell
  time-orb requirement was met, and returned to the title without the former
  Wasm out-of-bounds trap.
- A Chromium Lunatic Border Team Final-B endurance run crossed all six route
  stages, credits, result/score writing, title reconstruction, and a second
  start without a runtime trap. Per-stage spell observations were 1, 5, 9,
  12; 16, 20, 24, 28, 31; 35, 38, 42, 46, 50, 53; 80, 84, 88, 92, 96, 99;
  103, 107, 111, 115, 118; and 150, 154, 158, 162, 166, 170, 174, 178, 182,
  186. Failing Kaguya's fourth Last Spell correctly reached the original 5:00
  cutoff and skipped the remaining request. A separate collision-free Stage
  6B practice run followed the real ECL flow through spell 190 and its All
  Clear transition, completing conditional coverage of all 37 expected route
  spells. The result path wrote a 17,074-byte `score.dat` into volatile
  `/game`; persistence remains deliberately disabled.

Run the bounded probes with:

```bash
scripts/build-web-probe.sh
scripts/build-web-layout-probe.sh
scripts/build-web-data-probe.sh
scripts/build-web-renderer-probe.sh
python3 scripts/check-web-provenance.py
```

## Remaining work

The port has crossed the full-link, title/menu, input, audio-device, BGM-range,
direct-renderer, full-route, conditional-spell, and volatile result/save gates.
It is an engineering preview, not a release. The next work is:

1. add an IDBFS save overlay for cfg, score, replay, and backup files, with an
   explicit guarantee that the retail archives cannot enter it;
2. run Firefox full-route plus Chromium/Firefox replay, pause/focus,
   audio-underrun, and repeated stage-reload regressions;
3. measure and tune the fixed shared-memory ceiling, then produce an
   allowlisted static release artifact and clean-profile deployment test;
4. add gamepad mapping and user-facing diagnostics for unsupported browsers.

## Primary references

- [Emscripten pthreads support](https://emscripten.org/docs/porting/pthreads.html)
- [Emscripten file system API](https://emscripten.org/docs/api_reference/Filesystem-API.html)
- [Emscripten OpenGL support](https://emscripten.org/docs/porting/multimedia_and_graphics/OpenGL-support.html)
- [Emscripten compiler settings](https://emscripten.org/docs/tools_reference/settings_reference.html)
