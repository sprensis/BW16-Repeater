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

#include "WifiManager.h"
#include "../utils/BuildConfig.h"
#include "../utils/Utils.h"

void WifiManager::begin(ConfigStore& cfg, Logger& log) {
  logger = &log;
  cfgRef = &cfg;
  auto& c = cfg.get();
  startAP(cfg.get());
  delay(500);
  if (cfg.get().staSsid[0]) startSTA(cfg.get());
  lastMsg = String("AP started on ch ")+String((int)cfg.get().apChannel);
}

void WifiManager::startSTA(const RepeaterConfig& c) {
  if (!c.staSsid[0]) { if (logger) logger->warn(String("STA skipped: empty SSID")); return; }
  WiFi.disconnect();
  if (c.staStatic && c.staIP && c.staMask && c.staGateway) {
    IPAddress ip((c.staIP>>24)&255, (c.staIP>>16)&255, (c.staIP>>8)&255, c.staIP&255);
    IPAddress gw((c.staGateway>>24)&255, (c.staGateway>>16)&255, (c.staGateway>>8)&255, c.staGateway&255);
    IPAddress mask((c.staMask>>24)&255, (c.staMask>>16)&255, (c.staMask>>8)&255, c.staMask&255);
    WiFi.config(ip, gw, mask);
  }
  WiFi.begin((char*)c.staSsid, c.staPass);
  lastMsg = String("Connecting STA to ")+String(c.staSsid);
}

void WifiManager::startAP(const RepeaterConfig& c) {
  IPAddress ip((c.apIP>>24)&255, (c.apIP>>16)&255, (c.apIP>>8)&255, c.apIP&255);
  IPAddress gw((c.apGateway>>24)&255, (c.apGateway>>16)&255, (c.apGateway>>8)&255, c.apGateway&255);
  IPAddress mask((c.apMask>>24)&255, (c.apMask>>16)&255, (c.apMask>>8)&255, c.apMask&255);
  WiFi.config(ip, gw, mask);
  int chInt = (int)c.apChannel;
  if (chInt>13) chInt=6;
  String ch = String(chInt);
  int st = 0;
  if (c.apPass[0]) {
    st = WiFi.apbegin((char*)c.apSsid, (char*)c.apPass, (char*)ch.c_str(), 0);
  } else {
    st = WiFi.apbegin((char*)c.apSsid, (char*)ch.c_str(), (uint8_t)0);
  }
  if (logger) {
    logger->info(String("AP start ")+String(st)+String(" on ch ")+String(chInt));
  }
  delay(120);
  lastMsg = String("AP restarted on ch ")+String(chInt);
}

void WifiManager::loop() {
  if (millis()-lastCheck>3000) { lastCheck=millis(); ensureReconnect(); }
}

void WifiManager::ensureReconnect() {
  if (WiFi.status()!=WL_CONNECTED) {
    if (cfgRef && cfgRef->get().staSsid[0]) {
      if (millis()-lastStaTryMs>6000) { lastStaTryMs=millis(); WiFi.begin((char*)cfgRef->get().staSsid, cfgRef->get().staPass); lastMsg = String("STA retry: ")+String(cfgRef->get().staSsid); }
      if (millis()-lastFallbackMs>12000) {
        lastFallbackMs = millis();
        auto& conf = cfgRef->mutableRef();
        bool autoCh = conf.apAutoChannel;
        int n = WiFi.scanNetworks();
        bool seen=false; String target=String(conf.staSsid);
        for(int i=0;i<n;i++){ String s = String(WiFi.SSID(i)); if (s == target) { seen=true; break; } }
        if (autoCh && !seen) {
          const uint8_t cand24[13] = {1,2,3,4,5,6,7,8,9,10,11,12,13};
          conf.apChannel = cand24[fallbackIndex%13];
          startAP(conf);
          fallbackIndex++;
          lastMsg = String("AP channel rotate ")+String((int)conf.apChannel);
          if (logger) logger->warn(lastMsg);
        }
      }
    }
  } else {
    triedFallback=false;
    lastMsg = String("STA connected: ")+ipToStr(WiFi.localIP());
  }
}

bool WifiManager::staConnected() const {
  IPAddress lip = WiFi.localIP();
  IPAddress aip = apIP();
  if (WiFi.status()!=WL_CONNECTED) return false;
  if (lip[0]==0 && lip[1]==0 && lip[2]==0 && lip[3]==0) return false;
  return !(lip[0]==aip[0] && lip[1]==aip[1] && lip[2]==aip[2] && lip[3]==aip[3]);
}
IPAddress WifiManager::staIP() const { return WiFi.localIP(); }
IPAddress WifiManager::apIP() const {
  if (!cfgRef) return IPAddress(192,168,4,1);
  uint32_t v = cfgRef->get().apIP; return IPAddress((v>>24)&255, (v>>16)&255, (v>>8)&255, v&255);
}

void WifiManager::restartAP() { if (cfgRef) startAP(cfgRef->get()); }
void WifiManager::restartSTA() { if (cfgRef) startSTA(cfgRef->get()); }