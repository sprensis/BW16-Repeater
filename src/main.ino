/*                                      _     
        ___ _ __  _ __ ___ _ __  ___(_)___ 
        / __| '_ \| '__/ _ \ '_ \/ __| / __| 
        \__ \ |_) | | |  __/ | | \__ \ \__ \ 
        |___/ .__/|_|  \___|_| |_|___/_|___/ 
            |_|                             
                © Copyright 2025 
            ✈ `https://github.com/sprensis` 
    Name: BW16 WiFi Repeater 
    Description: Full-featured WiFi repeater for BW16 (RTL8720DN) 
    Author: @sprensis 
    Platform: BW16 (RTL8720dn) - Ameba Arduino 
    License: MIT 
*/

#include "utils/ConfigStore.h"
#include "logger/Logger.h"
#include "wifi_manager/WifiManager.h"
#include "nat_manager/NatManager.h"
#include "dhcp_server/DhcpServer.h"
#include "ble_config/BleConfig.h"
#include "webui/WebUI.h"
#include "utils/Watchdog.h"

ConfigStore configStore;
Logger logger;
WifiManager wifi;
NatManager nat;
DhcpServer dhcp;
BleConfig ble;
WebUI webui;
Watchdog wdt;

void setup() {
  Serial.begin(115200);
  logger.begin();
  configStore.begin();
  configStore.load();
  wdt.begin();
  ble.begin(configStore, logger);
  wifi.begin(configStore, logger);
  dhcp.begin(configStore, logger);
  webui.begin(configStore, logger, wifi, nat, dhcp);
  if (configStore.mode() == RepeaterMode::NAT) {
    nat.beginNat(configStore, logger, wifi);
  } else {
    nat.beginBridge(configStore, logger, wifi);
  }
}

void loop() {
  ble.loop();
  webui.loop();
  wifi.loop();
  dhcp.loop();
  nat.loop();
  wdt.feed();
}