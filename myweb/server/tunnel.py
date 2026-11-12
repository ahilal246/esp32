"""Makes the public Cloudflare link for the local web server.

You do not need to edit this file.

It runs Cloudflare's free "cloudflared" program, which connects this
computer to Cloudflare and gives back a public address like
https://some-random-words.trycloudflare.com  -  no account needed.
The address is new every time you start the server.
"""
import collections
import re
import shutil
import subprocess
import threading
from pathlib import Path

HOW_TO_INSTALL = """\
Cloudflare: the "cloudflared" program was not found, so there is no public link.
Install it once, then start this server again:
  Windows:  winget install --id Cloudflare.cloudflared   (then open a NEW terminal)
  Mac:      brew install cloudflared
  Linux:    https://pkg.cloudflare.com
Or download it from https://github.com/cloudflare/cloudflared/releases
and put the file next to server.py, named cloudflared (cloudflared.exe on Windows)."""

PUBLIC_LINK = re.compile(r"https://[a-z0-9-]+\.trycloudflare\.com")


def _find_cloudflared():
    """Look next to this file first, then among the installed programs."""
    here = Path(__file__).parent
    for name in ("cloudflared.exe", "cloudflared"):
        if (here / name).is_file():
            return str(here / name)
    return shutil.which("cloudflared")


def _watch(process):
    """Read what cloudflared prints and show the public link when it appears."""
    last_lines = collections.deque(maxlen=15)
    link_shown = False
    for line in process.stdout:
        last_lines.append(line.rstrip())
        link = PUBLIC_LINK.search(line)
        if link and "api.trycloudflare.com" not in link.group():
            link_shown = True
            print()
            print("=" * 64)
            print("  Open this link in any browser, anywhere:")
            print("  " + link.group())
            print("=" * 64)
            print()

    # The loop only ends when cloudflared has stopped.
    if not link_shown:
        print("Cloudflare: could not make the public link. Last messages from cloudflared:")
        for line in last_lines:
            print("  " + line)


def start(port):
    """Start the tunnel to http://localhost:<port>.

    Returns the running cloudflared process (call .terminate() on it to
    stop sharing), or None if cloudflared is not installed.
    """
    program = _find_cloudflared()
    if program is None:
        print(HOW_TO_INSTALL)
        return None

    print("Cloudflare: making the public link, this takes a few seconds...")
    process = subprocess.Popen(
        [program, "tunnel", "--url", f"http://localhost:{port}"],
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,   # cloudflared prints the link on stderr
        text=True,
        errors="replace",
    )
    threading.Thread(target=_watch, args=(process,), daemon=True).start()
    return process
