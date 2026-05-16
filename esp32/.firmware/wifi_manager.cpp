#include "wifi_manager.h"
#include <WiFi.h>
#include <Arduino.h>
#include <esp_wifi.h>

bool wifi_ap_init(const FSDState *state) {
    WiFi.mode(WIFI_AP);
    // Disable modem-sleep before bringing up the soft-AP. With the Arduino-
    // ESP32 default (WIFI_PS_MIN_MODEM), the radio sleeps between beacons,
    // which on ESP32-S3 boards has been observed to leave the SSID invisible
    // to clients (iPhone in particular) until a station happens to wake the
    // AP with a probe. PS_NONE keeps the radio on, costing ~30 mA extra but
    // making AP visibility instant and reliable.
    WiFi.setSleep(WIFI_PS_NONE);
    // softAP(ssid, password, channel, hidden, max_connection)
    bool ok = WiFi.softAP(state->wifi_ssid, state->wifi_pass, 1, state->wifi_hidden);
    if (ok) {
        // Bump TX power to maximum after the interface is up. Some IDF
        // builds bring up the AP with a conservative default (~7 dBm); the
        // higher setting ensures a usable signal from inside a Tesla's
        // shielded interior trim.
        WiFi.setTxPower(WIFI_POWER_19_5dBm);
        Serial.printf("[WiFi] AP: \"%s\"%s IP: %s\n",
            state->wifi_ssid,
            state->wifi_hidden ? " (HIDDEN)" : "",
            WiFi.softAPIP().toString().c_str());
        Serial.println("[WiFi] Dashboard: http://192.168.4.1");
    } else {
        Serial.println("[WiFi] AP start FAILED — continuing without web");
    }
    return ok;
}
