# TikTok Bike

The faster you ride, the faster TikTok moves on. An MPU6050 on the bike
wheel measures how fast the wheel turns (RPM). The ESP32 is a Bluetooth
mouse that scrolls down to the next video:

| Wheel speed | Next video every |
|---|---|
| faster than `RPM_X` (default 120 RPM) | 40 s |
| slower than `RPM_X` | 200 s |

No buttons any more.

## Files

| File | What it is |
|---|---|
| `config.h` | **Your settings**: `RPM_X`, the 40 s / 200 s times, pins, Bluetooth name |
| `tiktok_bike.ino` | The sketch: picks 40 s or 200 s from the RPM and changes the video |
| `tiktok_mouse.h` / `tiktok_mouse.cpp` | The Bluetooth mouse code. No need to edit |
| `mpu6050_rpm.h` / `mpu6050_rpm.cpp` | The MPU6050 code (MPU6050_light library): wheel RPM from the gyro. No need to edit |

## How the RPM is measured

The gyro gives the turning speed around its 3 axes (gx, gy, gz in
degrees per second). Together they make the wheel's turning speed,
whichever way the sensor faces:

```
deg/s = square root of (gx*gx + gy*gy + gz*gz)
RPM   = deg/s / 6        (360 deg/s = 1 turn a second = 60 RPM)
```

The gyro range is set to ±2000 deg/s with `mpu.begin(3)`. The library's
default, ±500 deg/s, would stop at 83 RPM, below X.

## Wiring

| MPU6050 | ESP32 |
|---|---|
| VCC | 3V3 |
| GND | GND |
| SCL | GPIO 22 |
| SDA | GPIO 21 |

Leave AD0, XDA, XCL and INT unconnected. The old buttons on GPIO 26/27
are not used any more.

## Mounting on the wheel

- **The ESP32 and its battery ride on the wheel too**, next to the
  MPU6050. Wires can't go from a turning wheel to the frame: they would
  wind up around the axle. Bluetooth works fine from the wheel.
- Fix everything to the spokes **near the hub** (the middle of the
  wheel), not on the tyre. The tyre touches the ground or the trainer
  roller, and near the hub the parts feel much less force.
- Any spot and any direction works. The sensor measures turning in all
  directions, so it doesn't need to be lined up.
- Fasten it well (tape and zip ties). Put the battery on the opposite
  side of the hub from the ESP32 so the wheel stays balanced.
- Power: a small USB power bank in the ESP32's USB port is the easiest.

## Setup

1. Libraries (Library Manager): **HijelHID_BLEMouse** and
   **NimBLE-Arduino** (same as before), plus **MPU6050_light** by rfetick.
2. Board: your ESP32 (e.g. *ESP32 Dev Module*), ESP32 core 3.x.
3. Open `tiktok_bike.ino` and upload.
4. Switch it on with the wheel **still** for a few seconds: it measures
   the gyro's zero point.
5. Pair **TikTok Remote** in the phone's Bluetooth settings (iPhone: turn
   on Settings > Accessibility > Touch > AssistiveTouch first). Open TikTok.

The Bluetooth name is the same as the button version, so a phone that
was already paired should reconnect by itself.

## Choosing X (`RPM_X` in `config.h`)

RPM = km/h × 16.7 ÷ wheel circumference in metres

| Speed | Normal wheel, 26" or 700c (≈ 2.1 m) | 29" wheel (≈ 2.3 m) |
|---|---|---|
| 10 km/h | 79 RPM | 72 RPM |
| 15 km/h | 119 RPM | 109 RPM |
| 20 km/h | 159 RPM | 145 RPM |
| 25 km/h | 198 RPM | 181 RPM |
| 30 km/h | 238 RPM | 217 RPM |

The sensor reads up to about 330 RPM (about 42 km/h on a normal wheel).
Faster than that still shows about 330, which still counts as fast.

## How the timing works

- The clock starts when the phone connects and starts again after every
  video change.
- It uses your speed **right now**. Example: you rode slowly for 60 s,
  then speed up. The video changes straight away, because 60 s is
  already more than 40 s.
- The RPM is averaged over about 1 s, so one bump doesn't count.
- If a sensor wire comes loose, the RPM drops to about 0, which counts
  as slow (200 s videos).

## Test at your desk

Keep USB plugged in, open Serial Monitor at 115200, and turn the ESP32
and sensor around in your hand. One turn per second is 60 RPM. Every
second it prints a line like:

```
RPM 142  FAST  next video in 39 s
```

## Troubleshooting

| You see | Do this |
|---|---|
| `MPU6050: not found!` | Check the 4 wires; SDA and SCL may be swapped. Leave AD0 unconnected |
| A few RPM when still, or RPM looks wrong | Switch on again with the wheel still (it calibrates at power on) |
| RPM stays at 0 while riding | A wire came loose: fix it and restart the ESP32 |
| One scroll doesn't change the video | Set `SCROLL_STEPS` to 2 or 3 |
| Phone doesn't reconnect | Remove *TikTok Remote* in the phone's Bluetooth settings and pair again |
