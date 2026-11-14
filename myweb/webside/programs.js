// ============================================================
//  Helper that starts the other programs (MediaMTX, FFmpeg,
//  cloudflared).  You do not need to edit this file.
//
//    startProgram('ffmpeg', [arguments], { onLine, onExit })
//
//  A program is taken from this folder if it is here (for example
//  mediamtx.exe next to server.js), otherwise from the PATH.
// ============================================================

const { spawn } = require('child_process');
const fs = require('fs');
const path = require('path');
const readline = require('readline');

const HOW_TO_INSTALL = {
  mediamtx: 'Download it from https://github.com/bluenviron/mediamtx/releases and put mediamtx(.exe) in this folder.',
  ffmpeg: 'Windows: winget install Gyan.FFmpeg   Mac: brew install ffmpeg   Linux: sudo apt install ffmpeg',
  cloudflared: 'Windows: winget install Cloudflare.cloudflared   Mac: brew install cloudflared',
};

const running = new Set();

function findProgram(name) {
  for (const file of [name + '.exe', name]) {
    const here = path.join(__dirname, file);
    if (fs.existsSync(here)) return here;
  }
  return name;   // let the system look for it in the PATH
}

function startProgram(name, args, { onLine = () => {}, onExit = () => {} } = {}) {
  const child = spawn(findProgram(name), args, { cwd: __dirname, windowsHide: true });
  running.add(child);

  let finished = false;
  const finish = (info) => {
    if (finished) return;
    finished = true;
    running.delete(child);
    onExit(info);
  };

  child.on('error', (error) => {
    if (error.code === 'ENOENT') {
      console.log(`\n"${name}" is not installed. ${HOW_TO_INSTALL[name] || ''}\n`);
      finish({ missing: true });
    } else {
      console.log(`${name}: ${error.message}`);
      finish({ missing: false });
    }
  });
  child.on('exit', () => finish({ missing: false }));

  // FFmpeg rewrites its status line with \r, so split on that too.
  for (const stream of [child.stdout, child.stderr]) {
    readline.createInterface({ input: stream }).on('line', (text) => {
      for (const line of text.split('\r')) if (line.trim()) onLine(line.trim());
    });
  }
  return child;
}

function stopAllPrograms() {
  for (const child of running) child.kill();
}

module.exports = { startProgram, stopAllPrograms };
