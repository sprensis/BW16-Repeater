/*                                      _     
        ___ _ __  _ __ ___ _ __  ___(_)___ 
        / __| '_ \| '__/ _ \ '_ \/ __| / __| 
        \__ \ |_) | | |  __/ | | \__ \ \__ \ 
        |___/ .__/|_|  \___|_| |_|___/_|___/ 
            |_|                             
                © Copyright 2025 
            ✈ `https://github.com/sprensis` 
    Name: DhcpServer 
    Description: DHCP control for AP clients 
    Author: @sprensis 
    Platform: BW16 (RTL8720dn) - Ameba Arduino 
    License: MIT 
*/

#pragma once
#include <Arduino.h>
#include <WiFiUdp.h>
#include "../utils/ConfigStore.h"
#include "../logger/Logger.h"

class DhcpServer {
 public:
  void begin(ConfigStore& cfg, Logger& log);
  void loop();
 private:
  void handleDns();
  WiFiUDP dns;
  ConfigStore* store=nullptr;
  Logger* logger=nullptr;
};