# ESP32-CAM video in the browser, shared through Cloudflare

A web page with the live camera video and two buttons: **LED ON** and **LED OFF**.
You can open it from anywhere with a public Cloudflare link.

```
browser (anywhere)  <-->  Cloudflare  <-->  Python server (your computer)  <-->  ESP32-CAM
                                                         same WiFi ------------------^
```

The ESP32 and the computer must be on the same WiFi. The computer has to stay
on, with the Python server running, for as long as you want to share.

## Files

**`esp32cam/`** – the ESP32 side (Arduino sketch)

| File | What it is |
|---|---|
| `config.h` | **Your settings**: WiFi, LED pin, picture size |
| `esp32cam.ino` | The sketch: starts the four parts below |
| `wifi_setup.h` / `.cpp` | Connects to WiFi, prints the IP address |
| `camera.h` / `.cpp` | Starts the camera, takes pictures |
| `led.h` / `.cpp` | Switches the LED |
| `web_server.h` / `.cpp` | Sends the video and receives the LED commands |

**`server/`** – the computer side (Python)

| File | What it is |
|---|---|
| `config.py` | **Your settings**: the ESP32's IP address |
| `server.py` | The web server. This is the one you start |
| `esp32_link.py` | Talks to the ESP32 (video and LED) |
| `tunnel.py` | Makes the public Cloudflare link |
| `index.html` | The web page: video and two buttons |
| `offline.jpg` | Picture shown while the camera is not reachable |

## Step 1 – ESP32

1. Open `esp32cam/esp32cam.ino` in the Arduino IDE.
2. Fill in your WiFi name and password in `config.h`.
3. Tools → Board → **AI Thinker ESP32-CAM**, then upload.
4. Open the Serial Monitor at 115200. Note the address it prints:
   ```
   ESP32 IP address: 192.168.1.50
   ```

Quick test without Python: on a phone or computer on the same WiFi, open
`http://192.168.1.50:81/stream` (your address) to see the video, and
`http://192.168.1.50/led/on` to switch the LED on.

## Step 2 – Python server

1. Put the ESP32's address in `server/config.py`:
   ```python
   ESP32_ADDRESS = "192.168.1.50"
   ```
2. Install **cloudflared** once (Cloudflare's free tunnel program, no account needed):
   - Windows: `winget install --id Cloudflare.cloudflared`, then open a new terminal
   - Mac: `brew install cloudflared`
   - Linux: see https://pkg.cloudflare.com
3. Start the server (Python 3.8 or newer, nothing to install with pip):
   ```
   cd myweb/server
   python server.py
   ```
4. After a few seconds it prints the public link:
   ```
   ================================================================
     Open this link in any browser, anywhere:
     https://some-random-words.trycloudflare.com
   ================================================================
   ```

Open that link on any phone or computer. Press Ctrl+C to stop sharing.

On the computer itself the page is also at http://localhost:8000, with or
without Cloudflare.

## Good to know

- **The link is new every time** you start the server.
- **Anyone who has the link can watch the camera and switch the LED.** Only
  give it to people you trust, and stop the server when you are done.
- Several people can watch at the same time. While nobody is watching, the
  server disconnects from the camera so the ESP32 can rest.
- If the video box shows **Camera offline**, the server cannot reach the
  ESP32: check that it has power, that both are on the same WiFi, and that
  `ESP32_ADDRESS` is right (the router can give the ESP32 a new address
  after a restart).

## Settings

`esp32cam/config.h`

| Setting | Meaning |
|---|---|
| `LED_PIN` | `4` = the bright white LED next to the camera, `33` = small red LED on the back, or any pin with your own LED |
| `LED_ON_LEVEL` | `HIGH` normally. `LOW` for pin 33, which is wired the other way round |
| `CAMERA_SIZE` | `FRAMESIZE_QVGA` 320x240, `FRAMESIZE_VGA` 640x480, `FRAMESIZE_SVGA` 800x600 |
| `CAMERA_QUALITY` | 10 = sharp and big … 30 = rough and small |

Smaller pictures and rougher quality make the video faster.

`server/config.py`

| Setting | Meaning |
|---|---|
| `ESP32_ADDRESS` | IP address the ESP32 prints in the Serial Monitor |
| `PORT` | Port of the page on your computer (`8000`) |
| `USE_CLOUDFLARE` | `False` = no public link, page only on your own computer |
