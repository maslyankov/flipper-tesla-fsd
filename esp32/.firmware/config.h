#pragma once

// ── CAN IDs ───────────────────────────────────────────────────────────────────
#define CAN_ID_STW_ACTN_RQ    0x045u  // 69   - STW_ACTN_RQ:  steering stalk (Legacy follow distance)
#define CAN_ID_TRIP_PLANNING  0x082u  // 130  - UI_tripPlanning: precondition trigger
#define CAN_ID_BMS_HV_BUS     0x132u  // 306  - BMS_hvBusStatus: pack voltage / current
#define CAN_ID_BMS_SOC        0x292u  // 658  - BMS_socStatus:   state of charge
#define CAN_ID_BMS_THERMAL    0x312u  // 786  - BMS_thermalStatus: battery temp
#define CAN_ID_GTW_CAR_STATE  0x318u  // 792  - GTW_carState:    OTA detection
#define CAN_ID_EPAS_STATUS    0x370u  // 880  - EPAS3P_sysStatus: nag killer target
#define CAN_ID_GTW_CAR_CONFIG 0x398u  // 920  - GTW_carConfig:   HW version detection
#define CAN_ID_ISA_SPEED      0x399u  // 921  - ISA speed limit:  HW4 chime suppress
#define CAN_ID_AP_LEGACY      0x3EEu  // 1006 - DAS_autopilot:   Legacy / HW1 / HW2
#define CAN_ID_FOLLOW_DIST    0x3F8u  // 1016 - DAS_followDistance: speed profile source
#define CAN_ID_DAS_AP_CONFIG  0x331u  // 817  - DAS autopilot config (tier restore target, ~1 Hz)
#define CAN_ID_AP_CONTROL     0x3FDu  // 1021 - DAS_autopilotControl: HW3 / HW4 core
#define CAN_ID_DAS_STATUS     0x39Bu  // 923  - DAS_status: AP hands-on state (nag gating)
#define CAN_ID_DI_SPEED       0x257u  // 599  - DI_speed: vehicle speed
#define CAN_ID_ESP_STATUS     0x145u  // 325  - ESP_status: brake apply, stability (Party CAN)
#define CAN_ID_BATT_STATUS    0x420u  // 1056 - Empirically (2026.8.3 EU HW3 on Chassis CAN):
                                      //         byte 2 = SoC %  (verified at 64%)
                                      //         Identified on Chassis CAN where the standard
                                      //         0x132/0x292/0x312 BMS frames aren't broadcast.
                                      //         byte 0 and 3 hold stable values whose meaning
                                      //         is unconfirmed.
#define CAN_ID_BATT_TEMP      0x239u  // 569  - Empirically (Chassis CAN, low-rate, ~1 Hz):
                                      //         byte 5 × 0.5 − 40 = battery temperature °C
                                      //         (verified at 22.5°C reported vs cluster 21.8°C —
                                      //         matches standard Tesla temp encoding).
#define CAN_ID_DC_BUS         0x2B5u  // 693  - Empirically (2026.8.3 EU HW3 on Chassis CAN):
                                      //         bytes 0-1 LE × 0.01 = LV (12V) bus voltage
                                      //         bytes 2-3 LE × 0.1  = HV pack voltage
                                      //         byte 4    × 0.1     = LV bus current (A)
                                      //         Verified against enhauto Commander readouts.
#define CAN_ID_DI_TORQUE      0x108u  // 264  - DI_torque: drive motor torque
#define CAN_ID_STEER_ANGLE    0x129u  // 297  - SCCM_steeringAngleSensor

// ── GPIO ──────────────────────────────────────────────────────────────────────
#if defined(BOARD_LILYGO)
  #define PIN_CAN_TX         27
  #define PIN_CAN_RX         26
  #define PIN_CAN_SPEED_MODE 23   // SN65HVD230 Rs — must be LOW for TX+RX
  #define PIN_LED            4    // SK6812 NeoPixel
  #define PIN_BUTTON         0    // Boot button, active-LOW, internal pull-up
  #define SD_MISO            2
  #define SD_MOSI            15
  #define SD_SCLK            14
  #define SD_CS              13
#else
  #ifndef PIN_CAN_TX
  #define PIN_CAN_TX   22   // TWAI TX → ATOMIC CAN Base TX
  #endif
  #ifndef PIN_CAN_RX
  #define PIN_CAN_RX   19   // TWAI RX ← ATOMIC CAN Base RX
  #endif
  #ifndef PIN_LED
  #define PIN_LED      27   // SK6812 NeoPixel (single LED)
  #endif
#endif
#ifndef PIN_BUTTON
#define PIN_BUTTON    0   // Built-in button (GPIO0), active-LOW, internal pull-up
#endif

// MCP2515 SPI — only used in CAN_DRIVER_MCP2515 build (generic ESP32)
// Standard VSPI pins: SCK=18, MISO=19, MOSI=23, CS=5
#define PIN_MCP_CS   5
#define PIN_MCP_SCK  18
#define PIN_MCP_MISO 19
#define PIN_MCP_MOSI 23

// MCP2515 oscillator (megahertz). The Mcp2515Pins struct translates this to
// the autowp CAN_CLOCK enum internally so callers don't need to include the
// driver header. Common Chinese MCP2515 modules and the T-2CAN V1.0 use 8 MHz.
#define MCP_CRYSTAL_HZ_DEFAULT  8u

// ── Timing ────────────────────────────────────────────────────────────────────
#define WIRING_WARN_MS        5000u   // Red LED / serial warning if no CAN after this
#define PRECOND_INTERVAL_MS    500u   // Re-inject 0x082 precondition every N ms
#define BMS_PRINT_MS          1000u   // BMS serial print interval
#define BUTTON_DEBOUNCE_MS        50u
#define LONG_PRESS_MS           3000u   // Long press → toggle NAG killer
#define DOUBLE_CLICK_MS          400u   // Max gap between two clicks for double-click
#define FACTORY_RESET_HOLD_MS   5000u   // Hold duration to arm factory reset
#define FACTORY_RESET_WINDOW_MS 20000u  // Clean-boot window during which reset is possible
#define STATUS_PRINT_MS       5000u   // Periodic status line when Active

// OTA detection hardening on GTW_carState (0x318)
// Some firmware versions keep non-zero states when no update is actively running.
// We only treat one specific raw value as "update in progress" and require
// consecutive-frame confirmation to avoid false positives.
#define OTA_IN_PROGRESS_RAW_VALUE  1u
#define OTA_ASSERT_FRAMES          3u
#define OTA_CLEAR_FRAMES           6u

#if defined(BOARD_LILYGO)
  #define ME2107_EN 16
#endif

// ── T-2CAN V1.0 (ESP32-S3, dual-bus) ──────────────────────────────────────────
// One MCP2515 over SPI + one ESP32-S3 TWAI peripheral with external
// transceiver. Per LilyGO pin_config.h.
#if defined(BOARD_LILYGO_T2CAN)
  // TWAI ("Can B") pins
  #undef  PIN_CAN_TX
  #undef  PIN_CAN_RX
  #define PIN_CAN_TX   7
  #define PIN_CAN_RX   6
  // MCP2515 ("Can A") pins — override the generic defaults above
  #undef  PIN_MCP_CS
  #undef  PIN_MCP_SCK
  #undef  PIN_MCP_MISO
  #undef  PIN_MCP_MOSI
  #define PIN_MCP_CS    10
  #define PIN_MCP_SCK   12
  #define PIN_MCP_MOSI  11
  #define PIN_MCP_MISO  13
  #define PIN_MCP_INT    8   // RTC-capable on ESP32-S3 → EXT1 wake target
  #define PIN_MCP_RST    9
  // T-2CAN V1.0 has no user-controllable RGB LED. The generic default
  // PIN_LED=27 collides with the internal QIO flash interface on
  // ESP32-S3-WROOM-1 (GPIO 26-32 are reserved for flash), so driving it as
  // a NeoPixel during led_init() corrupts flash access and triggers a
  // TG1WDT_SYS_RST reset loop. Point at a free user GPIO instead — the
  // NeoPixel bit-stream goes nowhere and the firmware boots cleanly.
  #undef  PIN_LED
  #define PIN_LED       21
#endif

// Bus count — 2 for dual builds, 1 otherwise. Drives g_can[] array sizing
// and per-bus state in the dashboard.
#if defined(CAN_DRIVER_DUAL)
  #define CAN_BUS_COUNT 2
#else
  #define CAN_BUS_COUNT 1
#endif

// ── Deep sleep ────────────────────────────────────────────────────────────────
// Two strategies, selected per board:
//   SLEEP_STRATEGY_EXT0  — wake on CAN_RX edge. Needs PIN_CAN_RX on an
//                          RTC-capable GPIO. LilyGO's GPIO 26 qualifies; the
//                          SN65HVD230 Rs pin has an internal 100 kΩ pull-down
//                          so the transceiver stays in normal-receive mode
//                          while GPIO 23 floats during deep sleep.
//   SLEEP_STRATEGY_TIMER — wake periodically and listen briefly for CAN
//                          traffic. Works on any board. Used by M5Stack ATOM
//                          variants whose ATOMIC CAN Base routes RX to GPIO 19
//                          (not RTC-capable, so EXT0 is unavailable).
//   SLEEP_STRATEGY_EXT1  — wake on ANY of several RTC pins (ESP32-S3 only).
//                          T-2CAN uses this to wake on either the TWAI RX
//                          edge or the MCP2515 INT line, so traffic on
//                          either bus brings the chip out of deep sleep
//                          with zero polling overhead.
// Both ATOM Lite and ATOM Matrix share the ATOMIC CAN Base wiring (RX on
// GPIO 19, not RTC-capable), so neither can use EXT0-on-CAN_RX. Both opt
// into the timer-poll strategy. To disable sleep on a permanently-powered
// dev setup, raise sleep_idle_ms via the web dashboard (max 3600 s).
#if defined(BOARD_LILYGO)
  #define SLEEP_STRATEGY_EXT0 1
#elif defined(BOARD_LILYGO_T2CAN)
  #define SLEEP_STRATEGY_EXT1 1
#elif defined(BOARD_M5STACK_ATOM) || defined(BOARD_M5STACK_ATOM_MATRIX)
  #define SLEEP_STRATEGY_TIMER 1
#endif

#define SLEEP_IDLE_MS          60000u   // CAN silence before entering deep sleep (runtime override via web UI)
#define SLEEP_WARN_MS           5000u   // serial/log warning this many ms before sleep
#define SLEEP_TIMER_WAKE_S        60u   // TIMER strategy: deep-sleep duration between probes
#define SLEEP_PROBE_MS          5000u   // TIMER strategy: listen window for CAN after a timer wake
