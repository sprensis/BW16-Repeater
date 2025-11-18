/*                                      _     
        ___ _ __  _ __ ___ _ __  ___(_)___ 
        / __| '_ \| '__/ _ \ '_ \/ __| / __| 
        \__ \ |_) | | |  __/ | | \__ \ \__ \ 
        |___/ .__/|_|  \___|_| |_|___/_|___/ 
            |_|                             
                © Copyright 2025 
            ✈ `https://github.com/sprensis` 
    Name: BleConfig 
    Description: BLE GATT-based configurator 
    Author: @sprensis 
    Platform: BW16 (RTL8720dn) - Ameba Arduino 
    License: MIT 
*/

#pragma once
#include <Arduino.h>
#include "../utils/ConfigStore.h"
#include "../logger/Logger.h"

class BleConfig {
 public:
  void begin(ConfigStore& cfg, Logger& log);
  void loop();
 private:
  ConfigStore* store;
  Logger* logger;
};