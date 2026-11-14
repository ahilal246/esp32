// ============================================================
//  Video: ESP32 JPEG pictures  ->  WebRTC.
//  You do not need to edit this file.
//
//    startVideo();   call once when the server starts
//
//  Two programs do the work:
//
//    FFmpeg     reads the JPEG pictures from the ESP32, turns them
//               into H.264 video (browsers cannot play JPEG pictures
//               over WebRTC) and hands that to MediaMTX.
//    MediaMTX   offers the video to the browsers as WebRTC.
//               Its settings are in mediamtx.yml
//
//  If the ESP32 is off or out of reach, FFmpeg stops and is started
//  again every few seconds until the board is back.
// ============================================================

const config = require('./config');
const esp32 = require('./esp32');
const { startProgram } = require('./programs');

// These must match mediamtx.yml
const STREAM_NAME = 'cam';
const MEDIAMTX_RTSP = `rtsp://127.0.0.1:8554/${STREAM_NAME}`;   // FFmpeg sends the video here
const MEDIAMTX_WEB  = { host: '127.0.0.1', port: 8889 };        // browsers ask for WebRTC here
const WEBRTC_PATH   = `/${STREAM_NAME}/whep`;

const RETRY_SECONDS = 3;

// The ESP32 sends pictures as fast as it can, without a clock. This stamps
// every picture with the moment it arrived, so the video plays at real speed.
const ARRIVAL_TIME = 'setpts=(RTCTIME-RTCSTART)/(TB*1000000)';

function startMediaMTX() {
  startProgram('mediamtx', ['mediamtx.yml'], {
    onLine: (line) => {
      // MediaMTX is chatty; only show warnings and errors.
      if (/\b(WAR|ERR)\b/.test(line)) console.log('MediaMTX: ' + line);
    },
    onExit: ({ missing }) => {
      if (missing) return;
      console.log(`MediaMTX stopped. Starting it again in ${RETRY_SECONDS} s...`);
      setTimeout(startMediaMTX, RETRY_SECONDS * 1000);
    },
  });
}

function startFFmpeg() {
  let live = false;
  const keyframeEvery = config.VIDEO_FPS * 2;   // a new viewer waits at most 2 s for a picture

  const args = [
    '-nostdin', '-hide_banner', '-loglevel', 'error', '-stats',

    // Input: the JPEG stream from the ESP32
    '-rw_timeout', '5000000',   // give up after 5 s without data
    '-fflags', 'nobuffer',
    '-f', 'mpjpeg',
    '-i', esp32.streamUrl,

    // Output: H.264 that every browser can play, tuned for low delay
    '-an',
    '-vf', `${ARRIVAL_TIME},fps=${config.VIDEO_FPS},format=yuv420p`,
    '-c:v', 'libx264', '-preset', 'ultrafast', '-tune', 'zerolatency',
    '-profile:v', 'baseline', '-bf', '0', '-g', String(keyframeEvery),
    '-f', 'rtsp', '-rtsp_transport', 'tcp', MEDIAMTX_RTSP,
  ];

  startProgram('ffmpeg', args, {
    onLine: (line) => {
      if (line.startsWith('frame=')) {   // FFmpeg's progress line: pictures are flowing
        if (!live) console.log('Video: receiving pictures from the ESP32');
        live = true;
      } else if (live) {
        console.log('FFmpeg: ' + line);
      }
    },
    onExit: ({ missing }) => {
      if (missing) return;
      if (live) console.log(`Video: lost the ESP32 (${esp32.streamUrl}). Trying again...`);
      setTimeout(startFFmpeg, RETRY_SECONDS * 1000);
    },
  });
}

function startVideo() {
  console.log(`Video: waiting for the ESP32 at ${esp32.streamUrl}`);
  startMediaMTX();
  setTimeout(startFFmpeg, 1000);   // give MediaMTX a moment to start first
}

module.exports = { startVideo, MEDIAMTX_WEB, WEBRTC_PATH };
