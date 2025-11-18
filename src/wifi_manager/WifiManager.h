/*                                      _     
        ___ _ __  _ __ ___ _ __  ___(_)___ 
        / __| '_ \| '__/ _ \ '_ \/ __| / __| 
        \__ \ |_) | | |  __/ | | \__ \ \__ \ 
        |___/ .__/|_|  \___|_| |_|___/_|___/ 
            |_|                             
                © Copyright 2025 
            ✈ `https://github.com/sprensis` 
    Name: WifiManager 
    Description: STA+AP control with auto-reconnect and band preference 
    Author: @sprensis 
    Platform: BW16 (RTL8720dn) - Ameba Arduino 
    License: MIT 
*/

#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include "../utils/ConfigStore.h"
#include "../logger/Logger.h"

class WifiManager {
 public:
  void begin(ConfigStore& cfg, Logger& log);
  void loop();
  bool staConnected() const;
  IPAddress staIP() const;
  IPAddress apIP() const;
  void restartAP();
  void restartSTA();
  String lastStatusMsg() const { return lastMsg; }
 private:
  void startSTA(const RepeaterConfig& c);
  void startAP(const RepeaterConfig& c);
  void ensureReconnect();
  Logger* logger;
  unsigned long lastCheck=0;
  bool triedFallback=false;
  unsigned long lastFallbackMs=0;
  unsigned long lastStaTryMs=0;
  uint8_t fallbackIndex=0;
  ConfigStore* cfgRef=nullptr;
  String lastMsg;
};