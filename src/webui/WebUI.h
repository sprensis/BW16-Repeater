/*                                      _     
        ___ _ __  _ __ ___ _ __  ___(_)___ 
        / __| '_ \| '__/ _ \ '_ \/ __| / __| 
        \__ \ |_) | | |  __/ | | \__ \ \__ \ 
        |___/ .__/|_|  \___|_| |_|___/_|___/ 
            |_|                             
                © Copyright 2025 
            ✈ `https://github.com/sprensis` 
    Name: WebUI 
    Description: Embedded web UI with auth 
    Author: @sprensis 
    Platform: BW16 (RTL8720dn) - Ameba Arduino 
    License: MIT 
*/

#pragma once
#include <Arduino.h>
#include <WiFiServer.h>
#include <WiFiClient.h>
#include "../utils/ConfigStore.h"
#include "../logger/Logger.h"
#include "../wifi_manager/WifiManager.h"
#include "../nat_manager/NatManager.h"
#include "../dhcp_server/DhcpServer.h"
#include "../utils/Base64.h"
#include "../utils/Utils.h"

class WebUI {
 public:
  void begin(ConfigStore& cfg, Logger& log, WifiManager& wifi, NatManager& nat, DhcpServer& dhcp);
  void loop();
 private:
  void handleClient(WiFiClient& c);
  bool authOk(const String& h);
  WiFiServer server{80};
  ConfigStore* store;
  Logger* logger;
  WifiManager* wifiRef;
  NatManager* natRef;
  DhcpServer* dhcpRef;
};