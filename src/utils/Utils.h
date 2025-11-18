/*                                      _     
        ___ _ __  _ __ ___ _ __  ___(_)___ 
        / __| '_ \| '__/ _ \ '_ \/ __| / __| 
        \__ \ |_) | | |  __/ | | \__ \ \__ \ 
        |___/ .__/|_|  \___|_| |_|___/_|___/ 
            |_|                             
                © Copyright 2025 
            ✈ `https://github.com/sprensis` 
    Name: Utils 
    Description: Helpers 
    Author: @sprensis 
    Platform: BW16 (RTL8720dn) - Ameba Arduino 
    License: MIT 
*/

#pragma once
#include <Arduino.h>

inline uint32_t ip4(uint8_t a,uint8_t b,uint8_t c,uint8_t d){return (a<<24)|(b<<16)|(c<<8)|d;}
inline String ipToStr(const IPAddress& ip){ return String(ip[0])+"."+String(ip[1])+"."+String(ip[2])+"."+String(ip[3]); }