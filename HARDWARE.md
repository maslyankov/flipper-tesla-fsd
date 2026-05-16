# Hardware

## Quick comparison

| Option | Cost | Connection | CAN buses | WiFi | Best for |
|--------|------|------------|-----------|------|----------|
| **Any ESP32 + MCP2515 → X179** | **~$5-7** | X179 4-wire | 1 (bus 6 = mixed) | Yes | Cheapest full-feature setup |
| M5Stack ATOM Lite + ATOMIC CAN → X179 | ~$13-15 | X179 4-wire | 1 (bus 6) | Yes | Plug & play, no soldering |
| M5Stack ATOM Matrix + ATOMIC CAN → X179 | ~$22-25 | X179 4-wire | 1 (bus 6) | Yes | 5×5 LED status, timer-poll deep sleep |
| **LILYGO T-2CAN ESP32-S3 V1.0** → X179 + Party | **~$24** | 2× 4-wire (Chassis + Party) | **2 independent (MCP2515 + TWAI)** | Yes | Dual-bus build (env `t-2can-v1`) with EXT1 wake-on-CAN |
| **LILYGO T-CAN485** → X179 | **~$15** | X179 4-wire | 1 (SN65HVD230) | Yes | SD card CAN dump, tested on Model X/S |
| Waveshare ESP32-S3-RS485-CAN → X179 | ~$18 | X179 4-wire | 1 (TWAI) | Yes | All-in-one board |
| Flipper Zero + Electronic Cats CAN Add-On → OBD-II | ~$234 | OBD-II plug | 1 (Party CAN) | No | If you already own a Flipper |
| Flipper Zero + generic MCP2515 → OBD-II | ~$202-205 | OBD-II wire | 1 (Party CAN) | No | Budget Flipper option |

---

## Connection points on Tesla Model 3/Y

There are two places to tap the CAN bus. **The X179 connector is
recommended** — it provides more signals and built-in 12V power.

### OBD-II (16-pin) — Tesla-specific notes

The standard automotive diagnostic port. On Tesla the location and
behavior differ by model and year:

- **2017–2018 Model 3**: no OBD-II port. Use X052 instead (see below).
- **2019+ Model 3 / 2020–April 2024 Model Y**: OBD-II J1962 port
  exists, but it is **not under the steering column** — it sits in
  the rear center console area and requires a Tesla-specific adapter
  cable (e.g., OHP, EVTV, Cybertool) to expose a standard 16-pin
  socket.
- **April 2024+ Juniper Model Y / refreshed Model 3 Highland (later
  builds)**: Tesla switched to **DoIP** (Diagnostic over IP) — the
  diagnostic port now carries 100 Mbps Ethernet, **not** CAN.

> [!CAUTION]
> Do not connect a CAN-based OBD-II adapter or scan tool to a DoIP
> port. The signal levels are incompatible and connecting a J1962-on-
> CAN device into a DoIP-only port can damage the vehicle's diagnostic
> module. If you have a 2024+ Juniper, tap X179 directly — see below.

```
    ┌──────────────────────────────┐
    │  1  2  3  4  5  6  7  8     │
    │   9 10 11 12 13 14 15 16    │
    └─────────────────────────────┘
```

| Pin | Signal | Notes |
|-----|--------|-------|
| 4 | Chassis GND | |
| 5 | Signal GND | |
| **6** | **CAN-H** | **Party CAN** |
| **14** | **CAN-L** | **Party CAN** |
| **16** | **+12V always-on** | Constant power even when car is locked |

**Bus: Party CAN only.** This carries DAS, ESP, BMS, FSD gate, nag
(EPAS), ISA chime, precondition — everything our v1.0–v2.3 used.

Limitation: Party CAN does not carry stalk signals (SCCM_rightStalk),
lighting commands (VCFRONT_lighting), or steering wheel button inputs
(STW_ACTN_RQ). Those are on Vehicle CAN.

### X052 — 2019 Model 3 (pre-facelift)

The 2019 Model 3 does **not** have the X179 connector or a standard
OBD-II port under the steering column. Instead, it uses the X052
connector, located behind the center console / passenger footwell area.

Confirmed by community tester @THER4iN (issue #21):

| X052 Pin | Signal | Notes |
|----------|--------|-------|
| **44** | **CAN-H** | CAN bus |
| **45** | **CAN-L** | CAN bus |
| **20** | **12V** | Power (no service mode errors confirmed) |
| **22** | **GND** | Ground |

Same 4-wire pattern as X179 — CAN + power. Compatible with all the
same ESP32/MCP2515 setups described below.

The 2019 Model 3 also has an **X930m** connector near the A-pillar.
Pinout not yet confirmed — if you test it, please report in an issue.

### X179 — behind the rear center console (2021+ Model 3/Y)

Tesla's own service/diagnostic connector. Requires removing a trim
panel behind the rear armrest. Two versions exist:

#### X179 20-pin (2021–2023 Model 3/Y)

```
     +---------------------------------+
     |  1   2   3   4   5   6   7      |
     |  8   9  10  11  12  13  14      |
     | 15  16  17  18  19  20          |
     +---------------------------------+
```

| Pin | Signal | CAN bus |
|-----|--------|---------|
| **1** | **+12V** | Power |
| 2 | CAN-H | Bus 4 (diagnostic/forwarded) |
| 3 | CAN-L | Bus 4 |
| 9 | CAN-H | Bus 2 (Vehicle CAN) |
| 10 | CAN-L | Bus 2 |
| **13** | **CAN-H** | **Bus 6 (Body/Left — Gateway mixed forwarding)** |
| **14** | **CAN-L** | **Bus 6** |
| **15** | **+12V** | Power (2mm² wire, alternate to pin 1) |
| 18 | CAN-H | Bus 3 (Chassis CAN — EPAS/brake) |
| 19 | CAN-L | Bus 3 |
| **20** | **GND** | Ground |

**4 separate CAN bus pairs** on one connector. Pin 13/14 (bus 6) is
what aftermarket products (Feifan Commander, enhauto, etc.) connect to.

#### 26-pin rear connector — two variants

The 26-pin rear connector ships in two electrical configurations
depending on production date. **They are not interchangeable.**

> [!NOTE]
> The connector name is unsettled. The 20-pin variant is documented as
> X179 in community sources, but Tesla service documentation may use a
> different identifier for the 26-pin. If you find the official name
> in a Tesla service doc, please open an issue with the reference and
> we will update.

##### Pre-April 2024 builds (CAN)

Confirmed on community testing of 2021–2023 Model 3/Y and early 2024
Highland / Juniper builds.

| Pin | Signal | Notes |
|-----|--------|-------|
| **13** | **CAN-H** | Bus 6 (Gateway-forwarded) |
| **14** | **CAN-L** | Bus 6 |
| **15** | **+12V** | Power (red wire, 2mm²) |
| 18 | CAN-H | Vehicle CAN (blue wire) |
| 19 | CAN-L | Vehicle CAN (yellow wire) |
| **26** | **GND** | Ground (black wire, 2mm²) |

##### Post-April 2024 builds (mixed DoIP / CAN)

> [!CAUTION]
> Tesla migrated several pin pairs from CAN to **DoIP (100 Mbps
> Ethernet)** in April 2024 — coinciding with the EU's OBD compliance
> deadline. This migration appears to apply at least to EU-region
> builds and likely beyond, but the exact scope is **not yet pinned
> down** (see SOP variants below). **Do not assume your 26-pin
> follows the pre-April 2024 layout — oscilloscope-verify before
> powering up a transceiver.**

The classification axis is April 2024 production date and region, **not**
Juniper-or-not. A pre-Juniper EU-build Model Y from the same window
shows the same DoIP migration as Juniper builds.

Confirmed by @0n3-70uch via oscilloscope measurements on a Berlin-built
EU Model Y (pre-Juniper, post-April-2024 production) running 2026.14.3
(issue [#52](https://github.com/hypery11/flipper-tesla-fsd/issues/52)):

| Pin | Signal on this car |
|-----|--------------------|
| 9 / 10 | DoIP (Ethernet) — **not** CAN |
| 12 / 13 | DoIP (Ethernet) — **not** CAN |
| **18 / 19** | **Vehicle CAN — only working CAN pair** |
| 15 | +12V (unchanged) |
| 26 | GND (unchanged) |

This is a **single empirical data point** and may not generalise to
every post-April-2024 build. @TianzeWang notes (issue [#52](https://github.com/hypery11/flipper-tesla-fsd/issues/52))
that Tesla's [Model Y Electrical Reference](https://service.tesla.com/docs/ModelY/ElectricalReference/)
distinguishes between **Berlin Juniper (SOP8)** and **Shanghai Juniper
(SOP9)** — pin maps may differ further by SOP variant. Until a
broader sweep is published, the safe recommendation on any 2024+
build is:

1. **Oscilloscope-verify** every pair before connecting a transceiver.
2. If the 120 Ω differential-signal check fails on pin 13/14, the
   pair is likely DoIP — try pin 18/19 instead.
3. The +12V (pin 15) and GND (pin 26) appear stable across SOPs.

### Why X179 Pin 13/14 is the best single connection point (pre-April 2024 only)

The Gateway forwards signals from **multiple internal CAN buses** onto
bus 6 (pin 13/14). Community testing confirms that the following
"Party CAN" signals are visible on X179 pin 13/14:

- `0x3FD` UI_autopilotControl (FSD gate)
- `0x370` EPAS3S_sysStatus (nag killer)
- `0x132` BMS_hvBusStatus (battery voltage/current)
- `0x292` BMS_socStatus (state of charge)
- `0x312` BMS_thermalStatus (battery temp)
- `0x399` ISA speed limit
- `0x39B` DAS_status (AP state, blind spot)
- `0x2B9` DAS_control (ACC state)

And these "Vehicle CAN" signals are also writable on bus 6:

- `0x229` SCCM_rightStalk (gear shift, park)
- `0x3F5` VCFRONT_lighting (hazard, wiper)
- `0x249` SCCM_leftStalk (high beam, turn signal)

**One bus, one connection, reads and writes almost everything.**

This is how the 非凡指揮官 (Feifan Commander, 69K+ sales in China)
achieves its full feature set with just 4 wires:

```
X179 Pin 13 → CAN-H ─┐
X179 Pin 14 → CAN-L ──┤── CAN module (MCP2515 / TWAI)
X179 Pin 15 → 12V ────┤── buck converter → 3.3V/5V
X179 Pin 20 → GND ────┘   (26-pin: use Pin 26 for GND)
```

### enhauto Commander harness — Chassis CAN at the passenger footwell

A separate viable install path for owners of the enhauto Commander cable
who want to repurpose the harness with our firmware. The cable exposes
two CAN pairs labeled **Chassis CAN H/L** and **Vehicle CAN H/L**.

- **Chassis CAN** carries the autopilot control frames (`0x3FD`),
  steering (`0x129`), DAS_status / DAS_autopilot (`0x331`), DI_speed
  (`0x257`), DI_torque (`0x108`) — i.e. enough for FSD activation,
  HW detection (with override — see PR notes), and most diagnostics.
- **Vehicle CAN** does not carry the AP frames; HW auto-detect won't
  fire on this bus. Use Chassis CAN instead.

Confirmed limitations on Chassis CAN (2026.8.3 EU HW3):
- `0x398` GTW_carConfig is not on this tap → use the dashboard's
  HW Override dropdown to pin HW3 manually
- `0x132` / `0x292` / `0x312` BMS frames are not broadcast on this
  bus → battery dashboard stays empty (the enhauto Commander itself
  reads them via UDS query/response, which our passive listener
  doesn't implement)
- `0x39B` DAS_status is not broadcast (only `0x39D` is, which carries
  different fields)

---

## Recommended setups

### Setup A — Cheapest full-feature (~$6)

Any ESP32 dev board + any MCP2515 CAN module from Aliexpress.

| Component | Price |
|-----------|-------|
| ESP32-C3-SuperMini or ESP32-DevKitC | ~$3-4 |
| MCP2515 CAN module (TJA1050 transceiver) | ~$1.50-3 |
| X179 pigtail cable (4-wire, or DIY from connector) | ~$3-5 |
| **Total** | **~$8-12** |

Wire: X179 CAN-H/CAN-L → MCP2515 module CAN-H/CAN-L. X179 12V → buck
converter → ESP32 VIN. X179 GND → common GND.

Build with `pio run -e esp32-mcp2515`, adjust pin config in
`esp32/.firmware/config.h`.

### Setup B — M5Stack plug & play (~$20)

| Component | Price |
|-----------|-------|
| [M5Stack ATOM Lite](https://shop.m5stack.com/products/atom-lite-esp32-development-kit) | ~$7.50 |
| [ATOMIC CAN Base (CA-IS3050G)](https://shop.m5stack.com/products/atomic-can-base) | ~$5-7 |
| X179 pigtail cable (4-wire) | ~$3-5 |
| **Total** | **~$16-20** |

ATOMIC CAN Base snaps onto the ATOM Lite. Solder X179 CAN-H/CAN-L to
the screw terminals, 12V to VIN, GND to GND. Build: `pio run -e esp32-twai`.

### Setup C — LILYGO T-2CAN dual-bus (~$33)

| Component | Price |
|-----------|-------|
| [LILYGO T-2CAN ESP32-S3 V1.0](https://lilygo.cc/products/t-2can) | ~$24 |
| 2× X179 pigtail cable (4-wire) or one X179 + one OBD-II pigtail | ~$5-10 |
| **Total** | **~$29-34** |

T-2CAN V1.0 carries **two independent CAN controllers** on one PCB —
**not** two MCP2515s as earlier docs claimed. Per LilyGO's official
[pin\_config.h](https://github.com/Xinyuan-LilyGO/T-2CAN/blob/main/libraries/private_library/pin_config.h):

| Bus | Controller | Pins | Driver flag |
|-----|------------|------|-------------|
| **Can A** | MCP2515 over SPI | CS=10, SCK=12, MOSI=11, MISO=13, INT=8, RST=9 (8 MHz crystal) | `CAN_DRIVER_MCP2515` |
| **Can B** | ESP32-S3 built-in TWAI (external transceiver) | TX=7, RX=6 | `CAN_DRIVER_TWAI` |

The `Fd V1.0` variant ships an **MCP2518FD** on the SPI side (CAN-FD,
not compatible with this firmware's `autowp/autowp-mcp2515` driver).
Confirm the silkscreen reads `T-2Can V1.0`, not `T-2Can-Fd V1.0`,
before flashing the `t-2can-v1` env.

Build: `pio run -e t-2can-v1`. This env defines `CAN_DRIVER_DUAL`,
so both drivers are compiled in and both buses are polled every loop
iteration. TX echoes (NAG, TLSSC, 0x3FD modification) target the same
bus the source frame arrived on; self-initiated TX (precondition inject)
defaults to **bus 0 (MCP2515)**.

#### Recommended wiring

| Wire | Target | Why |
|------|--------|-----|
| **Bus 0 (MCP2515)** — Can A screw terminal | Chassis CAN (X179 Pins 13/14, or enhauto Commander Chassis pair) | Has a dedicated `INT` pin for EXT1 wake-on-CAN — the bus most likely to be silent while the car is asleep benefits from the lowest-latency wake source. |
| **Bus 1 (TWAI)** — Can B screw terminal | Party CAN (OBD-II Pins 6/14) or a second X179 tap | The TWAI RX pin (GPIO 6) is also RTC-capable, so the same EXT1 wake mask covers both buses. |

Sharing a single 12V/GND supply for both screw terminals is fine
because the car's buses share a common chassis ground.

#### Deep sleep

T-2CAN uses `SLEEP_STRATEGY_EXT1` — `esp_sleep_enable_ext1_wakeup()`
with both `PIN_CAN_RX` (TWAI) and `PIN_MCP_INT` (MCP2515) in the wake
mask, polarity `ESP_EXT1_WAKEUP_ANY_LOW`. Traffic on **either** bus
ends sleep, no polling required. The MCP2515 keeps its default
interrupt enables (RX0IE+RX1IE) so a received frame on Bus 0 pulls
GPIO 8 LOW and wakes the SoC just like a TWAI RX edge on GPIO 6.

### Setup D — Flipper Zero + CAN Add-On (~$210)

The original reference platform. Connect to **OBD-II** (not X179) using
the Electronic Cats CAN Bus Add-On or a generic MCP2515 module.

| Component | Price |
|-----------|-------|
| [Flipper Zero](https://flipper.net/) | $199 |
| [Electronic Cats CAN Bus Add-On](https://electroniccats.com/store/flipper-addon-canbus/) | $35 |
| OBD-II pigtail cable | ~$5-10 |
| **Total** | **~$239-244** |

OBD-II wiring (Party CAN only):

| OBD-II pin | Wire | Add-On terminal |
|------------|------|-----------------|
| Pin 6 | CAN-H | CAN-H |
| Pin 14 | CAN-L | CAN-L |
| Pin 4/5 | GND | GND |

The Flipper can also be wired to X179 instead of OBD-II for bus 6
access, but the cable run from the rear console to the Flipper is long.

### Termination resistor

Tesla's CAN buses are already terminated. **Do not add a second 120 Ω
terminator.** Most aftermarket CAN modules ship with the termination
resistor enabled — disable it before connecting to the car.

- **Electronic Cats Add-On v0.1**: open the `J1 / TERM` solder jumper
- **Electronic Cats Add-On v0.2+**: ships disabled, no action needed
- **Generic MCP2515 modules**: find and remove `R4` or `J1`
- **M5Stack ATOMIC CAN Base**: no termination by default
- **LILYGO T-2CAN V1.0**: termination state per the published schematic is not confirmed by this PR's author — measure each screw terminal with the board off-car before connecting (see the verify step below).

Verify: measure resistance between CAN-H and CAN-L with the module
disconnected from the car. ~120 Ω = good (terminator off, car provides
its own). ~60 Ω = your module's terminator is on, disable it.

---

## Power and sleep

### OBD-II Pin 16 — always on

OBD-II Pin 16 supplies +12V **even when the car is locked and
sleeping**. If your module draws 50 mA at 12V (typical ESP32 idle),
that's 0.6W continuous → will drain the 12V battery over days.

### X179 Pin 1/15 — behavior varies

On some Model 3/Y builds, X179 12V is gated by the car's wake state.
On others it's always-on like OBD-II. Test with a multimeter before
relying on it.

### Deep sleep (recommended for permanent install)

For any module that stays plugged in:

1. Monitor CAN bus traffic. If no frames seen for 5 minutes → the car
   is asleep.
2. Enter ESP32 deep sleep (~10 µA draw, negligible battery impact).
3. Wake on MCP2515 INT pin (frame received = car woke up) or on a
   timer (check every 60 seconds).

This is how commercial products (Feifan Commander, enhauto Commander)
handle permanent installation without draining the 12V battery.

---

## What about other CAN modules?

Anything with an MCP2515-over-SPI interface or an ESP32 TWAI peripheral
works with a config change. Community-confirmed boards:

- Joy-IT SBC-CAN01 (MCP2515) — Europe source
- Waveshare RS485-CAN-HAT (MCP2515) — re-wire jumpers for Flipper
- Waveshare ESP32-S3-RS485-CAN — TWAI driver, all-in-one
- Adafruit RP2040 / Feather M4 CAN — see upstream
  [Karolynaz/waymo-fsd-can-mod](https://github.com/Karolynaz/waymo-fsd-can-mod)

If you get a non-listed board working, open a PR with the pin map.
