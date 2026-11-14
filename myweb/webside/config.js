// ============================================================
//  SETTINGS  -  this is the only file you need to change
// ============================================================

module.exports = {
  // ---- 1. Your ESP32-CAM -----------------------------------
  // The address the board prints in the Serial Monitor.
  ESP32_IP: '192.168.1.50',

  // Where the board serves the video and the buttons.
  // These match the esp32side sketch, so normally leave them alone.
  ESP32_STREAM: ':81/stream',
  ESP32_BUTTONS: {
    button1: '/button1',
    button2: '/button2',
  },

  // ---- 2. The local website --------------------------------
  // On this computer the page is at http://localhost:3000
  WEB_PORT: 3000,

  // ---- 3. Video --------------------------------------------
  // Pictures per second sent to the viewers. The ESP32 itself decides how
  // many it can take; 15 is a good value for a QVGA/VGA picture.
  VIDEO_FPS: 15,

  // ---- 4. Online website -----------------------------------
  // true  = also make a public https://....trycloudflare.com link
  // false = local website only
  USE_CLOUDFLARE: true,
};
