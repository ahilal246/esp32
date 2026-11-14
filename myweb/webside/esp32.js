// ============================================================
//  Talks to the ESP32-CAM.  You do not need to edit this file.
//
//    streamUrl             where the board serves its JPEG video
//    pressButton(name)     tells the board a button was clicked and
//                          returns its answer, e.g. "Button 1: ON"
//
//  The board's address is in config.js
// ============================================================

const config = require('./config');

const streamUrl = `http://${config.ESP32_IP}${config.ESP32_STREAM}`;

async function pressButton(name) {
  const buttonPath = config.ESP32_BUTTONS[name];
  if (!buttonPath) throw new Error(`Unknown button "${name}"`);

  // Give up after 3 seconds so the web page is not left waiting.
  const reply = await fetch(`http://${config.ESP32_IP}${buttonPath}`, {
    signal: AbortSignal.timeout(3000),
  });
  if (!reply.ok) throw new Error(`The board answered with error ${reply.status}`);
  return (await reply.text()).trim();
}

module.exports = { streamUrl, pressButton };
