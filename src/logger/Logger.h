/*                                      _     
        ___ _ __  _ __ ___ _ __  ___(_)___ 
        / __| '_ \| '__/ _ \ '_ \/ __| / __| 
        \__ \ |_) | | |  __/ | | \__ \ \__ \ 
        |___/ .__/|_|  \___|_| |_|___/_|___/ 
            |_|                             
                © Copyright 2025 
            ✈ `https://github.com/sprensis` 
    Name: Logger 
    Description: NVM-backed event logging 
    Author: @sprensis 
    Platform: BW16 (RTL8720dn) - Ameba Arduino 
    License: MIT 
*/

#pragma once
#include <Arduino.h>

struct LogRecord { uint32_t ts; uint8_t level; char msg[64]; };

class Logger {
 public:
  void begin();
  void info(const String& m);
  void warn(const String& m);
  void error(const String& m);
  size_t read(LogRecord* out, size_t max);
 private:
  void write(uint8_t lvl, const String& m);
};