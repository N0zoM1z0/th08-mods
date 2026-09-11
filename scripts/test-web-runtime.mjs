#!/usr/bin/env node

import fs from "node:fs";
import http from "node:http";
import os from "node:os";
import path from "node:path";
import process from "node:process";
import { chromium } from "playwright-core";

const REQUIRED_ARTIFACTS = [
  "th08-web.html",
  "th08-web.js",
  "th08-web.wasm",
  "th08-web-firefox.html",
  "th08-web-firefox.js",
  "th08-web-firefox.wasm",
  "th08-web-icon.png",
];

const MODIFIER_INPUTS = new Map([
  ["HD", "#mod-hidden"],
  ["FL", "#mod-flashlight"],
  ["AT", "#mod-autoshot"],
  ["MR", "#mod-mirror"],
  ["NF", "#mod-nofail"],
  ["DT", "#mod-doubletime"],
  ["HR", "#mod-hardrock"],
  ["EZ", "#mod-easy"],
  ["RX", "#mod-relax"],
  ["BS", "#mod-blindspot"],
  ["NB", "#mod-nobomb"],
]);

const MIRROR_MODES = new Map([
  ["horizontal", { value: "0", manifest: "horizontal" }],
  ["vertical", { value: "1", manifest: "vertical" }],
  ["90", { value: "2", manifest: "rotate-90" }],
  ["rotate-90", { value: "2", manifest: "rotate-90" }],
  ["180", { value: "3", manifest: "rotate-180" }],
  ["rotate-180", { value: "3", manifest: "rotate-180" }],
  ["270", { value: "4", manifest: "rotate-270" }],
  ["rotate-270", { value: "4", manifest: "rotate-270" }],
]);

function usage() {
  return `Usage:
  npm run test:web-runtime -- \\
    --artifact build/web-dist \\
    --game-data /path/to/th08.dat \\
    --bgm-data /path/to/thbgm.dat \\
    --replay /path/to/replay/th8_03.rpy \\
    --expected-stage 5 \\
    --mods HD,FL,MR,DT \\
    --mirror-mode 90

Options:
  --mode direct|proxy|both  Browser link(s) to test (default: both)
  --mods CODE,...           Modifier codes to enable (default: none)
  --mirror-mode MODE        horizontal, vertical, 90, 180, or 270
  --warmup-seconds N       Stage warm-up before measurement (default: 5)
  --duration-seconds N     Gameplay sampling time (default: 20)
  --minimum-fps N          Minimum presentation-equivalent rate (default: 50)
  --browser PATH           Chrome/Chromium executable (auto-detected by default)
  --output-dir PATH        Screenshots/report destination (temporary by default)
  --swiftshader            Force Chrome's software WebGL backend
  --no-sandbox             Pass --no-sandbox to Chrome when the host requires it
`;
}

function optionName(property) {
  return property.replace(/[A-Z]/g, (character) => `-${character.toLowerCase()}`);
}

function parseArguments(argv) {
  const options = {
    mode: "both",
    warmupSeconds: 5,
    durationSeconds: 20,
    minimumFps: 50,
    mods: "",
    mirrorMode: "horizontal",
    swiftshader: false,
    noSandbox: false,
  };
  const valueOptions = new Map([
    ["--artifact", "artifact"],
    ["--game-data", "gameData"],
    ["--bgm-data", "bgmData"],
    ["--replay", "replay"],
    ["--expected-stage", "expectedStage"],
    ["--mode", "mode"],
    ["--mods", "mods"],
    ["--mirror-mode", "mirrorMode"],
    ["--warmup-seconds", "warmupSeconds"],
    ["--duration-seconds", "durationSeconds"],
    ["--minimum-fps", "minimumFps"],
    ["--browser", "browser"],
    ["--output-dir", "outputDir"],
  ]);

  for (let index = 0; index < argv.length; ++index) {
    const argument = argv[index];
    if (argument === "--help" || argument === "-h") {
      console.log(usage());
      process.exit(0);
    }
    if (argument === "--swiftshader") {
      options.swiftshader = true;
      continue;
    }
    if (argument === "--no-sandbox") {
      options.noSandbox = true;
      continue;
    }
    const property = valueOptions.get(argument);
    if (!property || index + 1 >= argv.length) {
      throw new Error(`Unknown or incomplete argument: ${argument}\n\n${usage()}`);
    }
    options[property] = argv[++index];
  }

  for (const property of ["artifact", "gameData", "bgmData", "replay"]) {
    if (!options[property]) throw new Error(`Missing --${optionName(property)}`);
    options[property] = path.resolve(options[property]);
  }
  if (!["direct", "proxy", "both"].includes(options.mode)) {
    throw new Error("--mode must be direct, proxy, or both");
  }
  const requestedModifiers = new Set(options.mods
    .split(",")
    .map((code) => code.trim().toUpperCase())
    .filter(Boolean));
  for (const code of requestedModifiers) {
    if (!MODIFIER_INPUTS.has(code)) throw new Error(`Unknown modifier code: ${code}`);
  }
  if (requestedModifiers.has("HR") && requestedModifiers.has("EZ")) {
    throw new Error("HR and EZ cannot be enabled together");
  }
  if (requestedModifiers.has("AT") && requestedModifiers.has("RX")) {
    throw new Error("AT and RX cannot be enabled together");
  }
  options.mods = [...MODIFIER_INPUTS.keys()].filter((code) => requestedModifiers.has(code));
  const mirrorMode = MIRROR_MODES.get(options.mirrorMode.toLowerCase());
  if (!mirrorMode) throw new Error(`Unknown mirror mode: ${options.mirrorMode}`);
  if (!requestedModifiers.has("MR") && mirrorMode.manifest !== "horizontal") {
    throw new Error("--mirror-mode requires MR in --mods");
  }
  options.mirrorMode = mirrorMode;
  options.expectedCalculationMultiplier = requestedModifiers.has("DT") ? 1.5 : 1.0;
  for (const property of ["durationSeconds", "minimumFps"]) {
    options[property] = Number(options[property]);
    if (!Number.isFinite(options[property]) || options[property] <= 0) {
      throw new Error(`--${optionName(property)} must be positive`);
    }
  }
  options.warmupSeconds = Number(options.warmupSeconds);
  if (!Number.isFinite(options.warmupSeconds) || options.warmupSeconds < 0) {
    throw new Error("--warmup-seconds must be zero or positive");
  }
  if (options.expectedStage !== undefined) {
    options.expectedStage = Number(options.expectedStage);
    if (!Number.isInteger(options.expectedStage) || options.expectedStage < 0 || options.expectedStage > 8) {
      throw new Error("--expected-stage must be a TH08 stage enum from 0 through 8");
    }
  }
  options.outputDir = options.outputDir
    ? path.resolve(options.outputDir)
    : fs.mkdtempSync(path.join(os.tmpdir(), "th08-web-runtime-"));
  return options;
}

function validateInputs(options) {
  for (const artifact of REQUIRED_ARTIFACTS) {
    const candidate = path.join(options.artifact, artifact);
    if (!fs.statSync(candidate).isFile()) throw new Error(`Missing Web artifact: ${candidate}`);
  }
  const requiredNames = new Map([
    [options.gameData, "th08.dat"],
    [options.bgmData, "thbgm.dat"],
  ]);
  for (const [candidate, expectedName] of requiredNames) {
    if (!fs.statSync(candidate).isFile() || path.basename(candidate).toLowerCase() !== expectedName) {
      throw new Error(`Expected a file named ${expectedName}: ${candidate}`);
    }
  }
  if (!fs.statSync(options.replay).isFile() ||
      !/^th8_(?:\d{2}|ud....)\.rpy$/i.test(path.basename(options.replay))) {
    throw new Error("Replay must use the retail th8_NN.rpy or th8_udNNNN.rpy naming scheme");
  }
  fs.mkdirSync(options.outputDir, { recursive: true });
}

function detectBrowser(requested) {
  if (requested) return path.resolve(requested);
  const candidates = [
    "/Applications/Google Chrome.app/Contents/MacOS/Google Chrome",
    "/usr/bin/google-chrome-stable",
    "/usr/bin/google-chrome",
    "/usr/bin/chromium",
    "/usr/bin/chromium-browser",
  ];
  const browser = candidates.find((candidate) => fs.existsSync(candidate));
  if (!browser) throw new Error("Chrome/Chromium was not found; pass --browser PATH");
  return browser;
}

function mimeType(fileName) {
  if (fileName.endsWith(".html")) return "text/html; charset=utf-8";
  if (fileName.endsWith(".js")) return "text/javascript; charset=utf-8";
  if (fileName.endsWith(".wasm")) return "application/wasm";
  if (fileName.endsWith(".png")) return "image/png";
  return "application/octet-stream";
}

async function startServer(artifactDirectory) {
  const artifactRoot = fs.realpathSync(artifactDirectory);
  const server = http.createServer((request, response) => {
    const pathname = new URL(request.url, "http://127.0.0.1").pathname;
    response.setHeader("Cross-Origin-Opener-Policy", "same-origin");
    response.setHeader("Cross-Origin-Embedder-Policy", "require-corp");
    response.setHeader("Cross-Origin-Resource-Policy", "same-origin");
    response.setHeader("Cache-Control", "no-store");
    if (pathname === "/") {
      response.writeHead(302, { Location: "/th08-web.html" });
      response.end();
      return;
    }
    const fileName = path.basename(decodeURIComponent(pathname));
    const filePath = path.join(artifactRoot, fileName);
    if (!REQUIRED_ARTIFACTS.includes(fileName) || !fs.existsSync(filePath)) {
      response.writeHead(404);
      response.end("Not found\n");
      return;
    }
    response.writeHead(200, { "Content-Type": mimeType(fileName) });
    fs.createReadStream(filePath).pipe(response);
  });
  await new Promise((resolve, reject) => {
    server.once("error", reject);
    server.listen(0, "127.0.0.1", resolve);
  });
  return server;
}

async function pressAndSettle(page, key, milliseconds = 250) {
  await page.keyboard.press(key);
  await page.waitForTimeout(milliseconds);
}

function modifierCodesFromManifest(manifest) {
  const match = /(?:^|;)mods=([^;]*)/.exec(manifest);
  if (!match || match[1] === "none") return [];
  return match[1].split("+").map((entry) => entry.split("@", 1)[0]);
}

async function testMode(browser, baseUrl, options, mode) {
  const context = await browser.newContext({ viewport: { width: 1180, height: 820 } });
  const page = await context.newPage();
  const failures = [];
  const consoleErrors = [];
  page.on("pageerror", (error) => failures.push(`page error: ${error.message}`));
  page.on("crash", () => failures.push("page crashed"));
  page.on("console", (message) => {
    if (message.type() === "error") consoleErrors.push(message.text());
  });

  try {
    const launcher = mode === "proxy" ? "th08-web-firefox.html" : "th08-web.html";
    await page.goto(`${baseUrl}/${launcher}?perf=1&presentation=${mode}`, {
      waitUntil: "networkidle",
    });
    const isolated = await page.evaluate(() => crossOriginIsolated);
    if (!isolated) throw new Error(`${mode}: page is not cross-origin isolated`);
    await page.waitForFunction(
      () => typeof window.th08GetModifierManifest?.() === "string",
      null,
      { timeout: 30000 },
    );

    for (const code of options.mods) {
      await page.locator(MODIFIER_INPUTS.get(code)).check();
    }
    if (options.mods.includes("MR")) {
      await page.locator("#mod-mirror-mode").selectOption(options.mirrorMode.value);
    }
    const selectedManifest = await page.evaluate(() => window.th08GetModifierManifest());
    if (typeof selectedManifest !== "string") {
      throw new Error(`${mode}: modifier manifest is unavailable`);
    }
    const selectedCodes = modifierCodesFromManifest(selectedManifest);
    if (selectedCodes.join(",") !== options.mods.join(",")) {
      throw new Error(`${mode}: expected modifiers ${options.mods.join(",") || "none"}, ` +
        `observed ${selectedCodes.join(",") || "none"}`);
    }
    if (options.mods.includes("MR") &&
        !selectedManifest.includes(`MR@1(${options.mirrorMode.manifest})`)) {
      throw new Error(`${mode}: manifest does not contain Mirror mode ${options.mirrorMode.manifest}`);
    }

    await page.locator("#game-data").setInputFiles(options.gameData);
    await page.locator("#bgm-data").setInputFiles(options.bgmData);
    await page.locator("#start").click();
    await page.waitForFunction(() => document.querySelector("#log").textContent.includes(
      "th08-web: startup: main archive and initial chain processed"), null, { timeout: 90000 });
    await page.waitForTimeout(2000);

    const replayBytes = Array.from(fs.readFileSync(options.replay));
    const replayName = path.basename(options.replay);
    await page.evaluate(({ bytes, name }) => {
      Module.FS.writeFile(`/game/replay/${name}`, new Uint8Array(bytes));
    }, { bytes: replayBytes, name: replayName });

    // A clean browser profile has Extra and Spell Practice locked, so the
    // authored menu skips both: two Down presses select Replay.
    await pressAndSettle(page, "ArrowDown");
    await pressAndSettle(page, "ArrowDown");
    await pressAndSettle(page, "KeyZ");
    await page.waitForFunction(() => document.querySelector("#log").textContent.includes(
      "th08-web: replay menu: 1 valid replay entries"), null, { timeout: 15000 });
    await page.waitForTimeout(1000);
    await pressAndSettle(page, "KeyZ", 1000); // replay
    await pressAndSettle(page, "KeyZ", 1000); // first recorded stage
    await pressAndSettle(page, "KeyZ", 1000); // normal playback
    await page.waitForFunction(() => document.querySelector("#log").textContent.includes(
      "th08-web: gameplay: stage setup ready"), null, { timeout: 60000 });
    if (options.expectedStage !== undefined) {
      await page.waitForFunction((stage) =>
        (Module._th08_web_get_route_snapshot() & 0xf) === stage,
      options.expectedStage, { timeout: 15000 });
    }
    await page.waitForTimeout(options.warmupSeconds * 1000);

    const sampleBefore = await page.evaluate(() => ({
      browserTime: performance.now(),
      frame: Module._th08_web_get_frame_snapshot() >>> 0,
    }));
    await page.waitForTimeout(options.durationSeconds * 1000);
    const snapshot = await page.evaluate((before) => {
      const after = Module._th08_web_get_frame_snapshot() >>> 0;
      return {
        browserTime: performance.now(),
        status: document.querySelector("#status").textContent,
        performanceSummary: document.querySelector("#performance").textContent,
        runtimeLog: document.querySelector("#log").textContent,
        callbackDelta: ((after & 0xffff) - (before.frame & 0xffff)) & 0xffff,
        calculationDelta: ((after >>> 16) - (before.frame >>> 16)) & 0xffff,
        route: Module._th08_web_get_route_snapshot() >>> 0,
        playerShots: Module._th08_web_get_player_shot_snapshot() >>> 0,
        modifierManifest: window.th08GetModifierManifest(),
      };
    }, sampleBefore);
    const elapsedSeconds = (snapshot.browserTime - sampleBefore.browserTime) / 1000;
    snapshot.callbackRate = snapshot.callbackDelta / elapsedSeconds;
    snapshot.calculationRate = snapshot.calculationDelta / elapsedSeconds;
    snapshot.stage = snapshot.route & 0xf;
    snapshot.mode = mode;
    snapshot.consoleErrors = consoleErrors;
    snapshot.failures = failures;

    const modifierSuffix = options.mods.length ? options.mods.join("-").toLowerCase() : "none";
    const screenshot = path.join(
      options.outputDir, `stage-replay-${modifierSuffix}-${mode}.png`);
    await page.locator("#canvas").screenshot({ path: screenshot });
    snapshot.screenshot = screenshot;

    if (snapshot.status !== "TH08 gameplay is running from local retail data.") {
      failures.push(`unexpected status: ${snapshot.status}`);
    }
    if (options.expectedStage !== undefined && snapshot.stage !== options.expectedStage) {
      failures.push(`expected stage ${options.expectedStage}, observed ${snapshot.stage}`);
    }
    if (snapshot.callbackRate < options.minimumFps) {
      failures.push(`worker callback rate ${snapshot.callbackRate.toFixed(1)} is below ${options.minimumFps}`);
    }
    const minimumCalculationRate =
      options.minimumFps * options.expectedCalculationMultiplier;
    if (snapshot.calculationRate < minimumCalculationRate) {
      failures.push(`game rate ${snapshot.calculationRate.toFixed(1)} is below ` +
        `${minimumCalculationRate.toFixed(1)} for the selected modifiers`);
    }
    const calculationMultiplier = snapshot.calculationRate / snapshot.callbackRate;
    if (Math.abs(calculationMultiplier - options.expectedCalculationMultiplier) > 0.15) {
      failures.push(`game/worker multiplier ${calculationMultiplier.toFixed(2)} does not match ` +
        `${options.expectedCalculationMultiplier.toFixed(2)}`);
    }
    if (/abort|out of bounds|wasm trap/i.test(snapshot.runtimeLog)) {
      failures.push("runtime log contains an abort, out-of-bounds error, or Wasm trap");
    }
    if (consoleErrors.length) failures.push(`${consoleErrors.length} browser console error(s)`);
    return snapshot;
  } finally {
    await context.close();
  }
}

async function main() {
  const options = parseArguments(process.argv.slice(2));
  validateInputs(options);
  const browserPath = detectBrowser(options.browser);
  const server = await startServer(options.artifact);
  const address = server.address();
  const baseUrl = `http://127.0.0.1:${address.port}`;
  const browserArguments = [];
  if (options.noSandbox) browserArguments.push("--no-sandbox");
  if (options.swiftshader) {
    browserArguments.push(
      "--use-gl=angle",
      "--use-angle=swiftshader",
      "--enable-unsafe-swiftshader",
    );
  }

  let browser;
  try {
    browser = await chromium.launch({
      executablePath: browserPath,
      headless: true,
      args: browserArguments,
    });
    const modes = options.mode === "both" ? ["direct", "proxy"] : [options.mode];
    const results = [];
    for (const mode of modes) {
      console.log(`Testing ${mode} presentation at ${baseUrl} ...`);
      const result = await testMode(browser, baseUrl, options, mode);
      results.push(result);
      console.log(
        `${mode}: worker ${result.callbackRate.toFixed(1)} Hz, ` +
        `game ${result.calculationRate.toFixed(1)} FPS, stage ${result.stage}`,
      );
    }
    const report = {
      artifact: options.artifact,
      browser: browserPath,
      warmupSeconds: options.warmupSeconds,
      durationSeconds: options.durationSeconds,
      minimumFps: options.minimumFps,
      modifiers: options.mods,
      mirrorMode: options.mods.includes("MR") ? options.mirrorMode.manifest : null,
      expectedCalculationMultiplier: options.expectedCalculationMultiplier,
      results,
    };
    const reportPath = path.join(options.outputDir, "runtime-report.json");
    fs.writeFileSync(reportPath, `${JSON.stringify(report, null, 2)}\n`);
    console.log(`Runtime report and screenshots: ${options.outputDir}`);
    if (results.some((result) => result.failures.length)) {
      for (const result of results) {
        for (const failure of result.failures) console.error(`${result.mode}: ${failure}`);
      }
      process.exitCode = 1;
    }
  } finally {
    if (browser) await browser.close();
    await new Promise((resolve) => server.close(resolve));
  }
}

main().catch((error) => {
  console.error(error.stack || error.message || error);
  process.exitCode = 1;
});
