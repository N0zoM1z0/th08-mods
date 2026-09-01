"use strict";

const path = require("node:path");

if (process.argv.length !== 3) {
  console.error("usage: node ManifestSmoke.cjs <emscripten-runtime.js>");
  process.exit(2);
}

const runtimePath = path.resolve(process.argv[2]);
let runtime;
try {
  runtime = require(runtimePath);
} catch (error) {
  console.error(`Unable to load ${runtimePath}:`, error);
  process.exit(1);
}

let finished = false;
const timeout = setTimeout(() => {
  if (!finished) {
    console.error(`Timed out while initializing ${runtimePath}.`);
    process.exit(1);
  }
}, 30000);

function finish(exitCode, message) {
  if (finished) return;
  finished = true;
  clearTimeout(timeout);
  const stream = exitCode === 0 ? process.stdout : process.stderr;
  stream.write(`${message}\n`, () => process.exit(exitCode));
}

function configure(mask) {
  const size = runtime._th_mod_config_v1_size();
  if (size < 28 || size > 4096 || size % 4 !== 0) {
    throw new Error(`Invalid configuration size ${size}.`);
  }

  const pointer = runtime._malloc(size);
  if (!pointer) throw new Error("Unable to allocate configuration memory.");
  try {
    if (runtime._th_mod_get_default_config_v1(pointer) !== 0) {
      throw new Error("Unable to read configuration defaults.");
    }
    new DataView(runtime.HEAPU8.buffer, pointer, size).setUint32(8, mask, true);
    if (runtime._th_mod_configure_v1(pointer) !== 0) {
      throw new Error(`Configuration mask ${mask} was rejected.`);
    }
  } finally {
    runtime._free(pointer);
  }
}

function readManifest() {
  const size = runtime._th_mod_manifest_v1_size();
  if (size < 2 || size > 4096) {
    throw new Error(`Invalid manifest size ${size}.`);
  }

  const pointer = runtime._malloc(size);
  if (!pointer) throw new Error("Unable to allocate manifest memory.");
  try {
    if (runtime._th_mod_write_manifest_v1(pointer, size) !== 0) {
      throw new Error("Unable to write the manifest.");
    }
    return new TextDecoder().decode(
      runtime.HEAPU8.subarray(pointer, pointer + size - 1),
    );
  } finally {
    runtime._free(pointer);
  }
}

runtime.onAbort = (reason) => {
  finish(1, `Wasm runtime aborted: ${reason}`);
};

runtime.onRuntimeInitialized = () => {
  try {
    const prefix =
      "game=th08@1.00d;engine=th08-mods@1;base=th08-web@3f926db;api=1;mods=";
    const cases = [
      [0, "none"],
      [1, "HD@1(45,45)"],
      [2, "FL@1(96,224)"],
      [4, "AT@1"],
      [7, "HD@1(45,45)+FL@1(96,224)+AT@1"],
    ];

    for (const [mask, suffix] of cases) {
      configure(mask);
      const actual = readManifest();
      const expected = prefix + suffix;
      if (actual !== expected) {
        throw new Error(`Mask ${mask}: expected ${expected}, received ${actual}.`);
      }
    }
    finish(0, `Wasm modifier manifest smoke passed: ${path.basename(runtimePath)}`);
  } catch (error) {
    finish(1, `Wasm modifier manifest smoke failed: ${error.stack || error}`);
  }
};
