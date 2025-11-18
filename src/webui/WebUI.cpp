/*                                      _     
        ___ _ __  _ __ ___ _ __  ___(_)___ 
        / __| '_ \| '__/ _ \ '_ \/ __| / __| 
        \__ \ |_) | | |  __/ | | \__ \ \__ \ 
        |___/ .__/|_|  \___|_| |_|___/_|___/ 
            |_|                             
                © Copyright 2025 
            ✈ `https://github.com/sprensis` 
    Name: WebUI 
    Description: Embedded web UI with auth 
    Author: @sprensis 
    Platform: BW16 (RTL8720dn) - Ameba Arduino 
    License: MIT 
*/

#include "WebUI.h"

void WebUI::begin(ConfigStore& cfg, Logger& log, WifiManager& wifi, NatManager& nat, DhcpServer& dhcp) {
  store=&cfg; logger=&log; wifiRef=&wifi; natRef=&nat; dhcpRef=&dhcp; server.begin();
}

void WebUI::loop() {
  WiFiClient c = server.available();
  if (c) handleClient(c);
}

bool WebUI::authOk(const String& h) {
  String u = store->get().webUser; String p = store->get().webPass; String token = b64(String(u+":"+p));
  String needle = String("Authorization: Basic ")+token;
  return h.indexOf(needle)>=0;
}

void WebUI::handleClient(WiFiClient& c) {
  String headers=""; headers.reserve(512); unsigned long t=millis(); while(c.connected() && millis()-t<400) { while(c.available()) { char ch=c.read(); headers+=ch; if(headers.endsWith("\r\n\r\n")) goto hdr_done; } }
hdr_done:
  if (!authOk(headers)) { c.print("HTTP/1.1 401 Unauthorized\r\nWWW-Authenticate: Basic realm=BW16\r\nContent-Length:0\r\n\r\n"); c.stop(); return; }
  int clPos = headers.indexOf("Content-Length:"); int contentLen = 0; if (clPos>=0) { int endLine = headers.indexOf("\r\n", clPos); if (endLine>clPos) { String v = headers.substring(clPos+15, endLine); v.trim(); contentLen = v.toInt(); } }
  String body=""; body.reserve(contentLen>0?contentLen:256); unsigned long t2=millis(); while(c.connected() && (int)body.length()<contentLen && millis()-t2<800) { while(c.available()) { body+=char(c.read()); if ((int)body.length()>=contentLen) break; } }
  String reqLine = headers.substring(0, headers.indexOf("\r\n"));
  String qPath = reqLine; int sp = qPath.indexOf(' '); if (sp>0) qPath = qPath.substring(0, sp);
  String prefillSsid=""; int qs = qPath.indexOf("?ssid="); if (qs>0) { prefillSsid = qPath.substring(qs+6); }
  bool didAuth=false; bool didConfig=false;
  if (reqLine.startsWith("GET /scan")) {
    int n = WiFi.scanNetworks();
    String html=""; html.reserve(1024);
    html += "<html><head><style>body{font-family:sans-serif;margin:20px}table{border-collapse:collapse}td,th{border:1px solid #ccc;padding:6px 8px}</style></head><body>";
    html += "<h3>Scan Results</h3><table><tr><th>SSID</th><th>Enc</th><th>RSSI</th></tr>";
    for(int i=0;i<n;i++){ String s=WiFi.SSID(i); int r=WiFi.RSSI(i); int e=WiFi.encryptionType(i); html += String("<tr><td>")+s+"</td><td>"+String(e)+"</td><td>"+String(r)+"</td></tr>"; }
    html += "</table><p><a href='/'>&larr; Back</a></p></body></html>";
    c.print("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: "+String(html.length())+"\r\n\r\n"+html);
    c.stop();
    return;
  }
  if (reqLine.startsWith("POST /auth")) {
    int uPos=body.indexOf("user="); int pPos=body.indexOf("pass=");
    if (uPos>=0) { int amp=body.indexOf('&', uPos); String uv=body.substring(uPos+5, amp>=0?amp:body.length()); uv.trim(); strncpy(store->mutableRef().webUser, uv.c_str(), 31); }
    if (pPos>=0) { int amp=body.indexOf('&', pPos); String pv=body.substring(pPos+5, amp>=0?amp:body.length()); pv.trim(); strncpy(store->mutableRef().webPass, pv.c_str(), 63); }
    store->save(); didAuth=true;
  }
  if (reqLine.startsWith("POST /config")) {
    int mPos=body.indexOf("mode="); int sPos=body.indexOf("ssid="); int pPos=body.indexOf("pass="); int chPos=body.indexOf("channel="); int apSPos=body.indexOf("apssid="); int apPPos=body.indexOf("appass=");
    if (mPos>=0) { int amp=body.indexOf('&', mPos); String mv=body.substring(mPos+5, amp>=0?amp:body.length()); store->mutableRef().mode=(mv.startsWith("BRIDGE")?RepeaterMode::BRIDGE:RepeaterMode::NAT); }
    if (sPos>=0) { int amp=body.indexOf('&', sPos); String sv=body.substring(sPos+5, amp>=0?amp:body.length()); sv.trim(); strncpy(store->mutableRef().staSsid, sv.c_str(), 63); }
    if (pPos>=0) { int amp=body.indexOf('&', pPos); String pv=body.substring(pPos+5, amp>=0?amp:body.length()); pv.trim(); strncpy(store->mutableRef().staPass, pv.c_str(), 63); }
    if (chPos>=0) { int amp=body.indexOf('&', chPos); String cv=body.substring(chPos+8, amp>=0?amp:body.length()); store->mutableRef().apChannel = (uint8_t)cv.toInt(); }
    int aPos = body.indexOf("auto="); if (aPos>=0) { int amp=body.indexOf('&', aPos); String av=body.substring(aPos+5, amp>=0?amp:body.length()); store->mutableRef().apAutoChannel = (av=="on"||av=="1"); }
    if (apSPos>=0) { int amp=body.indexOf('&', apSPos); String av=body.substring(apSPos+7, amp>=0?amp:body.length()); av.trim(); strncpy(store->mutableRef().apSsid, av.c_str(), 63); }
    if (apPPos>=0) { int amp=body.indexOf('&', apPPos); String apv=body.substring(apPPos+7, amp>=0?amp:body.length()); apv.trim(); strncpy(store->mutableRef().apPass, apv.c_str(), 63); }
    store->save(); didConfig=true;
    if (chPos>=0 || apSPos>=0 || apPPos>=0) wifiRef->restartAP();
    if (sPos>=0 || pPos>=0) wifiRef->restartSTA();
  }
  String html=""; html.reserve(1024);
  html += "<html><head><style>body{font-family:sans-serif;margin:20px}h3{margin:0 0 10px}fieldset{border:1px solid #ccc;padding:12px;margin-bottom:14px;border-radius:8px}legend{padding:0 6px}label{display:inline-block;min-width:90px;margin:6px 12px 6px 0}input,select{padding:6px 8px;margin:6px 0}button{padding:6px 12px} .row{display:flex;align-items:center;flex-wrap:wrap} .notice{background:#e6ffed;border:1px solid #b7ffcb;padding:8px 12px;border-radius:6px;margin-bottom:10px} .warn{background:#fff7e6;border:1px solid #ffe1a6;padding:8px 12px;border-radius:6px;margin-bottom:10px}</style></head><body>";
  if (didAuth || didConfig) { html += "<div class=notice>Saved & applied.</div>"; }
  html += "<h3>BW16 Repeater</h3><div class=row><div>Status: STA ";
  html += ipToStr(wifiRef->staIP());
  html += " | AP ";
  html += ipToStr(wifiRef->apIP());
  html += "</div><div>Last: "; html += wifiRef->lastStatusMsg(); html += "</div>";
  html += "<form method=post action=/config>";
  html += "<fieldset><legend>Mode</legend><label>Mode </label><select name=mode>";
  if (store->get().mode==RepeaterMode::BRIDGE) {
    html += "<option value=BRIDGE selected>BRIDGE</option><option value=NAT>NAT</option>";
  } else {
    html += "<option value=BRIDGE>BRIDGE</option><option value=NAT selected>NAT</option>";
  }
  html += "</select></fieldset>";
  html += "<fieldset><legend>STA Config</legend>";
  html += "<label>SSID </label><input name=ssid placeholder='Home WiFi SSID' value='";
  html += (prefillSsid.length()?prefillSsid:String(store->get().staSsid));
  html += "'>";
  html += "<label>Password </label><input type=password name=pass placeholder='WiFi password' value='";
  html += String(store->get().staPass);
  html += "'>";
  html += "</fieldset>";
  html += "<fieldset><legend>AP Config</legend>";
  html += "<label>SSID </label><input name=apssid placeholder='AP SSID' value='"; html += String(store->get().apSsid); html += "'>";
  html += "<label>Password </label><input type=password name=appass placeholder='AP password (>=8)' value='"; html += String(store->get().apPass); html += "'>";
  html += "<label>Channel </label><input name=channel value='"; html += String((int)store->get().apChannel); html += "'>";
  html += "<div class=warn>For AP+STA, set AP Channel equal to your router's channel.</div>";
  html += "<label><input type=checkbox name=auto"; if (store->get().apAutoChannel) html += " checked"; html += "> Auto Channel</label>";
  html += "</fieldset>";
  html += "<button>Save</button> <a href=/scan style='margin-left:10px'>Scan</a></form>";
  html += "<form method=post action=/auth><fieldset><legend>Web Auth</legend><label>User </label><input name=user value='";
  html += String(store->get().webUser);
  html += "'>";
  html += "<label>Password </label><input type=password name=pass value='";
  html += String(store->get().webPass);
  html += "'>";
  html += "<button>Update</button></fieldset></form>";
  html += "</body></html>";
  c.print("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: "+String(html.length())+"\r\n\r\n"+html);
  c.stop();
}