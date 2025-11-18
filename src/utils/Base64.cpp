/*                                      _     
        ___ _ __  _ __ ___ _ __  ___(_)___ 
        / __| '_ \| '__/ _ \ '_ \/ __| / __| 
        \__ \ |_) | | |  __/ | | \__ \ \__ \ 
        |___/ .__/|_|  \___|_| |_|___/_|___/ 
            |_|                             
                © Copyright 2025 
            ✈ `https://github.com/sprensis` 
    Name: Base64 
    Description: Base64 encoding helper 
    Author: @sprensis 
    Platform: BW16 (RTL8720dn) - Ameba Arduino 
    License: MIT 
*/

#include "Base64.h"

static const char* T="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
String b64(const String& in){
  String out=""; int val=0, valb=-6; for (size_t i=0;i<in.length();i++){ unsigned char c=in[i]; val=(val<<8)+c; valb+=8; while(valb>=0){ out+=T[(val>>valb)&0x3F]; valb-=6; } } if(valb>-6) out+=T[((val<<8)>>(valb+8))&0x3F]; while(out.length()%4) out+='='; return out; }