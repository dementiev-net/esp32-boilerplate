#pragma once

#include <Arduino.h>

void wifiInit();
void wifiLoop();
void wifiReset();

bool wifiIsConnected();
String wifiGetIP();
String wifiGetSSID();