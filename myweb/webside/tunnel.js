// ============================================================
//  Online website: makes a public Cloudflare link to the local
//  website.  You do not need to edit this file.
//
//    startTunnel(port);   call once, after the local website runs
//
//  It uses a free "quick tunnel": no Cloudflare account needed.
//  The link is new every time you start the server.
// ============================================================

const { startProgram } = require('./programs');

function startTunnel(port) {
  let linkShown = false;

  startProgram('cloudflared', ['tunnel', '--no-autoupdate', '--url', `http://localhost:${port}`], {
    onLine: (line) => {
      const link = line.match(/https:\/\/[a-z0-9-]+\.trycloudflare\.com/);
      if (link && !linkShown) {
        linkShown = true;
        console.log('\n==================================================');
        console.log('  Online website:  ' + link[0]);
        console.log('==================================================\n');
      } else if (/\bERR\b/.test(line)) {
        console.log('cloudflared: ' + line);
      }
    },
    onExit: ({ missing }) => {
      if (missing) return;
      console.log('cloudflared stopped. Starting it again in 5 s (the link will change)...');
      setTimeout(() => startTunnel(port), 5000);
    },
  });
}

module.exports = { startTunnel };
