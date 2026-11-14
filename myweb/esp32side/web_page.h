// ============================================================
//  The test page shown at http://<board ip>/
//  Plain HTML: change the text or the look here if you like.
// ============================================================
#pragma once

#include <Arduino.h>

static const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>ESP32-CAM</title>
<style>
  body { margin: 0; padding: 16px; background: #0e1116; color: #e8ebf0;
         font: 16px system-ui, sans-serif; text-align: center; }
  img { width: 100%; max-width: 640px; background: #000; border-radius: 10px; }
  button { font: inherit; padding: 12px 18px; margin: 10px 6px; border-radius: 10px;
           border: 1px solid #2c3442; background: #1b212b; color: inherit; }
  p { color: #98a2b3; font-size: 14px; }
</style>
</head>
<body>
  <h3>ESP32-CAM test page</h3>
  <img id="video" alt="video">
  <div>
    <button onclick="press('/button1')">Button 1</button>
    <button onclick="press('/button2')">Button 2</button>
  </div>
  <p id="reply">&nbsp;</p>
  <p>The board sends video to one place at a time. Close this page before
     starting the WebRTC sender.</p>
<script>
  document.getElementById('video').src = 'http://' + location.hostname + ':81/stream';
  function press(path) {
    fetch(path).then(r => r.text()).then(t => document.getElementById('reply').textContent = t)
               .catch(() => document.getElementById('reply').textContent = 'No answer from the board');
  }
</script>
</body>
</html>
)rawliteral";
