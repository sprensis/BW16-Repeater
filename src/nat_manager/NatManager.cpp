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

#include "NatManager.h"
#include "../utils/Utils.h"

void NatManager::beginNat(ConfigStore& cfg, Logger& log, WifiManager& wifi) { logger=&log; wifiRef=&wifi; natMode=true; }
void NatManager::beginBridge(ConfigStore& cfg, Logger& log, WifiManager& wifi) { logger=&log; wifiRef=&wifi; natMode=false; }

void NatManager::loop() { if (natMode) runNatTick(); else runBridgeTick(); }

void NatManager::runNatTick() {
  if (!wifiRef) return;
  if (millis()-lastProbe>30000) {
    lastProbe=millis();
    if (wifiRef->staConnected()) {
      WiFiClient c;
      bool ok=false;
      if (c.connect(IPAddress(1,1,1,1), 53)) { ok=true; c.stop(); }
      if (!ok && c.connect(IPAddress(8,8,8,8), 53)) { ok=true; c.stop(); }
      if (logger) logger->info(String("NAT probe ")+(ok?String("OK"):String("FAIL")));
    } else {
      if (logger) logger->warn(String("NAT: STA not connected"));
    }
  }
}
void NatManager::runBridgeTick() {
  if (!wifiRef) return;
  if (millis()-lastProbe>30000) {
    lastProbe=millis();
    if (wifiRef->staConnected()) {
      if (logger) logger->info(String("BRIDGE probe OK"));
    } else {
      if (logger) logger->warn(String("BRIDGE: STA not connected"));
    }
  }
}