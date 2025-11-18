/*                                      _     
        ___ _ __  _ __ ___ _ __  ___(_)___ 
        / __| '_ \| '__/ _ \ '_ \/ __| / __| 
        \__ \ |_) | | |  __/ | | \__ \ \__ \ 
        |___/ .__/|_|  \___|_| |_|___/_|___/ 
            |_|                             
                © Copyright 2025 
            ✈ `https://github.com/sprensis` 
    Name: ConfigStore 
    Description: Persistent configuration for BW16 repeater 
    Author: @sprensis 
    Platform: BW16 (RTL8720dn) - Ameba Arduino 
    License: MIT 
*/

#include "ConfigStore.h"
#include <string.h>

static const uint32_t MAGIC = 0xB8162025;

void ConfigStore::begin() {
  memset(&cfg, 0, sizeof(cfg));
  cfg.mode = RepeaterMode::NAT;
  cfg.prefer5g = true;
  strcpy(cfg.apSsid, "BW16-Repeater");
  strcpy(cfg.apPass, "bw16pass");
  cfg.apChannel = 1;
  cfg.apAutoChannel = true;
  strcpy(cfg.webUser, "admin");
  strcpy(cfg.webPass, "admin");
  cfg.apIP = (192<<24)|(168<<16)|(4<<8)|1;
  cfg.apMask = (255<<24)|(255<<16)|(255<<8)|0;
  cfg.apGateway = cfg.apIP;
  cfg.staStatic = false;
  cfg.staIP = 0;
  cfg.staMask = 0;
  cfg.staGateway = 0;
}

void ConfigStore::load() {
  begin();
#ifdef FLASHMEMORY_H
  uint32_t hdr[2];
  FlashMemory.read(0, (uint8_t*)hdr, sizeof(hdr));
  if (hdr[0] == MAGIC && hdr[1] == sizeof(RepeaterConfig)) {
    FlashMemory.read(sizeof(hdr), (uint8_t*)&cfg, sizeof(cfg));
  }
#endif
}

void ConfigStore::save() {
#ifdef FLASHMEMORY_H
  uint32_t hdr[2] = {MAGIC, sizeof(RepeaterConfig)};
  FlashMemory.write(0, (const uint8_t*)hdr, sizeof(hdr));
  FlashMemory.write(sizeof(hdr), (const uint8_t*)&cfg, sizeof(cfg));
  FlashMemory.commit();
#endif
}