/*                                      _     
        ___ _ __  _ __ ___ _ __  ___(_)___ 
        / __| '_ \| '__/ _ \ '_ \/ __| / __| 
        \__ \ |_) | | |  __/ | | \__ \ \__ \ 
        |___/ .__/|_|  \___|_| |_|___/_|___/ 
            |_|                             
                © Copyright 2025 
            ✈ `https://github.com/sprensis` 
    Name: DhcpServer 
    Description: DHCP control for AP clients 
    Author: @sprensis 
    Platform: BW16 (RTL8720dn) - Ameba Arduino 
    License: MIT 
*/

#include "DhcpServer.h"

void DhcpServer::begin(ConfigStore& cfg, Logger& log) { store=&cfg; logger=&log; dns.begin(53); }
void DhcpServer::loop() { handleDns(); }
void DhcpServer::handleDns() {
  int p = dns.parsePacket();
  if (p<=0) return;
  uint8_t buf[512];
  int len = p>512?512:p;
  int r = dns.read(buf, len);
  if (r<=12) return;
  uint16_t id = (buf[0]<<8)|buf[1];
  int pos = 12;
  while (pos<r && buf[pos]!=0) { pos++; }
  if (pos>=r) return;
  pos++;
  if (pos+4>r) return;
  uint16_t qtype = (buf[pos]<<8)|buf[pos+1];
  uint16_t qclass = (buf[pos+2]<<8)|buf[pos+3];
  IPAddress ip = IPAddress((store->get().apIP>>24)&255, (store->get().apIP>>16)&255, (store->get().apIP>>8)&255, store->get().apIP&255);
  uint8_t ans[512];
  int a=0;
  ans[a++]=buf[0]; ans[a++]=buf[1];
  ans[a++]=0x81; ans[a++]=0x80;
  ans[a++]=buf[4]; ans[a++]=buf[5];
  ans[a++]=0x00; ans[a++]=0x01;
  ans[a++]=0x00; ans[a++]=0x00;
  ans[a++]=0x00; ans[a++]=0x00;
  for(int i=12;i<pos+4 && i<r;i++){ ans[a++]=buf[i]; }
  ans[a++]=0xC0; ans[a++]=0x0C;
  ans[a++]=0x00; ans[a++]=0x01;
  ans[a++]=0x00; ans[a++]=0x01;
  ans[a++]=0x00; ans[a++]=0x00; ans[a++]=0x00; ans[a++]=0x78;
  ans[a++]=0x00; ans[a++]=0x04;
  ans[a++]=ip[0]; ans[a++]=ip[1]; ans[a++]=ip[2]; ans[a++]=ip[3];
  dns.beginPacket(dns.remoteIP(), dns.remotePort());
  dns.write(ans, a);
  dns.endPacket();
}