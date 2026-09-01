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

function configure(mask, mirrorMode = 0) {
  const size = runtime._th_mod_config_v1_size();
  if (size < 32 || size > 4096 || size % 4 !== 0) {
    throw new Error(`Invalid configuration size ${size}.`);
  }

  const pointer = runtime._malloc(size);
  if (!pointer) throw new Error("Unable to allocate configuration memory.");
  try {
    if (runtime._th_mod_get_default_config_v1(pointer) !== 0) {
      throw new Error("Unable to read configuration defaults.");
    }
    const config = new DataView(runtime.HEAPU8.buffer, pointer, size);
    config.setUint32(8, mask, true);
    config.setUint32(28, mirrorMode, true);
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
      [0, 0, "none"],
      [1, 0, "HD@1(45,45)"],
      [2, 0, "FL@1(96,224)"],
      [4, 0, "AT@1"],
      [8, 0, "MR@1(horizontal)"],
      [8, 1, "MR@1(vertical)"],
      [8, 2, "MR@1(rotate-90)"],
      [8, 3, "MR@1(rotate-180)"],
      [8, 4, "MR@1(rotate-270)"],
      [16, 0, "NF@1"],
      [32, 0, "DT@1"],
      [64, 0, "HR@1"],
      [128, 0, "EZ@1"],
      [256, 0, "RX@1"],
      [512, 0, "BS@1"],
      [639, 4, "HD@1(45,45)+FL@1(96,224)+AT@1+MR@1(rotate-270)+NF@1+DT@1+HR@1+BS@1"],
      [703, 4, "HD@1(45,45)+FL@1(96,224)+AT@1+MR@1(rotate-270)+NF@1+DT@1+EZ@1+BS@1"],
      [891, 4, "HD@1(45,45)+FL@1(96,224)+MR@1(rotate-270)+NF@1+DT@1+HR@1+RX@1+BS@1"],
      [955, 4, "HD@1(45,45)+FL@1(96,224)+MR@1(rotate-270)+NF@1+DT@1+EZ@1+RX@1+BS@1"],
    ];

    for (const [mask, mirrorMode, suffix] of cases) {
      configure(mask, mirrorMode);
      const actual = readManifest();
      const expected = prefix + suffix;
      if (actual !== expected) {
        throw new Error(
          `Mask ${mask}, Mirror ${mirrorMode}: expected ${expected}, received ${actual}.`,
        );
      }
    }
    finish(0, `Wasm modifier manifest smoke passed: ${path.basename(runtimePath)}`);
  } catch (error) {
    finish(1, `Wasm modifier manifest smoke failed: ${error.stack || error}`);
  }
};
