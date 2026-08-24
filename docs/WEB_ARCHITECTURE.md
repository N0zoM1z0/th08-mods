# Web port architecture and feasibility evidence

## Decision

Build the browser edition from the reconstructed C++ with Emscripten targeting
32-bit WebAssembly. JavaScript is a thin browser bootstrap only: local file
selection, worker/canvas startup, persistence synchronization, and browser
error reporting. Gameplay, archive parsing, simulation, and game state remain
the authored C++ implementation.

This is not a TypeScript reimplementation and it is not an x86 emulator. The
VC7 exact build remains the evidence product; the Web build is a separate
modern port from the same authored sources and makes no binary-exact claim.

The first playable milestone should use the smallest compatibility path:

```text
browser shell
  local File picker + canvas + user audio gesture
                    |
                    v
Web Worker / wasm32 authored game
  PBG and game logic unchanged
  Win32/D3D8/DirectSound/GDI-shaped platform interfaces
                    |
         +----------+-----------+
         |          |           |
     WORKERFS    WebGL 2    Web Audio/SDL
   retail, read  renderer      audio
         |
   user-owned th08.dat + thbgm.dat

IDBFS is a separate writable mount for cfg, score, replay, and backup files.
No retail data travels to the server or enters a deployment artifact.
```

## Provenance boundary

The repository and every deployable artifact contain only project source,
HTML/JavaScript glue, WebAssembly, and clearly licensed project-owned assets.
They must never contain `th08.dat`, `thbgm.dat`, the original executables, or
files extracted from the retail archives.

At runtime the user selects exactly `th08.dat` and `thbgm.dat` from a legally
obtained local installation. The browser passes those `File` objects to a Web
Worker. Emscripten WORKERFS mounts them read-only and services range reads
without copying the whole files into the Wasm heap. There is no upload API and
no server component.

Keep writes out of the retail mount. Mount IDBFS at `/save`, populate it before
startup, and make it the current working directory. Present the two read-only
archives there through virtual links to `/retail`, or resolve those two names
in the Web filesystem adapter if cross-mount links prove unreliable. Persist
configuration, score, replays, and backups using `FS.syncfs()` at explicit
checkpoints (or IDBFS `autoPersist`). Never persist either retail archive.

`scripts/check-web-provenance.py` rejects tracked DAT files, original
executables, and common retail archive containers. This is a minimum automatic
gate, not a substitute for source/asset review. A release job must stage a new
allowlisted directory rather than publish an arbitrary repository or build
directory.

## Feasibility probes

All probes use the digest-pinned `emscripten/emsdk:6.0.8` container. They are
small decision experiments, not claims that the game is playable in a browser.

### Authored-source compiler gate

`scripts/build-web-probe.sh` compiles all 44 shared production-authored game
and PBG translation units as wasm32 objects. On 2026-08-24 it completed 44/44.
Only a Web forced-include boundary and compiler compatibility flags were
needed; no gameplay source was replaced.

### Local retail-file gate

`scripts/build-web-data-probe.sh` builds a browser worker probe. With the two
local retail files selected, C++ `fopen`/`fseek` read both their first and last
bytes through WORKERFS. The observed sizes were 46,838,025 and 449,961,024
bytes while the Wasm heap remained 16,908,288 bytes. The local HTTP server saw
requests only for HTML, JavaScript, Worker, and Wasm files: neither DAT was
uploaded or fetched over HTTP.

This disproves the risky assumption that the roughly 450 MiB music archive
must be embedded in a download or copied wholesale into linear memory.

### Target-address arena gate

`scripts/build-web-layout-probe.sh` places ordinary WebAssembly data at or
above 32 MiB with `GLOBAL_BASE`, writes a representative target-owned slot at
`0x004c6c3c`, and verifies that the regions do not overlap. This proves that
raw 32-bit target-address accesses can remain valid in a reserved low-memory
arena.

It does **not** solve symbol identity by itself. The Linux port uses ELF linker
aliases from `th08-layout.ld`; wasm-ld has no equivalent arbitrary absolute
symbol layout. Before a full Web link, generate a Web binding layer that makes
every identity-sensitive global and raw-address view resolve to one object in
the low arena. Do not duplicate objects or hand-maintain a second address map.

### Renderer gate

`scripts/build-web-renderer-probe.sh` builds the existing D3D8 compatibility
backend with Emscripten's legacy fixed-function support and presents a 640x480
frame through WebGL 2. Browser verification reached
`renderer-probe=ok` and showed the expected frame.

This is the fastest title-screen bring-up route, but it is deliberate
prototype debt. WebGL has no desktop attribute stack, and Emscripten warns
that legacy GL emulation is limited. Keep the `IDirect3D8`/`IDirect3DDevice8`
surface used by authored code, then replace the backend internals with explicit
WebGL 2 / GLES2 shaders and state tracking. WebGPU adds a larger semantic and
tooling gap and is not justified for the first port.

Run the probes with:

```bash
scripts/build-web-probe.sh
scripts/build-web-layout-probe.sh
scripts/build-web-data-probe.sh
scripts/build-web-renderer-probe.sh
python3 scripts/check-web-provenance.py
```

Serve `build/web-data-mount` or `build/web-renderer-probe` from localhost for
manual browser checks. Generated files and local data remain under ignored
paths.

## MVP platform choices

### Threads and event loop

The current game has a blocking main loop plus Win32-style startup and BGM
threads. For the shortest route to a playable title, compile with pthreads,
move `main()` off the browser UI thread with `PROXY_TO_PTHREAD`, and enable the
OffscreenCanvas/proxied WebGL path. This also places synchronous WORKERFS
access in a worker, where it is supported.

That build requires `SharedArrayBuffer`, so the static host must send
cross-origin isolation headers (`Cross-Origin-Opener-Policy: same-origin` and
`Cross-Origin-Embedder-Policy: require-corp`) for every entry point and asset.
There is still no application backend and no retail-data upload. A development
server and deployment smoke test must fail clearly when `crossOriginIsolated`
is false.

After the full game works, evaluate whether broader hosting compatibility is
worth the larger refactor: split the blocking loop into one
`GameWindow::Tick()` scheduled by `emscripten_set_main_loop_arg`, and replace
startup/BGM threads with cooperative state machines or worker messages. Since
WORKERFS itself is worker-only, a threadless design still needs a dedicated
worker/OffscreenCanvas arrangement or a separate Blob-range streaming adapter.
Do not pay this complexity before the pthread MVP has exposed real browser
compatibility costs.

### Rendering

Use the current D3D8 compatibility interface as the seam. The order is:

1. legacy GL emulation for title-screen and gameplay bring-up;
2. inventory the actually used D3D8 render states, texture combiners, vertex
   formats, FBO copies, and dynamic text surfaces;
3. replace legacy calls with a small shader/state-cache WebGL 2 backend;
4. retain replay-visible viewport, blending, fog, and half-pixel behavior.

### Audio, input, and text

- Route keyboard/gamepad through SDL2 initially and preserve the authored
  DirectInput-shaped interface. Prevent browser scrolling/default shortcuts
  while the canvas owns focus.
- Reuse the Linux SDL audio boundary for sound effects and streamed BGM. Audio
  startup must follow a click/key gesture. The BGM thread and WORKERFS range
  behavior must be exercised before claiming the title milestone complete.
- Replace Fontconfig discovery with one explicitly licensed Japanese font or a
  browser text rasterizer behind the GDI-shaped interface. Never extract or
  redistribute a font or glyph asset from the retail game.

### Memory and addresses

Keep wasm32: reconstructed layouts and pointers are 32-bit. Reserve the low
target-address arena and place normal Wasm static data above it. Pthread builds
also need a fixed maximum shared memory size, selected from measurements of
title, stage, archive cache, audio buffers, and save data rather than from the
combined retail archive sizes. WORKERFS means the two DAT sizes do not dictate
the heap size.

## Reuse ledger

| Disposition | Components |
| --- | --- |
| Reuse unchanged | 44 authored game/PBG translation units, PBG formats and decompression, simulation and replay logic, 32-bit layouts, D3DX math compatibility |
| Adapt behind existing interfaces | Win32 runtime, target-global identity, D3D8 renderer, SDL input/audio, GDI text, paths and persistence |
| New thin browser code | local file picker, Worker/Canvas bootstrap, isolation checks, IDBFS synchronization, error/status UI |
| Explicitly excluded | TypeScript gameplay rewrite, x86 emulation, bundled/preloaded retail DATs, extracted retail assets, WebGPU-first renderer |

## Milestones and acceptance checks

1. **Compiler gate — complete:** all authored shared sources compile as wasm32.
2. **Data gate — complete:** both local DATs support zero-upload range reads
   without whole-file heap copies.
3. **Renderer gate — complete:** the current adapter presents a WebGL 2 frame.
4. **Full-link gate:** resolve platform symbols and global identity, initialize
   from selected data, and enter a browser-safe loop without aborts.
5. **Title gate:** title/menu render, input works, Japanese text is legible,
   SFX and streamed BGM play, and cfg/score survive reload.
6. **Gameplay gate:** deterministic stage/replay smoke plus stage transitions,
   pause, focus loss, resizing, save failure, and audio underrun tests.
7. **Release gate:** clean-browser test of the staged static artifact, required
   isolation headers, no network request containing retail bytes, provenance
   checker pass, and an explicit user-data/license notice.

Current prototype debt is intentionally visible: no full executable link,
generated target-global binding layer, browser main-loop integration, IDBFS
save overlay, production WebGL shader backend, Japanese font path, or full BGM
stream exists yet. The next coherent target is the full-link gate, not a
parallel rewrite of game systems.

## Primary references

- [Emscripten: Building Projects](https://emscripten.org/docs/compiling/Building-Projects.html)
- [Emscripten: File System API](https://emscripten.org/docs/api_reference/Filesystem-API.html)
- [Emscripten: Pthreads support](https://emscripten.org/docs/porting/pthreads.html)
- [Emscripten: OpenGL support](https://emscripten.org/docs/porting/multimedia_and_graphics/OpenGL-support.html)
- [Emscripten: compiler settings](https://emscripten.org/docs/tools_reference/settings_reference.html)
