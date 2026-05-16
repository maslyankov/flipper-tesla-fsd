#pragma once

#include "fsd_handler.h"  // for CanFrame
#include <stdint.h>

// ── Abstract CAN driver ───────────────────────────────────────────────────────
// Implemented by TwaiDriver and Mcp2515Driver. Single- and dual-bus builds use
// the same interface; the dual-bus T-2CAN env (CAN_DRIVER_DUAL) compiles both
// concrete drivers and instantiates one of each. Single-bus envs pick one
// driver via CAN_DRIVER_TWAI or CAN_DRIVER_MCP2515.

class CanDriver {
public:
    /** Initialise hardware and start the CAN bus.
     *  @param listen_only  If true, enter hardware listen-only mode (no ACK, no TX). */
    virtual bool begin(bool listen_only) = 0;

    /** Send one CAN frame.  Returns false when TX is not allowed (listen-only, bus-off, etc.). */
    virtual bool send(const CanFrame &frame) = 0;

    /** Non-blocking receive.  Fills frame and returns true if a frame was available. */
    virtual bool receive(CanFrame &frame) = 0;

    /** Cumulative bus-error counter (rx_missed + bus_errors). */
    virtual uint32_t errorCount() = 0;

    /** Switch between listen-only and normal TX mode at runtime.
     *  Implementations must reinitialise the hardware as needed. */
    virtual void setListenOnly(bool enable) = 0;

    /** Short human-readable label ("TWAI", "MCP2515 #1", ...). Useful for logs. */
    virtual const char *name() const = 0;

    virtual ~CanDriver() = default;
};

// ── Pin/config structs ────────────────────────────────────────────────────────
struct TwaiPins {
    int tx_pin;
    int rx_pin;
};

struct Mcp2515Pins {
    int cs_pin;
    int sck_pin;
    int mosi_pin;
    int miso_pin;
    int int_pin;          // optional: −1 disables interrupt-based wake
    uint8_t crystal_mhz;  // 8 / 16 / 20 — translated to autowp CAN_CLOCK inside the driver
    const char *label;    // for diagnostics; nullptr → "MCP2515"
};

// ── Concrete factories ────────────────────────────────────────────────────────
// Implemented in can_driver.cpp behind the matching compile-time flag.
// Callers may create one or more instances; the caller owns each pointer.

#if defined(CAN_DRIVER_TWAI) || defined(CAN_DRIVER_DUAL)
CanDriver *can_driver_create_twai(const TwaiPins &pins);
#endif

#if defined(CAN_DRIVER_MCP2515) || defined(CAN_DRIVER_DUAL)
CanDriver *can_driver_create_mcp2515(const Mcp2515Pins &pins);
#endif

/** Default factory — single-bus convenience using compile-time pins.
 *  Returns the driver selected by CAN_DRIVER_TWAI or CAN_DRIVER_MCP2515. */
CanDriver *can_driver_create();
