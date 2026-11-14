# Web side: ESP32-CAM video as WebRTC, online through Cloudflare

A website with the live camera video and two buttons. It runs on your
computer and is also reachable from anywhere through a Cloudflare link.

```
VIDEO    ESP32-CAM --JPEG--> FFmpeg --H.264--> MediaMTX --WebRTC--> browser
BUTTONS  browser --click--> server.js --> ESP32-CAM

Local website    http://localhost:3000
Online website   https://....trycloudflare.com   (cloudflared, same website)
```

The ESP32 and the computer must be on the same WiFi. The computer has to
stay on, with `node server.js` running, for as long as you want to watch.

## Install once

| Program | What it does | How to get it |
|---|---|---|
| **Node.js** 18 or newer | runs `server.js` | https://nodejs.org |
| **MediaMTX** | makes the WebRTC video | Download from https://github.com/bluenviron/mediamtx/releases and put `mediamtx.exe` (Windows) or `mediamtx` in **this folder** |
| **FFmpeg** | turns the JPEG pictures into video | Windows: `winget install Gyan.FFmpeg` · Mac: `brew install ffmpeg` · Linux: `sudo apt install ffmpeg` |
| **cloudflared** | makes the online link | Windows: `winget install Cloudflare.cloudflared` · Mac: `brew install cloudflared` |

After `winget install`, open a new terminal so the programs are found.
No `npm install` is needed. Tested with MediaMTX v1.21.3.

FFmpeg is needed because browsers cannot play JPEG pictures over WebRTC;
they need real video (H.264).

## Start

1. Upload the sketch in `../esp32side` to the board and note the IP address
   it prints in the Serial Monitor.
2. Put that address in `config.js`:
   ```js
   ESP32_IP: '192.168.1.50',
   ```
3. In this folder run:
   ```
   node server.js
   ```
4. It prints both addresses:
   ```
   Local website:  http://localhost:3000
   Video: receiving pictures from the ESP32

   ==================================================
     Online website:  https://some-random-words.trycloudflare.com
   ==================================================
   ```
   Open either one. Stop everything with `Ctrl+C`.

The online link is new every time you start the server.

## Files

| File | What it is |
|---|---|
| `config.js` | **Your settings**: the ESP32's address, port, pictures per second |
| `server.js` | The website. This is the one you start |
| `esp32.js` | Sends the button clicks to the ESP32 |
| `video.js` | Starts FFmpeg and MediaMTX (JPEG → WebRTC) |
| `tunnel.js` | Starts cloudflared (the online link) |
| `programs.js` | Helper that starts the three programs |
| `mediamtx.yml` | Settings of MediaMTX |
| `public/index.html` | The web page: video and two buttons |

## Good to know

- **The video does not travel through Cloudflare.** Cloudflare carries the
  web page, the button clicks and the WebRTC "handshake". The video itself
  goes directly from your computer to the viewer. A STUN server (set in
  `mediamtx.yml`) helps the two find each other, and that works on most
  home internet connections.
- **If a viewer sees the page but no video** (this happens on some mobile
  data networks and strict company networks), a direct path is not possible
  and you need a TURN server. Add it in `mediamtx.yml`; the place is marked.
- **Anyone who has the online link can press the buttons.** Only share it
  with people you trust.
- **Windows Firewall** may ask about MediaMTX the first time. Allow it.
- The ESP32 sends video to one reader at a time, and here that reader is
  FFmpeg. Close the board's own test page (`http://<board ip>/`) before you
  start the server.

## If something does not work

| What you see | What to check |
|---|---|
| `"mediamtx" is not installed` (or ffmpeg, cloudflared) | Install it (table above). After winget, open a new terminal |
| Page says "The camera is offline" | Is the board on? Is `ESP32_IP` right? Open `http://<board ip>:81/stream` in a browser to test, then close it again |
| Buttons answer "No answer from the ESP32" | Same checks as above |
| `Port 3000 is already in use` | The server is already running, or change `WEB_PORT` in `config.js` |
| `MediaMTX stopped` again and again | Read the `MediaMTX: ... ERR` line above it. Usually an old MediaMTX version: download the newest |
| Video works at home but not online | See "If a viewer sees the page but no video" above |
