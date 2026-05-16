/*
 * can_driver.cpp
 *
 * CAN driver implementations:
 *   CAN_DRIVER_TWAI    — ESP32 built-in TWAI peripheral (singleton; one bus only)
 *   CAN_DRIVER_MCP2515 — SPI-attached MCP2515. Multiple instances supported in
 *                        principle, though each needs its own CS/INT lines.
 *   CAN_DRIVER_DUAL    — Build both. Used by t-2can-v1 to drive TWAI + MCP2515
 *                        simultaneously.
 *
 * Each concrete driver implements the CanDriver interface from can_driver.h.
 */

#include "can_driver.h"
#include "config.h"
#include <Arduino.h>
#include <string.h>

#if defined(CAN_DRIVER_TWAI) || defined(CAN_DRIVER_DUAL)
  #include "driver/twai.h"
#endif

#if defined(CAN_DRIVER_MCP2515) || defined(CAN_DRIVER_DUAL)
  #include <SPI.h>
  #include <mcp2515.h>   // autowp/autowp-mcp2515
#endif

// ── TWAI driver ───────────────────────────────────────────────────────────────
// The ESP32 TWAI peripheral is a singleton — only one TwaiDriver instance can
// be active at a time across the whole binary.
#if defined(CAN_DRIVER_TWAI) || defined(CAN_DRIVER_DUAL)

class TwaiDriver : public CanDriver {
    TwaiPins pins_;
    bool     listen_only_ = false;
    bool     installed_   = false;

    bool install_and_start(bool listen_only) {
        twai_general_config_t g = TWAI_GENERAL_CONFIG_DEFAULT(
            (gpio_num_t)pins_.tx_pin,
            (gpio_num_t)pins_.rx_pin,
            listen_only ? TWAI_MODE_LISTEN_ONLY : TWAI_MODE_NORMAL);
        // Queue depths: 10 RX, 5 TX — sufficient for polling loop
        g.rx_queue_len = 10;
        g.tx_queue_len = 5;

        twai_timing_config_t t = TWAI_TIMING_CONFIG_500KBITS();
        twai_filter_config_t f = TWAI_FILTER_CONFIG_ACCEPT_ALL();

        if (twai_driver_install(&g, &t, &f) != ESP_OK) return false;
        if (twai_start() != ESP_OK) {
            twai_driver_uninstall();
            return false;
        }
        installed_    = true;
        listen_only_  = listen_only;
        return true;
    }

    void stop_and_uninstall() {
        if (!installed_) return;
        twai_stop();
        twai_driver_uninstall();
        installed_ = false;
    }

public:
    explicit TwaiDriver(const TwaiPins &pins) : pins_(pins) {}

    bool begin(bool listen_only) override {
        return install_and_start(listen_only);
    }

    bool send(const CanFrame &frame) override {
        if (listen_only_) return false;
        twai_message_t msg;
        memset(&msg, 0, sizeof(msg));
        msg.identifier       = frame.id;
        msg.data_length_code = frame.dlc;
        memcpy(msg.data, frame.data, frame.dlc);
        // 5 ms TX timeout — short enough to not stall the main loop
        return twai_transmit(&msg, pdMS_TO_TICKS(5)) == ESP_OK;
    }

    bool receive(CanFrame &frame) override {
        twai_message_t msg;
        // Non-blocking receive (timeout = 0)
        if (twai_receive(&msg, 0) != ESP_OK) return false;
        frame.id  = msg.identifier;
        frame.dlc = msg.data_length_code;
        memcpy(frame.data, msg.data, frame.dlc);
        return true;
    }

    uint32_t errorCount() override {
        twai_status_info_t info;
        if (twai_get_status_info(&info) != ESP_OK) return 0;
        return info.rx_missed_count + info.bus_error_count;
    }

    void setListenOnly(bool enable) override {
        if (listen_only_ == enable) return;
        stop_and_uninstall();
        install_and_start(enable);
    }

    const char *name() const override { return "TWAI"; }
};

CanDriver *can_driver_create_twai(const TwaiPins &pins) {
    return new TwaiDriver(pins);
}

#endif  // CAN_DRIVER_TWAI || CAN_DRIVER_DUAL

// ── MCP2515 driver ────────────────────────────────────────────────────────────
#if defined(CAN_DRIVER_MCP2515) || defined(CAN_DRIVER_DUAL)

class Mcp2515Driver : public CanDriver {
    MCP2515     mcp_;
    Mcp2515Pins pins_;
    bool        listen_only_  = false;
    uint32_t    err_count_    = 0;

public:
    explicit Mcp2515Driver(const Mcp2515Pins &pins)
        : mcp_(pins.cs_pin), pins_(pins) {}

    bool begin(bool listen_only) override {
        // SPI.begin() is idempotent across instances on the same bus; multiple
        // MCP2515 chips can share SCK/MOSI/MISO with their own CS lines. The
        // last begin() call wins for SPI pins — by convention all instances
        // should pass the same bus pins.
        SPI.begin(pins_.sck_pin, pins_.miso_pin, pins_.mosi_pin, pins_.cs_pin);
        SPI.setFrequency(8000000);

        mcp_.reset();
        CAN_CLOCK crystal;
        switch (pins_.crystal_mhz) {
            case 8:  crystal = MCP_8MHZ;  break;
            case 16: crystal = MCP_16MHZ; break;
            case 20: crystal = MCP_20MHZ; break;
            default: crystal = MCP_8MHZ;  break;  // safe fallback for common Chinese modules
        }
        if (mcp_.setBitrate(CAN_500KBPS, crystal) != MCP2515::ERROR_OK)
            return false;

        MCP2515::ERROR err = listen_only
            ? mcp_.setListenOnlyMode()
            : mcp_.setNormalMode();
        listen_only_ = listen_only;
        return err == MCP2515::ERROR_OK;
    }

    bool send(const CanFrame &frame) override {
        if (listen_only_) return false;
        struct can_frame f;
        f.can_id  = frame.id;
        f.can_dlc = frame.dlc;
        memcpy(f.data, frame.data, frame.dlc);
        if (mcp_.sendMessage(&f) != MCP2515::ERROR_OK) {
            err_count_++;
            return false;
        }
        return true;
    }

    bool receive(CanFrame &frame) override {
        struct can_frame f;
        if (mcp_.readMessage(&f) != MCP2515::ERROR_OK) return false;
        frame.id  = f.can_id;
        frame.dlc = f.can_dlc;
        memcpy(frame.data, f.data, f.can_dlc);
        return true;
    }

    uint32_t errorCount() override {
        return err_count_;
    }

    void setListenOnly(bool enable) override {
        if (listen_only_ == enable) return;
        listen_only_ = enable;
        if (enable)
            mcp_.setListenOnlyMode();
        else
            mcp_.setNormalMode();
    }

    const char *name() const override {
        return pins_.label ? pins_.label : "MCP2515";
    }
};

CanDriver *can_driver_create_mcp2515(const Mcp2515Pins &pins) {
    return new Mcp2515Driver(pins);
}

#endif  // CAN_DRIVER_MCP2515 || CAN_DRIVER_DUAL

// ── Default single-bus factory ────────────────────────────────────────────────
// Single-bus envs continue to call can_driver_create() and get the driver
// chosen at compile time, with pins drawn from config.h macros.
#if defined(CAN_DRIVER_TWAI) && !defined(CAN_DRIVER_DUAL)

CanDriver *can_driver_create() {
    TwaiPins p{ PIN_CAN_TX, PIN_CAN_RX };
    return can_driver_create_twai(p);
}

#elif defined(CAN_DRIVER_MCP2515) && !defined(CAN_DRIVER_DUAL)

CanDriver *can_driver_create() {
    Mcp2515Pins p{
        /*cs*/    PIN_MCP_CS,
        /*sck*/   PIN_MCP_SCK,
        /*mosi*/  PIN_MCP_MOSI,
        /*miso*/  PIN_MCP_MISO,
        /*int*/   -1,
        /*MHz*/   MCP_CRYSTAL_HZ_DEFAULT,
        /*label*/ "MCP2515",
    };
    return can_driver_create_mcp2515(p);
}

#elif defined(CAN_DRIVER_DUAL)
// Dual-bus envs don't use the default factory; main.cpp instantiates each
// driver explicitly with its own pin config. Provide a stub so any stray
// caller fails loudly at link time rather than producing nullptr surprises.
CanDriver *can_driver_create() { return nullptr; }

#else
#error "Define CAN_DRIVER_TWAI, CAN_DRIVER_MCP2515, or CAN_DRIVER_DUAL in platformio.ini build_flags"
#endif
