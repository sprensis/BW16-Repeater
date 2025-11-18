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
  if (Serial.available()) {
    static String ln;
    while (Serial.available()) { char ch=Serial.read(); if (ch=='\n' || ch=='\r') break; ln+=ch; }
    if (ln.length()) {
      if (ln.startsWith("set_sta ")) {
        int sp=ln.indexOf(' ',8); String ssid=ln.substring(8, sp>0?sp:ln.length()); String pass=""; if (sp>0) pass=ln.substring(sp+1);
        strncpy(configStore.mutableRef().staSsid, ssid.c_str(), 63);
        strncpy(configStore.mutableRef().staPass, pass.c_str(), 63);
        configStore.save(); wifi.restartSTA();
        Serial.println("OK");
      } else if (ln.startsWith("set_ap ")) {
        int p1=ln.indexOf(' ',7); int p2=p1>0?ln.indexOf(' ',p1+1):-1; int p3=p2>0?ln.indexOf(' ',p2+1):-1;
        String ssid=p1>0?ln.substring(7,p1):ln.substring(7);
        String pass=p2>0?ln.substring(p1+1,p2):"";
        int ch=p3>0?ln.substring(p2+1,p3).toInt():(p2>0?ln.substring(p2+1).toInt():configStore.get().apChannel);
        strncpy(configStore.mutableRef().apSsid, ssid.c_str(), 63);
        strncpy(configStore.mutableRef().apPass, pass.c_str(), 63);
        configStore.mutableRef().apChannel=(uint8_t)ch;
        configStore.save(); wifi.restartAP();
        Serial.println("OK");
      } else if (ln=="show") {
        Serial.println(String("STA ")+String(configStore.get().staSsid));
        Serial.println(String("AP ")+String(configStore.get().apSsid)+String(" ch ")+String((int)configStore.get().apChannel));
        Serial.println(String("MODE ")+(configStore.get().mode==RepeaterMode::BRIDGE?String("BRIDGE"):String("NAT")));
      } else if (ln=="reboot") {
        NVIC_SystemReset();
      }
      ln="";
    }
  }
}