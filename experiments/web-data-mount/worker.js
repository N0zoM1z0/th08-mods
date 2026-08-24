importScripts("probe.js");

self.onmessage = async (event) => {
  if (event.data?.type !== "inspect") return;

  try {
    const module = await createDataProbe({
      noInitialRun: true,
      print: (line) => self.postMessage({ type: "log", stream: "stdout", line }),
      printErr: (line) => self.postMessage({ type: "log", stream: "stderr", line }),
    });

    module.FS.mkdir("/game");
    module.FS.mount(module.WORKERFS, { files: event.data.files }, "/game");
    const status = module._inspect_data_files();
    self.postMessage({ type: "done", status });
  } catch (error) {
    self.postMessage({ type: "error", message: String(error?.stack || error) });
  }
};
