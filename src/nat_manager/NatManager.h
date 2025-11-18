/*                                      _     
        ___ _ __  _ __ ___ _ __  ___(_)___ 
        / __| '_ \| '__/ _ \ '_ \/ __| / __| 
        \__ \ |_) | | |  __/ | | \__ \ \__ \ 
        |___/ .__/|_|  \___|_| |_|___/_|___/ 
            |_|                             
                © Copyright 2025 
            ✈ `https://github.com/sprensis` 
    Name: NatManager 
    Description: NAT and pseudo-bridge management 
    Author: @sprensis 
    Platform: BW16 (RTL8720dn) - Ameba Arduino 
    License: MIT 
*/

#pragma once
#include <Arduino.h>
#include "../utils/ConfigStore.h"
#include "../logger/Logger.h"
#include "../wifi_manager/WifiManager.h"

class NatManager {
 public:
  void beginNat(ConfigStore& cfg, Logger& log, WifiManager& wifi);
  void beginBridge(ConfigStore& cfg, Logger& log, WifiManager& wifi);
  void loop();
 private:
 void runNatTick();
 void runBridgeTick();
 Logger* logger;
 WifiManager* wifiRef;
 bool natMode=false;
  unsigned long lastProbe=0;
};