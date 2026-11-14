// ============================================================
//  Local website for the ESP32-CAM.   Start it with:   node server.js
//  Settings are in config.js
//
//    ESP32-CAM --JPEG--> FFmpeg --> MediaMTX --WebRTC--> browser
//    browser --button click--> this server --> ESP32-CAM
//
//  The same website is reachable in two ways:
//    local    http://localhost:3000
//    online   the https://....trycloudflare.com link printed below
//
//  Files in this folder
//    config.js      YOUR SETTINGS: the ESP32's address
//    server.js      this file: the website
//    esp32.js       sends the button clicks to the ESP32
//    video.js       starts FFmpeg and MediaMTX (JPEG -> WebRTC)
//    tunnel.js      starts cloudflared (the online link)
//    programs.js    helper that starts the three programs
//    mediamtx.yml   settings of MediaMTX
//    public/        the web page
// ============================================================

const http = require('http');
const fs = require('fs');
const path = require('path');

const config = require('./config');
const esp32 = require('./esp32');
const video = require('./video');
const tunnel = require('./tunnel');
const { stopAllPrograms } = require('./programs');

const PAGE = path.join(__dirname, 'public', 'index.html');

function sendText(response, status, text) {
  response.writeHead(status, { 'Content-Type': 'text/plain; charset=utf-8', 'Cache-Control': 'no-store' });
  response.end(text);
}

// "/" : the web page
function sendPage(response) {
  fs.readFile(PAGE, (error, html) => {
    if (error) return sendText(response, 500, 'public/index.html is missing');
    response.writeHead(200, { 'Content-Type': 'text/html; charset=utf-8', 'Cache-Control': 'no-store' });
    response.end(html);
  });
}

// "/api/button1" : pass a button click on to the ESP32
async function handleButton(name, response) {
  try {
    const answer = await esp32.pressButton(name);
    console.log(`${name} -> ${answer}`);
    sendText(response, 200, answer);
  } catch (error) {
    console.log(`${name} -> no answer from the ESP32 (${error.message})`);
    sendText(response, 502, 'No answer from the ESP32');
  }
}

// "/cam/whep" : the browser and MediaMTX agree on the WebRTC connection.
// The request is passed on to MediaMTX unchanged.
function passToMediaMTX(request, response) {
  const forward = http.request({
    host: video.MEDIAMTX_WEB.host,
    port: video.MEDIAMTX_WEB.port,
    method: request.method,
    path: request.url,
    headers: { ...request.headers, host: `${video.MEDIAMTX_WEB.host}:${video.MEDIAMTX_WEB.port}` },
  }, (answer) => {
    response.writeHead(answer.statusCode, answer.headers);
    answer.pipe(response);
  });
  forward.on('error', () => {
    if (!response.headersSent) sendText(response, 502, 'MediaMTX is not running');
    else response.destroy();
  });
  request.pipe(forward);
}

const server = http.createServer((request, response) => {
  const url = new URL(request.url, 'http://localhost');
  const button = url.pathname.startsWith('/api/') ? url.pathname.slice(5) : '';

  if (url.pathname === video.WEBRTC_PATH || url.pathname.startsWith(video.WEBRTC_PATH + '/')) {
    passToMediaMTX(request, response);
  } else if (request.method !== 'GET') {
    sendText(response, 405, 'Not allowed');
  } else if (url.pathname === '/') {
    sendPage(response);
  } else if (config.ESP32_BUTTONS[button]) {
    handleButton(button, response);
  } else {
    sendText(response, 404, 'Not found');
  }
});

server.on('error', (error) => {
  if (error.code === 'EADDRINUSE') {
    console.log(`Port ${config.WEB_PORT} is already in use. Is the server already running? ` +
                'Close it, or change WEB_PORT in config.js');
  } else {
    console.log('Website error: ' + error.message);
  }
  process.exit(1);
});

server.listen(config.WEB_PORT, () => {
  console.log(`Local website:  http://localhost:${config.WEB_PORT}`);
  video.startVideo();
  if (config.USE_CLOUDFLARE) tunnel.startTunnel(config.WEB_PORT);
});

// Ctrl+C: stop FFmpeg, MediaMTX and cloudflared too.
process.on('exit', stopAllPrograms);
process.on('SIGINT', () => process.exit(0));
process.on('SIGTERM', () => process.exit(0));
