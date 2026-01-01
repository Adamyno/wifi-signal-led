#ifndef DISPLAY_UTILS_H
#define DISPLAY_UTILS_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <TFT_eSPI.h>

// --- Shared State ---
enum State { STATE_AP_MODE, STATE_CONNECTING, STATE_CONNECTED };

// --- External Globals ---
extern TFT_eSPI tft;
extern State currentState;

// --- Display Functions ---
void drawStatusBar();
void drawWifiIcon(int x, int y, long rssi);
void drawAPIcon(int x, int y);

#endif
