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
  String headers=""; headers.reserve(768); unsigned long t=millis(); while(c.connected() && millis()-t<1500) { while(c.available()) { char ch=c.read(); headers+=ch; if(headers.endsWith("\r\n\r\n")) goto hdr_done; } }
hdr_done:
  String reqLine = headers.substring(0, headers.indexOf("\r\n"));
  if (reqLine.startsWith("GET /generate_204") || reqLine.startsWith("GET /hotspot-detect.html") || reqLine.startsWith("GET /success") || reqLine.startsWith("GET /ncsi.txt")) {
    String html = String("<html><head><meta http-equiv=\"refresh\" content=\"0;url=/\"></head><body>OK</body></html>");
    c.print("HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=UTF-8\r\nContent-Length: "+String(html.length())+"\r\n\r\n"+html);
    c.stop();
    return;
  }
  if (!authOk(headers)) { String body401="<html><head><meta charset='UTF-8'></head><body><h3>BW16 Web UI</h3><p>Authentication required.</p></body></html>"; c.print("HTTP/1.1 401 Unauthorized\r\nWWW-Authenticate: Basic realm=BW16\r\nContent-Type: text/html; charset=UTF-8\r\nContent-Length: "+String(body401.length())+"\r\n\r\n"+body401); c.stop(); return; }
  int clPos = headers.indexOf("Content-Length:"); int contentLen = 0; if (clPos>=0) { int endLine = headers.indexOf("\r\n", clPos); if (endLine>clPos) { String v = headers.substring(clPos+15, endLine); v.trim(); contentLen = v.toInt(); } }
  String body=""; body.reserve(contentLen>0?contentLen:256); unsigned long t2=millis(); while(c.connected() && (int)body.length()<contentLen && millis()-t2<2000) { while(c.available()) { body+=char(c.read()); if ((int)body.length()>=contentLen) break; } }
  reqLine = headers.substring(0, headers.indexOf("\r\n"));
  String qPath = reqLine; int sp = qPath.indexOf(' '); if (sp>0) qPath = qPath.substring(0, sp);
  String prefillSsid=""; int qs = qPath.indexOf("?ssid="); if (qs>0) { prefillSsid = qPath.substring(qs+6); }
  if (reqLine.startsWith("GET /status")) {
    String json="{";
    json += "\"sta\":\""+ipToStr(wifiRef->staIP())+"\",";
    json += "\"ap\":\""+ipToStr(wifiRef->apIP())+"\",";
    json += "\"last\":\""+wifiRef->lastStatusMsg()+"\"}";
    c.print("HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: "+String(json.length())+"\r\n\r\n"+json);
    c.stop();
    return;
  }
  bool didAuth=false; bool didConfig=false;
  if (reqLine.startsWith("GET /scan")) {
    int n = WiFi.scanNetworks();
    String html=""; html.reserve(1024);
    html += "<html><head><style>body{font-family:sans-serif;margin:20px}table{border-collapse:collapse}td,th{border:1px solid #ccc;padding:6px 8px}</style></head><body>";
    html += "<h3>Scan Results</h3><table><tr><th>SSID</th><th>Enc</th><th>RSSI</th></tr>";
    for(int i=0;i<n;i++){ String s=WiFi.SSID(i); int r=WiFi.RSSI(i); int e=WiFi.encryptionType(i); html += String("<tr><td>")+s+"</td><td>"+String(e)+"</td><td>"+String(r)+"</td></tr>"; }
    html += "</table><p><a href='/'>&larr; Back</a></p></body></html>";
    c.print("HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=UTF-8\r\nContent-Length: "+String(html.length())+"\r\n\r\n"+html);
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
    int stEn=body.indexOf("sta_static="); int stIp=body.indexOf("sta_ip="); int stMa=body.indexOf("sta_mask="); int stGw=body.indexOf("sta_gw=");
    if (mPos>=0) { int amp=body.indexOf('&', mPos); String mv=body.substring(mPos+5, amp>=0?amp:body.length()); store->mutableRef().mode=(mv.startsWith("BRIDGE")?RepeaterMode::BRIDGE:RepeaterMode::NAT); }
    if (sPos>=0) { int amp=body.indexOf('&', sPos); String sv=body.substring(sPos+5, amp>=0?amp:body.length()); sv.trim(); strncpy(store->mutableRef().staSsid, sv.c_str(), 63); }
    if (pPos>=0) { int amp=body.indexOf('&', pPos); String pv=body.substring(pPos+5, amp>=0?amp:body.length()); pv.trim(); strncpy(store->mutableRef().staPass, pv.c_str(), 63); }
    if (chPos>=0) { int amp=body.indexOf('&', chPos); String cv=body.substring(chPos+8, amp>=0?amp:body.length()); store->mutableRef().apChannel = (uint8_t)cv.toInt(); }
    int aPos = body.indexOf("auto="); if (aPos>=0) { int amp=body.indexOf('&', aPos); String av=body.substring(aPos+5, amp>=0?amp:body.length()); store->mutableRef().apAutoChannel = (av=="on"||av=="1"); }
    if (apSPos>=0) { int amp=body.indexOf('&', apSPos); String av=body.substring(apSPos+7, amp>=0?amp:body.length()); av.trim(); strncpy(store->mutableRef().apSsid, av.c_str(), 63); }
    if (apPPos>=0) { int amp=body.indexOf('&', apPPos); String apv=body.substring(apPPos+7, amp>=0?amp:body.length()); apv.trim(); strncpy(store->mutableRef().apPass, apv.c_str(), 63); }
    if (stEn>=0) { int amp=body.indexOf('&', stEn); String v=body.substring(stEn+11, amp>=0?amp:body.length()); store->mutableRef().staStatic = (v=="on"||v=="1"); }
    auto parseIP=[&](const String& s){ uint8_t a=0,b=0,c=0,d=0; sscanf(s.c_str(), "%hhu.%hhu.%hhu.%hhu", &a,&b,&c,&d); return (uint32_t(a)<<24)|(uint32_t(b)<<16)|(uint32_t(c)<<8)|uint32_t(d); };
    if (stIp>=0) { int amp=body.indexOf('&', stIp); String sv=body.substring(stIp+7, amp>=0?amp:body.length()); store->mutableRef().staIP = parseIP(sv); }
    if (stMa>=0) { int amp=body.indexOf('&', stMa); String sv=body.substring(stMa+9, amp>=0?amp:body.length()); store->mutableRef().staMask = parseIP(sv); }
    if (stGw>=0) { int amp=body.indexOf('&', stGw); String sv=body.substring(stGw+7, amp>=0?amp:body.length()); store->mutableRef().staGateway = parseIP(sv); }
    store->save(); didConfig=true;
    if (chPos>=0 || apSPos>=0 || apPPos>=0) wifiRef->restartAP();
    if (sPos>=0 || pPos>=0 || stEn>=0 || stIp>=0 || stMa>=0 || stGw>=0) wifiRef->restartSTA();
  }
  String html=""; html.reserve(4096);
  html += "<html lang='en'><head><meta charset='UTF-8'><style>";
  html += "body{font-family:-apple-system,BlinkMacSystemFont,Segoe UI,Roboto,Ubuntu,Helvetica,Arial,sans-serif;margin:0;background:#f0f2f5;color:#1f2937}";
  html += ".container{max-width:980px;margin:0 auto;padding:20px}";
  html += ".topbar{background:#1f2937;color:#f9fafb;padding:14px 22px;display:flex;align-items:center;justify-content:space-between}";
  html += ".brand{font-weight:600;letter-spacing:.2px}";
  html += ".nav a{color:#f9fafb;text-decoration:none;margin-left:14px;opacity:.8}";
  html += ".nav a:hover{opacity:1;text-decoration:underline}";
  html += ".card{background:#fff;border:1px solid #d1d5db;border-radius:12px;padding:16px;margin:18px 0;box-shadow:0 4px 12px rgba(0,0,0,.05)}";
  html += "fieldset{border:1px solid #d1d5db;padding:12px;margin-bottom:14px;border-radius:10px}";
  html += "legend{padding:0 6px}";
  html += "label{display:inline-block;min-width:120px;margin:6px 12px 6px 0}";
  html += "input,select{padding:8px 10px;margin:6px 0;max-width:360px;border:1px solid #d1d5db;border-radius:8px}";
  html += "button{padding:8px 14px;background:#4b5563;color:#fff;border:0;border-radius:8px}";
  html += ".row{display:flex;align-items:center;flex-wrap:wrap;gap:10px}";
  html += ".notice{background:#f0f9ff;border:1px solid #e0f2fe;padding:8px 12px;border-radius:8px;margin-bottom:10px}";
  html += ".warn{background:#fefce8;border:1px solid #fef9c3;padding:8px 12px;border-radius:8px;margin-bottom:10px}";
  html += ".faq h3{margin:0 0 10px}";
  html += ".faq h4{margin:10px 0 6px}";
  html += "html[lang=ru] .en{display:none}";
  html += "html[lang=en] .ru{display:none}";
  html += ".hint{font-size:12px;color:#64748b;margin-top:4px}";
  html += ".modal{position:fixed;left:0;top:0;right:0;bottom:0;background:rgba(15,23,42,0.5);display:none;align-items:center;justify-content:center;z-index:9999}";
  html += ".modal.visible{display:flex}";
  html += ".modal-content{background:#fff;border-radius:16px;max-width:960px;width:92%;padding:18px;border:1px solid #d1d5db;box-shadow:0 12px 36px rgba(0,0,0,.15);animation:pop .18s ease-out}";
  html += ".modal-content .close{float:right;background:#374151;color:#fff;border:0;border-radius:999px;padding:6px 10px}";
  html += "@keyframes pop{from{opacity:0;transform:scale(.96)}to{opacity:1;transform:scale(1)}}";
  html += "</style><script>document.addEventListener('click',function(e){var t=e.target;if(t.tagName==='A'&&t.dataset&&t.dataset.open){e.preventDefault();var m=document.getElementById(t.dataset.open+'Modal');if(m){m.className='modal visible';}}if(t.dataset&&t.dataset.close){var m=document.getElementById(t.dataset.close+'Modal');if(m){m.className='modal';}}if(t.tagName==='BUTTON'&&t.dataset&&t.dataset.l){document.documentElement.setAttribute('lang',t.dataset.l);} });</script></head><body>";
  html += "<div class=topbar><div class=brand>BW16 Repeater</div><div class=nav><a href='/' >Status</a><a href='/scan'>Scan</a><a href='#' data-open='faq'>FAQ</a></div></div><div class=container>";
  if (didAuth || didConfig) { html += "<div class=notice>Saved & applied.</div>"; }
  html += "<div class=card><h3>BW16 Repeater</h3><div class=row><div>Status: STA ";
  html += ipToStr(wifiRef->staIP());
  html += " | AP ";
  html += ipToStr(wifiRef->apIP());
  html += "</div><div>Last: "; html += wifiRef->lastStatusMsg(); html += "</div></div>";
  html += "<div class=card><form method=post action=/config>";
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
  html += "<div class=hint en>Home Wi‑Fi network name</div><div class=hint ru>Имя вашей домашней сети Wi‑Fi</div><div class=hint en>Home Wi‑Fi password</div><div class=hint ru>Пароль вашей домашней сети Wi‑Fi</div></fieldset>";
  html += "<fieldset><legend>STA Static IP</legend>";
  html += "<label><input type=checkbox name=sta_static"; if (store->get().staStatic) html += " checked"; html += "> Enable</label>";
  auto toIpStr=[&](uint32_t v){ IPAddress ip((v>>24)&255,(v>>16)&255,(v>>8)&255,v&255); return ipToStr(ip); };
  html += "<label>IP </label><input name=sta_ip value='"; html += toIpStr(store->get().staIP); html += "'>";
  html += "<label>Mask </label><input name=sta_mask value='"; html += toIpStr(store->get().staMask); html += "'>";
  html += "<label>Gateway </label><input name=sta_gw value='"; html += toIpStr(store->get().staGateway); html += "'>";
  html += "</fieldset>";
  html += "<fieldset><legend>AP Config</legend>";
  html += "<label>SSID </label><input name=apssid placeholder='AP SSID' value='"; html += String(store->get().apSsid); html += "'>";
  html += "<label>Password </label><input type=password name=appass placeholder='AP password (>=8)' value='"; html += String(store->get().apPass); html += "'>";
  html += "<label>Channel </label><input name=channel value='"; html += String((int)store->get().apChannel); html += "'>";
  html += "<div class=warn en>For AP+STA, set AP Channel equal to your router's channel.</div><div class=warn ru>Для AP+STA установите канал AP равным каналу вашего роутера.</div>";
  html += "<label><input type=checkbox name=auto"; if (store->get().apAutoChannel) html += " checked"; html += "> Auto Channel</label>";
  html += "</fieldset>";
  html += "<button>Save</button> <a href=/scan style='margin-left:10px'>Scan</a></form></div>";
  html += "<div class=card><form method=post action=/auth><fieldset><legend>Web Auth</legend><label>User </label><input name=user value='";
  html += String(store->get().webUser);
  html += "'>";
  html += "<label>Password </label><input type=password name=pass value='";
  html += String(store->get().webPass);
  html += "'>";
  html += "<button>Update</button></fieldset></form></div>";
  html += "<div id=faqModal class='modal'><div class='modal-content'><button class=close data-close='faq'>×</button><div class='faq'>";
  html += "<div class=row><div class=lang><button data-l='en'>English</button> <button data-l='ru'>Русский</button></div></div>";
  html += "<h3 class='en'>FAQ</h3><h3 class='ru'>Вопросы и ответы</h3>";
  html += "<h4 class='en'>What is the BW16-Repeater AP?</h4><p class='en'>SoftAP provides a local hotspot. Clients receive IP like 192.168.4.x and can open Web UI at 192.168.4.1.</p>";
  html += "<h4 class='ru'>Что такое точка доступа BW16-Repeater?</h4><p class='ru'>SoftAP создаёт локальную сеть. Клиенты получают IP вида 192.168.4.x и открывают Web UI по адресу 192.168.4.1.</p>";
  html += "<h4 class='en'>Why 192.168.4.1 might not open?</h4><p class='en'>Ensure the device is connected to BW16-Repeater and has DHCP IP (see monitor). Some systems rely on captive checks; our DNS/HTTP captive is enabled. Use browser and enter http://192.168.4.1 directly.</p>";
  html += "<h4 class='ru'>Почему может не открываться 192.168.4.1?</h4><p class='ru'>Убедитесь, что устройство подключено к BW16-Repeater и получило IP по DHCP (смотрите монитор). Некоторые системы используют каптив‑проверки; у нас включён DNS/HTTP каптив. Введите http://192.168.4.1 напрямую в браузере.</p>";
  html += "<h4 class='en'>What do typical logs mean?</h4><ul class='en'><li>\"AP start 3 on ch X\": SoftAP started on channel X.</li><li>\"DHCP assign ip = 192.168.4.100\": client got IP from AP.</li><li>\"Accept connection successfully\": a client socket connected to Web UI.</li><li>\"NAT: STA not connected\": STA is not connected yet; configure SSID/password.</li><li>\"ERROR on binding\": first server bind may fail; it recovers.</li></ul>";
  html += "<h4 class='ru'>Что означают типичные логи?</h4><ul class='ru'><li>\"AP start 3 on ch X\": SoftAP запущен на канале X.</li><li>\"DHCP assign ip = 192.168.4.100\": клиент получил IP от AP.</li><li>\"Accept connection successfully\": клиентский сокет подключился к Web UI.</li><li>\"NAT: STA not connected\": STA ещё не подключён; настройте SSID/пароль.</li><li>\"ERROR on binding\": первый bind сервера может не пройти; далее восстанавливается.</li></ul>";
  html += "<h4 class='en'>How to configure STA?</h4><p class='en'>Use the STA form above or the serial commands: set_sta &lt;ssid&gt; &lt;pass&gt;. If your router uses fixed channels, set AP channel to match the STA channel for AP+STA.</p>";
  html += "<h4 class='ru'>Как настроить STA?</h4><p class='ru'>Используйте форму STA выше или команды по Serial: set_sta &lt;ssid&gt; &lt;pass&gt;. Если роутер на фиксированном канале, выставьте канал AP равным каналу STA для AP+STA.</p>";
  html += "</div></div></div>";
  html += "</div></body></html>";
  c.print("HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=UTF-8\r\nContent-Length: "+String(html.length())+"\r\n\r\n"+html);
  c.stop();
}
