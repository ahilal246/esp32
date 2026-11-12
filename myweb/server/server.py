"""Web page for the ESP32-CAM: live video and two LED buttons.

Start it with:   python server.py

    browser  <-->  Cloudflare  <-->  this server  <-->  ESP32 (same WiFi)

Settings are in config.py. Needs Python 3.8 or newer, nothing to install
with pip.
"""
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path

import config
import esp32_link
import tunnel

HERE = Path(__file__).parent
PAGE = HERE / "index.html"
# Shown in the video box while the ESP32 sends no pictures.
OFFLINE_PICTURE = (HERE / "offline.jpg").read_bytes()

video = esp32_link.VideoRelay()


class Handler(BaseHTTPRequestHandler):
    timeout = 30   # give up on a browser that stopped responding

    # ---- which address does what ---------------------------------

    def do_GET(self):
        path = self.path.split("?")[0]
        if path == "/":
            self._send(200, PAGE.read_bytes(), "text/html; charset=utf-8")
        elif path == "/video":
            self._send_video()
        elif path == "/led":
            self._send_led("/led")
        else:
            self._send(404, b"Not found")

    def do_POST(self):
        if self.path in ("/led/on", "/led/off"):
            self._send_led(self.path)
        else:
            self._send(404, b"Not found")

    # ---- answers -------------------------------------------------

    def _send(self, status, body, content_type="text/plain; charset=utf-8"):
        self.send_response(status)
        self.send_header("Content-Type", content_type)
        self.send_header("Content-Length", str(len(body)))
        self.send_header("Cache-Control", "no-store")
        self.end_headers()
        self.wfile.write(body)

    def _send_led(self, esp32_path):
        """Pass the LED command on to the ESP32 and answer "on" or "off"."""
        state = esp32_link.led(esp32_path)
        if state is None:
            self._send(502, b"ESP32 not reachable")
            return
        if esp32_path != "/led":
            print("LED", state)
        self._send(200, state.encode())

    def _send_video(self):
        """One endless answer: picture after picture until the page is closed."""
        self.send_response(200)
        self.send_header("Content-Type", "multipart/x-mixed-replace; boundary=frame")
        self.send_header("Cache-Control", "no-store")
        self.end_headers()

        # Every picture is announced by these lines. A browser only draws a
        # picture once it has seen the announcement of the NEXT one, so the
        # announcement is sent right behind each picture instead of waiting
        # for the next picture. Otherwise the page is always one picture late.
        next_picture_follows = b"--frame\r\nContent-Type: image/jpeg\r\n\r\n"

        seen = video.add_viewer()
        try:
            self.wfile.write(next_picture_follows)
            while True:
                seen, picture = video.next_picture(seen)
                if picture is None:
                    picture = OFFLINE_PICTURE
                self.wfile.write(picture + b"\r\n" + next_picture_follows)
        except OSError:
            pass   # the browser closed the page
        finally:
            video.remove_viewer()

    def log_message(self, format, *args):
        pass   # do not print a line for every request


def main():
    # 127.0.0.1 = only programs on this computer (the browser here and
    # cloudflared) can connect directly; everyone else comes in through
    # the Cloudflare link.
    server = ThreadingHTTPServer(("127.0.0.1", config.PORT), Handler)

    print(f"Server running. On this computer: http://localhost:{config.PORT}")
    print(f"ESP32 address: {config.ESP32_ADDRESS}")
    print("Press Ctrl+C to stop.")

    cloudflared = tunnel.start(config.PORT) if config.USE_CLOUDFLARE else None
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\nStopped.")
    finally:
        if cloudflared is not None:
            cloudflared.terminate()


if __name__ == "__main__":
    main()
