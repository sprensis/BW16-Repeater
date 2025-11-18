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

#pragma once
#include <Arduino.h>

enum class RepeaterMode { NAT, BRIDGE };

struct RepeaterConfig {
  char staSsid[64];
  char staPass[64];
  char apSsid[64];
  char apPass[64];
  uint8_t apChannel;
  bool apAutoChannel;
  char webUser[32];
  char webPass[64];
  bool prefer5g;
  RepeaterMode mode;
  uint32_t apIP;
  uint32_t apMask;
  uint32_t apGateway;
};

class ConfigStore {
 public:
  void begin();
  void load();
  void save();
  const RepeaterConfig& get() const { return cfg; }
  RepeaterConfig& mutableRef() { return cfg; }
  RepeaterMode mode() const { return cfg.mode; }
 private:
  RepeaterConfig cfg;
};