#if defined(DEVICE_TEMBEDS3) || defined(DEVICE_TEMBEDS3CC1101)

#include "TembedWifiSetup.h"
#include <WiFi.h>
#include <Preferences.h>
#include <TFT_eSPI.h>

TFT_eSPI tft;

Preferences& getPreferences() {
    static Preferences preferences;
    return preferences;
}

bool loadWifiCredentials(String& ssid, String& password) {
    Preferences& preferences = getPreferences();
    preferences.begin("wifi_settings", true);
    ssid = preferences.getString(NVS_SSID_KEY, "");
    password = preferences.getString(NVS_PASS_KEY, "");
    preferences.end();
    return !ssid.isEmpty() && !password.isEmpty();
}

void drawWifiBox(uint16_t borderColor, const String& msg1, const String& msg2, uint8_t textSize = 2) {
    tft.fillScreen(TFT_BLACK);

    tft.fillRoundRect(20, 20, tft.width() - 40, tft.height() - 40, 5, DARK_GREY);
    tft.drawRoundRect(20, 20, tft.width() - 40, tft.height() - 40, 5, borderColor);

    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(textSize);
    tft.drawString(msg1, 37, 45);

    tft.setTextFont(1);
    tft.setTextSize(1);
    tft.setTextColor(TFT_LIGHTGREY);
    tft.drawString(msg2, 36, 70);
}

bool setupTembedWifi() {
    tft.setRotation(3);
    String ssid, password;
    drawWifiBox(TFT_GREEN, "Loading WiFi...", "");

    if (!loadWifiCredentials(ssid, password)) {
        drawWifiBox(TFT_GREEN, "No saved WiFi", "Setup your Wi-Fi with USB Serial");
        delay(5000);
        return false;
    }

    drawWifiBox(TFT_GREEN, "Connecting", ssid);

    WiFi.begin(ssid.c_str(), password.c_str());

    tft.setTextColor(TFT_LIGHTGREY);
    tft.setCursor(55, 100);
    for (int i = 0; i < 30; ++i) {
        if (WiFi.status() == WL_CONNECTED) {
            return true;
        }
        delay(500);
        tft.print(".");
    }

    drawWifiBox(TFT_GREEN, "Failed to connect", "Setup your Wi-Fi with USB Serial");
    delay(4000);
    return false;
}

#endif
