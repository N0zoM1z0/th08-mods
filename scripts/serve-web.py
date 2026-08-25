#!/usr/bin/env python3
"""Serve a TH08 Web build with the headers required by SharedArrayBuffer."""

from __future__ import annotations

import argparse
from functools import partial
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
from http import HTTPStatus
from pathlib import Path
from urllib.parse import urlsplit


ROOT = Path(__file__).resolve().parents[1]


class IsolatedRequestHandler(SimpleHTTPRequestHandler):
    def redirect_entrypoint(self) -> bool:
        if urlsplit(self.path).path != "/":
            return False
        self.send_response(HTTPStatus.FOUND)
        self.send_header("Location", "/th08-web.html")
        self.end_headers()
        return True

    def do_GET(self) -> None:
        if not self.redirect_entrypoint():
            super().do_GET()

    def do_HEAD(self) -> None:
        if not self.redirect_entrypoint():
            super().do_HEAD()

    def end_headers(self) -> None:
        self.send_header("Cross-Origin-Opener-Policy", "same-origin")
        self.send_header("Cross-Origin-Embedder-Policy", "require-corp")
        self.send_header("Cross-Origin-Resource-Policy", "same-origin")
        self.send_header("Cache-Control", "no-store")
        super().end_headers()


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--bind", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=8000)
    parser.add_argument("--directory", type=Path, default=ROOT / "build" / "web-dist")
    args = parser.parse_args()

    directory = args.directory.resolve()
    if not directory.is_dir():
        parser.error(f"build directory does not exist: {directory}")

    handler = partial(IsolatedRequestHandler, directory=str(directory))
    server = ThreadingHTTPServer((args.bind, args.port), handler)
    print(f"Serving {directory} at http://{args.bind}:{args.port}/th08-web.html", flush=True)
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        pass
    finally:
        server.server_close()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
