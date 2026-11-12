"""Talks to the ESP32: switches the LED and reads the video.

You do not need to edit this file. The ESP32's address is in config.py
"""
import http.client
import threading
import time

import config

CONTROL_PORT = 80   # same ports as in web_server.cpp on the ESP32
STREAM_PORT = 81

# Everything that can go wrong while talking to the ESP32
# (switched off, wrong address, WiFi dropped, ...).
ERRORS = (OSError, http.client.HTTPException, ValueError)


def led(path):
    """Ask the ESP32 for "/led/on", "/led/off" or "/led" (= current state).

    Returns "on" or "off", or None if the ESP32 did not answer.
    """
    connection = http.client.HTTPConnection(config.ESP32_ADDRESS, CONTROL_PORT, timeout=5)
    try:
        connection.request("GET", path)
        answer = connection.getresponse()
        text = answer.read().decode().strip()
        return text if answer.status == 200 else None
    except ERRORS:
        return None
    finally:
        connection.close()


class VideoRelay:
    """Reads the video from the ESP32 once and shares it with every viewer.

    The ESP32 can only send its video to one receiver at a time. So this
    server is that one receiver, and it hands a copy of every picture to
    each open browser. While nobody is watching it disconnects, so the
    ESP32 can rest.
    """

    def __init__(self):
        self._changed = threading.Condition()
        self._picture = None   # the newest picture (JPEG bytes)
        self._number = 0       # goes up by one with every new picture
        self._viewers = 0
        self._connected = None   # None = not tried yet
        threading.Thread(target=self._run, daemon=True).start()

    # ---- used by the web server, once per open browser ----------

    def add_viewer(self):
        """Call when a browser starts watching. Returns the number to pass
        to the first next_picture()."""
        with self._changed:
            self._viewers += 1
            self._changed.notify_all()   # wakes up the reader below
            return self._number

    def remove_viewer(self):
        with self._changed:
            self._viewers -= 1

    def next_picture(self, seen):
        """Wait for a picture newer than number `seen`.

        Returns (number, picture). The picture is None if the camera sent
        nothing new for 5 seconds (it is switched off or out of reach).
        """
        with self._changed:
            if self._changed.wait_for(lambda: self._number > seen, timeout=5):
                return self._number, self._picture
            return seen, None

    # ---- background thread: ESP32 -> self._picture --------------

    def _run(self):
        while True:
            with self._changed:
                self._changed.wait_for(lambda: self._viewers > 0)
            try:
                self._read_stream()
            except ERRORS as error:
                if self._connected is not False:   # say it once, not every 2 seconds
                    print(f"Camera: no video from {config.ESP32_ADDRESS} ({error}). "
                          "Is the ESP32 on, and is ESP32_ADDRESS in config.py right? "
                          "Trying again every 2 seconds...")
                self._connected = False
                time.sleep(2)

    def _read_stream(self):
        connection = http.client.HTTPConnection(config.ESP32_ADDRESS, STREAM_PORT, timeout=10)
        try:
            connection.request("GET", "/stream")
            stream = connection.getresponse()
            if stream.status != 200:
                raise ConnectionError(f"ESP32 answered {stream.status}")
            print("Camera: connected")
            self._connected = True
            while self._viewers > 0:
                picture = _read_picture(stream)
                with self._changed:
                    self._picture = picture
                    self._number += 1
                    self._changed.notify_all()
            print("Camera: paused, nobody is watching")
        finally:
            connection.close()


def _read_picture(stream):
    """Read one JPEG picture out of the ESP32's endless video answer.

    Each picture arrives like this:
        --frame
        Content-Type: image/jpeg
        Content-Length: 12345
        (empty line)
        ...12345 bytes of JPEG...
    """
    length = None
    while True:
        line = stream.readline()
        if not line:
            raise ConnectionError("video ended")
        line = line.strip()
        if line.lower().startswith(b"content-length:"):
            length = int(line.split(b":")[1])
        elif not line and length is not None:
            break   # empty line after the length: the picture starts here

    picture = stream.read(length)
    if len(picture) != length:
        raise ConnectionError("video ended")
    return picture
