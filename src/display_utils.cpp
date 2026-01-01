#include "display_utils.h"

// Tracking variables for differential updates
static State lastState = (State)-1;
static long lastRssi = -1000;
static IPAddress lastIp;
static bool lastBlinkState = false;

void drawStatusBar() {
  // 1. Determine current values
  bool currentBlink = (millis() / 500) % 2 == 0;
  long currentRssi = (currentState == STATE_CONNECTED) ? WiFi.RSSI() : 0;
  IPAddress currentIp;
  if (currentState == STATE_CONNECTED)
    currentIp = WiFi.localIP();
  else if (currentState == STATE_AP_MODE)
    currentIp = WiFi.softAPIP();

  // 2. Check for changes
  bool stateChanged = (currentState != lastState);
  bool rssiChanged = (currentRssi != lastRssi);
  bool ipChanged = (currentIp != lastIp);
  bool blinkChanged = (currentBlink != lastBlinkState);

  // Icon Position (Right aligned)
  int iconX = 205;
  int iconY = 4;

  // 3. Update Background and Border if state changed
  if (stateChanged) {
    tft.fillRect(0, 0, 240, 24, TFT_DARKGREY);
    tft.drawFastHLine(0, 24, 240, TFT_WHITE);
  }

  // 4. Update Icon area
  if (stateChanged || (currentState == STATE_CONNECTED && rssiChanged) ||
      (currentState == STATE_CONNECTING && blinkChanged)) {

    // Clear only icon area if not already cleared by stateChanged
    if (!stateChanged) {
      tft.fillRect(iconX, iconY, 30, 16, TFT_DARKGREY);
    }

    if (currentState == STATE_AP_MODE) {
      drawAPIcon(iconX, iconY);
    } else if (currentState == STATE_CONNECTING) {
      if (currentBlink)
        drawWifiIcon(iconX, iconY, -50);
    } else if (currentState == STATE_CONNECTED) {
      drawWifiIcon(iconX, iconY, currentRssi);
    } else {
      tft.drawLine(iconX, iconY, iconX + 15, iconY + 15, TFT_RED);
      tft.drawLine(iconX + 15, iconY, iconX, iconY + 15, TFT_RED);
    }
  }

  // 5. Update IP Address area
  if (stateChanged || ipChanged) {
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_DARKGREY);

    if (currentIp[0] == 0) {
      tft.fillRect(5, 4, 100, 20, TFT_DARKGREY);
    } else {
      tft.setCursor(5, 4);
      tft.printf("%d.%d  ", currentIp[0], currentIp[1]);

      tft.setCursor(5, 14);
      tft.printf("%d.%d  ", currentIp[2], currentIp[3]);
    }
  }

  // 6. Store current state
  lastState = currentState;
  lastRssi = currentRssi;
  lastIp = currentIp;
  lastBlinkState = currentBlink;
}

void drawAPIcon(int x, int y) {
  tft.fillRoundRect(x, y, 24, 16, 3, TFT_ORANGE);
  tft.setTextColor(TFT_BLACK, TFT_ORANGE);
  tft.setTextSize(1);
  tft.setCursor(x + 6, y + 4);
  tft.print("AP");
}

void drawWifiIcon(int x, int y, long rssi) {
  int bars = 0;
  if (rssi >= -60)
    bars = 4;
  else if (rssi >= -70)
    bars = 3;
  else if (rssi >= -80)
    bars = 2;
  else if (rssi >= -90)
    bars = 1;

  for (int i = 0; i < 4; i++) {
    int h = (i + 1) * 4;
    uint16_t color = (i < bars) ? TFT_GREEN : TFT_BLACK;
    tft.fillRect(x + (i * 4), y + (16 - h), 3, h, color);
  }
}
