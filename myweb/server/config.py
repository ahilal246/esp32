# ============================================================
#  SETTINGS  -  this is the only Python file you need to change
# ============================================================

# IP address of the ESP32. It prints it in the Arduino Serial Monitor
# after it connects to WiFi:  "ESP32 IP address: 192.168.1.50"
ESP32_ADDRESS = "192.168.1.50"

# Port of the web page on this computer: http://localhost:8000
PORT = 8000

# True  = also make a public Cloudflare link you can open from anywhere
# False = only this computer can open the page
USE_CLOUDFLARE = True
