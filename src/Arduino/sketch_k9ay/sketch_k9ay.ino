/*
K9AY loop control box
ESP12 module
Irkutsk, December 2022
ra0sms@bk.ru

27052025 - Update for web-interface (CSS)
15102026 - Refactoring:
           - fixed memory leak and double HTTP responses in switch handlers
           - SSID/password length (33/65), HTML/JSON escaping of SSIDs
           - captive portal DNS works, non-blocking WiFi connect and scan
           - uptime via millis(), WLAN status handling for all states
           - state (direction/Rload/PA) and labels are saved to EEPROM
           - AJAX UI with compass layout, JSON API, mDNS (k9ay.local)
           - optional HTTP authentication, SoftAP name from chip ID (K9AY_Loop_xxxxxx)
           - SoftAP can be disabled while connected to WLAN, it turns on again
             automatically when the saved network is unavailable

HTTP API:
  GET  /api/state                   -> {"dir":1,"r":470,"pa":true,"labels":[...]}
  POST /set  dir=N|NE|E|SE|S|SW|W|NW  r=300|390|430|470|510|560  pa=0|1  off=1
       example: curl -d "dir=NE&r=470" http://k9ay.local/set
*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>
#include <SPI.h>
//**********************************************
#define FW_VERSION "2026-10-15"
// SoftAP name is built from chip ID: K9AY_Loop_xxxxxx
const char *softAP_password = "1234567890";
const char *myHostname = "k9ay";   // http://k9ay.local
// Web interface login. Empty user - no authentication
const char *www_user = "";
const char *www_pass = "";
//*********************************************
const uint8_t PA_pin = 4;
const uint8_t LED_pin = 5;
const uint8_t OE_pin = 12;
/***SPI*/
const uint8_t latchPin = 15;
const uint8_t clockPin = 14;
const uint8_t dataPin = 13;
/*******/
struct Direction {
  const char *code;
  uint8_t bits;   // second 74HC595
};
const Direction dirs[8] = {
  {"N",  0b00001000},
  {"NE", 0b00001001},
  {"E",  0b00001100},
  {"SE", 0b00000010},
  {"S",  0b00000000},
  {"SW", 0b00000001},
  {"W",  0b00000100},
  {"NW", 0b00001010},
};
const uint16_t rloads[6] = {300, 390, 430, 470, 510, 560};   // bit 0..5 of first 74HC595
//*********************************************
#define CFG_ADDR 128                 // 0..66 - credentials of old firmware (migrated)
#define CFG_MAGIC 0x4B394103UL
#define LABEL_LEN 24
struct Config {
  uint32_t magic;
  char ssid[33];
  char pass[65];
  char labels[8][LABEL_LEN];
  int8_t dir;     // -1 - off
  int8_t rload;   // -1 - off
  uint8_t pa;
  uint8_t apOff;  // 1 - disable SoftAP while connected to WLAN
} cfg;
//*********************************************
const byte DNS_PORT = 53;
DNSServer dnsServer;
ESP8266WebServer server(80);
IPAddress apIP(192, 168, 4, 1);
IPAddress netMsk(255, 255, 255, 0);
char apSsid[33];

const unsigned long RECONNECT_INTERVAL = 60000;   // retry after failed attempt
const unsigned long RETRY_ABORTED = 3000;         // retry after attempt aborted (e.g. by scan)
const unsigned long CONNECT_TIMEOUT = 30000;
const unsigned long STATE_SAVE_DELAY = 30000;
bool connectRequested = false;
unsigned long lastConnectTry = 0;
unsigned long nextConnectAt = 0;
bool connecting = false;   // STA connection attempt in progress
wl_status_t wifiStatus = WL_IDLE_STATUS;
uint32_t uptimeSec = 0;
uint32_t staConnectedAt = 0;
// SoftAP is switched off only after WLAN has been connected for AP_OFF_DELAY,
// and switched on again when WLAN has been lost for AP_ON_DELAY
const unsigned long AP_OFF_DELAY = 30000;
const unsigned long AP_ON_DELAY = 15000;
bool apActive = false;
bool staUp = false;
unsigned long staChangedAt = 0;
bool stateDirty = false;
unsigned long stateDirtyAt = 0;
//*********************************************

static const char STYLE_CSS[] PROGMEM = R"rawliteral(
:root{--primary:#3498db;--secondary:#2ecc71;--danger:#e74c3c;--dark:#2c3e50;--light:#ecf0f1}
*{box-sizing:border-box}
body{font-family:'Segoe UI',Roboto,Tahoma,Verdana,sans-serif;line-height:1.5;color:var(--dark);background:#f5f7fa;max-width:800px;margin:0 auto;padding:16px}
h1{color:var(--primary);text-align:center;margin:0 0 20px}
h2,h3{margin:0 0 12px}
.card{background:#fff;border-radius:8px;padding:20px;margin-bottom:20px;box-shadow:0 2px 10px rgba(0,0,0,.1)}
.nav{display:flex;flex-wrap:wrap;gap:10px}
.nav a{padding:10px 20px;background:var(--primary);color:#fff;text-decoration:none;border-radius:5px}
.nav a:hover{background:#2980b9}
.status{background:var(--light);padding:15px;border-radius:8px;text-align:center;font-size:18px;font-weight:500;margin-bottom:20px}
.btn{border:none;color:#fff;padding:12px 15px;font-size:16px;font-weight:500;border-radius:6px;cursor:pointer;min-width:80px;transition:all .2s;box-shadow:0 2px 5px rgba(0,0,0,.1)}
.btn:hover{transform:translateY(-2px);box-shadow:0 4px 8px rgba(0,0,0,.15)}
.btn:active{transform:translateY(0)}
.btn-dir{background:var(--primary)}
.btn-r{background:#9b59b6}
.btn-pa{background:#f39c12}
.btn-off{background:var(--danger)}
.btn.active{background:var(--secondary);box-shadow:0 0 0 3px rgba(46,204,113,.35)}
.compass{display:grid;grid-template-columns:repeat(3,1fr);gap:10px;max-width:360px;margin:0 auto}
.compass .btn{width:100%;padding:20px 5px;overflow:hidden;text-overflow:ellipsis}
.center{display:flex;align-items:center;justify-content:center;text-align:center;font-weight:700;font-size:20px;color:var(--secondary);border:2px dashed var(--light);border-radius:50%}
.row{display:grid;grid-template-columns:repeat(auto-fit,minmax(80px,1fr));gap:8px}
.mt{margin-top:20px}
table{width:100%;border-collapse:collapse}
th,td{padding:10px 12px;text-align:left;border-bottom:1px solid #ddd;word-break:break-all}
th{background:var(--primary);color:#fff}
tr.net{cursor:pointer}
tr.net:hover{background:#f5f5f5}
label{display:block}
input[type=text],input[type=password]{width:100%;padding:10px;margin:6px 0 12px;border:1px solid #ccc;border-radius:4px;font-size:16px}
input[type=checkbox]{width:18px;height:18px;vertical-align:middle;margin:0 6px 0 0}
input[type=submit],.submit{width:100%;background:var(--primary);color:#fff;padding:12px;border:none;border-radius:4px;font-size:16px;cursor:pointer;margin-top:8px}
input[type=submit]:hover,.submit:hover{background:#2980b9}
.submit.danger{background:var(--danger)}
.lbl-grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(150px,1fr));gap:0 12px}
.footer{text-align:center;margin-top:20px}
.footer a{color:var(--primary);text-decoration:none}
.footer a:hover{text-decoration:underline}
.muted{color:#7f8c8d;font-size:14px}
.err{color:var(--danger)}
)rawliteral";

static const char APP_JS[] PROGMEM = R"rawliteral(
var D=['N','NE','E','SE','S','SW','W','NW'];
function $(id){return document.getElementById(id)}
function fmtUp(s){return Math.floor(s/86400)+'d '+Math.floor(s/3600)%24+'h '+Math.floor(s/60)%60+'m '+s%60+'s'}
function chk(r){if(!r.ok)throw r.status;return r.json()}
function getJSON(u){return fetch(u,{cache:'no-store'}).then(chk)}
function postJSON(u,b){return fetch(u,{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:b}).then(chk)}
function showConn(i){
  $('conn').textContent=i.viaAP?'\u{1F535} Connected via Soft AP: '+i.apSsid:'\u{1F7E2} Connected to WiFi network: '+i.staSsid;
  $('up').textContent=fmtUp(i.uptime);
}
)rawliteral";

static const char PAGE_ROOT[] PROGMEM = R"rawliteral(<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>K9AY Controller</title>
  <link rel="stylesheet" href="/style.css">
</head>
<body>
  <h1>K9AY Controller</h1>
  <div class="card">
    <p id="conn">&hellip;</p>
    <p>Uptime: <strong id="up">&hellip;</strong></p>
    <p class="muted" id="fw"></p>
  </div>
  <div class="nav">
    <a href="/switch">Direction Control</a>
    <a href="/wifi">WiFi Configuration</a>
    <a href="/labels">Direction Labels</a>
  </div>
<script src="/app.js"></script>
<script>
getJSON('/api/info').then(function(i){showConn(i);$('fw').textContent='Firmware '+i.fw+' · http://'+i.host+'.local'});
</script>
</body>
</html>
)rawliteral";

static const char PAGE_SWITCH[] PROGMEM = R"rawliteral(<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>K9AY Controller</title>
  <link rel="stylesheet" href="/style.css">
</head>
<body>
  <div class="card">
    <h1>K9AY Controller</h1>
    <div class="status" id="status">&hellip;</div>

    <h2>Direction Control</h2>
    <div class="compass">
      <button class="btn btn-dir" data-d="7">NW</button>
      <button class="btn btn-dir" data-d="0">N</button>
      <button class="btn btn-dir" data-d="1">NE</button>
      <button class="btn btn-dir" data-d="6">W</button>
      <div class="center" id="cur">&mdash;</div>
      <button class="btn btn-dir" data-d="2">E</button>
      <button class="btn btn-dir" data-d="5">SW</button>
      <button class="btn btn-dir" data-d="4">S</button>
      <button class="btn btn-dir" data-d="3">SE</button>
    </div>

    <h2 class="mt">Rload Settings</h2>
    <div class="row">
      <button class="btn btn-r" data-r="300">300</button>
      <button class="btn btn-r" data-r="390">390</button>
      <button class="btn btn-r" data-r="430">430</button>
      <button class="btn btn-r" data-r="470">470</button>
      <button class="btn btn-r" data-r="510">510</button>
      <button class="btn btn-r" data-r="560">560</button>
    </div>

    <h2 class="mt">Preamp Settings</h2>
    <div class="row">
      <button class="btn btn-pa" id="paOn">PREAMP ON</button>
      <button class="btn btn-off" id="paOff">PREAMP OFF</button>
      <button class="btn btn-off" id="allOff">TURN ALL OFF</button>
    </div>

    <div class="footer">
      <a href="/">Return to Home Page</a>
    </div>
  </div>
<script src="/app.js"></script>
<script>
function each(sel,f){Array.prototype.forEach.call(document.querySelectorAll(sel),f)}
function render(s){
  each('[data-d]',function(b){var i=+b.dataset.d;b.textContent=s.labels[i];b.classList.toggle('active',s.dir==i)});
  each('[data-r]',function(b){b.classList.toggle('active',s.r==+b.dataset.r)});
  $('paOn').classList.toggle('active',s.pa);
  $('paOff').classList.toggle('active',!s.pa);
  var t=[];
  if(s.dir>=0)t.push(s.labels[s.dir]);
  if(s.r)t.push(s.r+' Ohm');
  if(s.pa)t.push('+15dB');
  $('status').textContent=t.length?t.join(' '):'OFF';
  $('status').classList.remove('err');
  $('cur').textContent=s.dir>=0?s.labels[s.dir]:'—';
}
function fail(){$('status').textContent='No connection to controller';$('status').classList.add('err')}
function set(b){postJSON('/set',b).then(render,fail)}
function poll(){if(!document.hidden)getJSON('/api/state').then(render,fail)}
each('[data-d]',function(b){b.onclick=function(){set('dir='+D[b.dataset.d])}});
each('[data-r]',function(b){b.onclick=function(){set('r='+b.dataset.r)}});
$('paOn').onclick=function(){set('pa=1')};
$('paOff').onclick=function(){set('pa=0')};
$('allOff').onclick=function(){set('off=1')};
poll();
setInterval(poll,2000);
</script>
</body>
</html>
)rawliteral";

static const char PAGE_WIFI[] PROGMEM = R"rawliteral(<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>WiFi Configuration - K9AY Controller</title>
  <link rel="stylesheet" href="/style.css">
</head>
<body>
  <h1>WiFi Configuration</h1>

  <div class="card">
    <p id="conn">&hellip;</p>
    <p>Uptime: <strong id="up">&hellip;</strong></p>
  </div>

  <div class="card">
    <h3>SoftAP Configuration</h3>
    <table>
      <tr><th>Parameter</th><th>Value</th></tr>
      <tr><td>SSID</td><td id="apSsid"></td></tr>
      <tr><td>Status</td><td id="apState"></td></tr>
      <tr><td>IP Address</td><td id="apIp"></td></tr>
    </table>
    <form method="POST" action="/apsave" class="mt">
      <label><input type="checkbox" name="apoff" value="1" id="apoff"> Disable access point while connected to WLAN</label>
      <p class="muted">The access point turns on automatically if the saved network is unavailable for 15 s,
        and always after power-on until the WLAN connection is established.
        Without the access point the controller is reachable only via the WLAN IP address or http://k9ay.local</p>
      <input type="submit" value="Save">
    </form>
  </div>

  <div class="card">
    <h3>WLAN Configuration</h3>
    <table>
      <tr><th>Parameter</th><th>Value</th></tr>
      <tr><td>SSID</td><td id="staSsid"></td></tr>
      <tr><td>Status</td><td id="staState"></td></tr>
      <tr><td>IP Address</td><td id="staIp"></td></tr>
      <tr><td>Signal</td><td id="staRssi"></td></tr>
    </table>
  </div>

  <div class="card">
    <h3>Available Networks</h3>
    <p class="muted">Click a network to select it</p>
    <table id="nets"><tr><th>Network Name</th><th>Signal</th></tr></table>
    <button class="submit" id="scanBtn">Scan</button>
  </div>

  <form class="card" method="POST" action="/wifisave">
    <h3>Connect to Network</h3>
    <label for="n">Network SSID:</label>
    <input type="text" id="n" name="n" maxlength="32" placeholder="Leave empty to forget network">
    <label for="p">Password:</label>
    <input type="password" id="p" name="p" maxlength="64" placeholder="Enter password">
    <input type="submit" value="Connect">
  </form>

  <div class="footer">
    <a href="/">&larr; Return to Home Page</a>
  </div>
<script src="/app.js"></script>
<script>
var apoffTouched=false;
$('apoff').onchange=function(){apoffTouched=true};
function info(){
  getJSON('/api/info').then(function(i){
    showConn(i);
    $('apSsid').textContent=i.apSsid;
    $('apState').textContent=i.apActive?'On':'Off';
    $('apIp').textContent=i.apActive?i.apIp:'\u2014';
    if(!apoffTouched)$('apoff').checked=i.apOff;
    $('staSsid').textContent=i.staSsid||'(not configured)';
    $('staState').textContent=i.connected?'Connected '+fmtUp(i.wlanUptime):(i.staSsid?'Not connected':'—');
    $('staIp').textContent=i.connected?i.staIp:'—';
    $('staRssi').textContent=i.connected?i.rssi+' dBm':'—';
  });
}
function row(a,b,cls){
  var tr=document.createElement('tr'),td1=document.createElement('td'),td2=document.createElement('td');
  td1.textContent=a;td2.textContent=b;tr.appendChild(td1);tr.appendChild(td2);
  if(cls)tr.className=cls;
  return tr;
}
function clearNets(){var t=$('nets');while(t.rows.length>1)t.deleteRow(1)}
function scan(){
  $('scanBtn').disabled=true;$('scanBtn').textContent='Scanning…';
  getJSON('/api/scan').then(function(r){
    if(r.scanning){setTimeout(scan,1000);return}
    var best={};
    r.nets.forEach(function(n){if(n.ssid&&(!best[n.ssid]||best[n.ssid].rssi<n.rssi))best[n.ssid]=n});
    var list=Object.keys(best).map(function(k){return best[k]}).sort(function(a,b){return b.rssi-a.rssi});
    clearNets();
    if(!list.length)$('nets').appendChild(row('No WiFi networks found',''));
    list.forEach(function(n){
      var tr=row(n.ssid+(n.secure?' (secured)':''),n.rssi+' dBm','net');
      tr.onclick=function(){$('n').value=n.ssid;$('p').focus()};
      $('nets').appendChild(tr);
    });
    $('scanBtn').disabled=false;$('scanBtn').textContent='Scan';
  },function(){$('scanBtn').disabled=false;$('scanBtn').textContent='Scan'});
}
$('scanBtn').onclick=scan;
info();
setInterval(info,5000);
scan();
</script>
</body>
</html>
)rawliteral";

static const char PAGE_LABELS[] PROGMEM = R"rawliteral(<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Direction Labels - K9AY Controller</title>
  <link rel="stylesheet" href="/style.css">
</head>
<body>
  <h1>Direction Labels</h1>
  <form class="card" method="POST" action="/labelsave">
    <p class="muted">Names shown on the direction buttons (azimuth, region, etc). Empty field - default name.</p>
    <div class="lbl-grid" id="fields"></div>
    <input type="submit" value="Save">
  </form>
  <form class="card" method="POST" action="/labelsave">
    <input type="hidden" name="reset" value="1">
    <input type="submit" class="submit danger" value="Reset to defaults">
  </form>
  <div class="footer">
    <a href="/">&larr; Return to Home Page</a>
  </div>
<script src="/app.js"></script>
<script>
D.forEach(function(d,i){
  var l=document.createElement('label'),inp=document.createElement('input');
  l.textContent=d;inp.type='text';inp.name='l'+i;inp.id='l'+i;inp.maxLength=23;
  l.appendChild(inp);$('fields').appendChild(l);
});
getJSON('/api/state').then(function(s){s.labels.forEach(function(v,i){$('l'+i).value=v})});
</script>
</body>
</html>
)rawliteral";

//*********************************************

String jsonEscape(const char *s)
{
  String o;
  for (; *s; s++) {
    char c = *s;
    if (c == '"' || c == '\\') {
      o += '\\';
      o += c;
    } else if ((uint8_t)c < 0x20) {
      char b[7];
      snprintf(b, sizeof(b), "\\u%04x", c);
      o += b;
    } else {
      o += c;
    }
  }
  return o;
}

void addJsonStr(String &j, const char *key, const char *val)
{
  j += '"';
  j += key;
  j += F("\":\"");
  j += jsonEscape(val);
  j += F("\",");
}

void setLabel(uint8_t i, const String &v)
{
  const size_t cap = LABEL_LEN - 1;
  size_t n = v.length();
  if (n > cap) {
    n = cap;
    // do not cut UTF-8 character in the middle
    while (n > 0 && ((uint8_t)v[n] & 0xC0) == 0x80) n--;
  }
  if (n == 0) {
    strlcpy(cfg.labels[i], dirs[i].code, LABEL_LEN);
    return;
  }
  memcpy(cfg.labels[i], v.c_str(), n);
  cfg.labels[i][n] = 0;
}

void setDefaults()
{
  memset(&cfg, 0, sizeof(cfg));
  cfg.magic = CFG_MAGIC;
  for (uint8_t i = 0; i < 8; i++) setLabel(i, "");
  cfg.dir = -1;
  cfg.rload = -1;
  cfg.pa = 0;
}

void saveConfig()
{
  EEPROM.put(CFG_ADDR, cfg);
  EEPROM.commit();
  stateDirty = false;
}

void loadConfig()
{
  EEPROM.get(CFG_ADDR, cfg);
  if (cfg.magic != CFG_MAGIC) {
    setDefaults();
    // Old firmware: ssid[32] at 0, password[32] at 32, "OK" at 64
    char ok[3];
    EEPROM.get(64, ok);
    if (ok[0] == 'O' && ok[1] == 'K' && ok[2] == 0) {
      for (uint8_t i = 0; i < 31; i++) {
        cfg.ssid[i] = EEPROM.read(i);
        cfg.pass[i] = EEPROM.read(32 + i);
      }
      Serial.println("Credentials migrated from old firmware");
    }
    saveConfig();
  }
  cfg.ssid[sizeof(cfg.ssid) - 1] = 0;
  cfg.pass[sizeof(cfg.pass) - 1] = 0;
  for (uint8_t i = 0; i < 8; i++) {
    cfg.labels[i][LABEL_LEN - 1] = 0;
    if (!cfg.labels[i][0]) setLabel(i, "");
  }
  if (cfg.dir < -1 || cfg.dir > 7) cfg.dir = -1;
  if (cfg.rload < -1 || cfg.rload > 5) cfg.rload = -1;
  cfg.pa = cfg.pa ? 1 : 0;
  cfg.apOff = cfg.apOff == 1 ? 1 : 0;
  Serial.println("Recovered credentials:");
  Serial.println(cfg.ssid);
  Serial.println(strlen(cfg.pass) > 0 ? "********" : "<no password>");
}

// Save switch state with delay to reduce flash wear on fast clicking
void markStateDirty()
{
  stateDirty = true;
  stateDirtyAt = millis();
}

void applyOutputs()
{
  uint8_t out1 = cfg.rload >= 0 ? (1 << cfg.rload) : 0;
  uint8_t out2 = cfg.dir >= 0 ? dirs[cfg.dir].bits : 0;
  digitalWrite(latchPin, LOW);
  SPI.transfer(out1);
  SPI.transfer(out2);
  digitalWrite(latchPin, HIGH);
  digitalWrite(PA_pin, cfg.pa ? HIGH : LOW);
}

void turnOffAll()
{
  cfg.dir = -1;
  cfg.rload = -1;
  cfg.pa = 0;
}

//*********************************************

void sendNoCache()
{
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
}

void sendPage(PGM_P page)
{
  sendNoCache();
  server.send_P(200, "text/html; charset=utf-8", page);
}

void sendJson(const String &j)
{
  sendNoCache();
  server.send(200, "application/json; charset=utf-8", j);
}

void redirect(const char *to)
{
  server.sendHeader("Location", to, true);
  server.send(303, "text/plain", "");
}

bool checkAuth()
{
  if (!www_user[0]) return true;
  if (server.authenticate(www_user, www_pass)) return true;
  server.requestAuthentication();
  return false;
}

void onAuth(const char *uri, HTTPMethod method, void (*fn)())
{
  server.on(uri, method, [fn]() {
    if (checkAuth()) fn();
  });
}

bool isIp(const String &s)
{
  for (char c : s)
    if (c != '.' && (c < '0' || c > '9')) return false;
  return s.length() > 0;
}

// SoftAP clients asking for a foreign host are redirected to our page (captive portal)
bool captivePortal()
{
  if (server.client().localIP() != apIP) return false;
  String host = server.hostHeader();
  if (isIp(host) || host == String(myHostname) + ".local") return false;
  server.sendHeader("Location", "http://" + apIP.toString() + "/", true);
  server.send(302, "text/plain", "");
  return true;
}

void handleRoot()
{
  if (captivePortal()) return;
  if (!checkAuth()) return;
  sendPage(PAGE_ROOT);
}

void handleSwitch() { sendPage(PAGE_SWITCH); }
void handleWifi()   { sendPage(PAGE_WIFI); }
void handleLabels() { sendPage(PAGE_LABELS); }

void handleState()
{
  String j;
  j.reserve(320);
  j = F("{\"dir\":");
  j += cfg.dir;
  j += F(",\"r\":");
  j += cfg.rload >= 0 ? rloads[cfg.rload] : 0;
  j += F(",\"pa\":");
  j += cfg.pa ? F("true") : F("false");
  j += F(",\"labels\":[");
  for (uint8_t i = 0; i < 8; i++) {
    if (i) j += ',';
    j += '"';
    j += jsonEscape(cfg.labels[i]);
    j += '"';
  }
  j += F("]}");
  sendJson(j);
}

void handleSet()
{
  // validate all arguments first, then apply
  int8_t dir = cfg.dir, rload = cfg.rload;
  uint8_t pa = cfg.pa;
  if (server.hasArg("off")) {
    dir = -1;
    rload = -1;
    pa = 0;
  }
  if (server.hasArg("dir")) {
    String d = server.arg("dir");
    dir = -2;
    for (uint8_t i = 0; i < 8; i++)
      if (d.equalsIgnoreCase(dirs[i].code)) dir = i;
    if (dir == -2) {
      server.send(400, "text/plain", "Bad dir");
      return;
    }
  }
  if (server.hasArg("r")) {
    long r = server.arg("r").toInt();
    rload = -2;
    for (uint8_t i = 0; i < 6; i++)
      if (rloads[i] == r) rload = i;
    if (rload == -2) {
      server.send(400, "text/plain", "Bad r");
      return;
    }
  }
  if (server.hasArg("pa")) pa = server.arg("pa") == "1";
  cfg.dir = dir;
  cfg.rload = rload;
  cfg.pa = pa;
  applyOutputs();
  markStateDirty();
  handleState();
}

void handleInfo()
{
  bool sta = WiFi.status() == WL_CONNECTED;
  String j;
  j.reserve(320);
  j = F("{\"viaAP\":");
  j += server.client().localIP() == apIP ? F("true,") : F("false,");
  addJsonStr(j, "apSsid", apSsid);
  addJsonStr(j, "apIp", WiFi.softAPIP().toString().c_str());
  addJsonStr(j, "staSsid", cfg.ssid);
  addJsonStr(j, "staIp", WiFi.localIP().toString().c_str());
  addJsonStr(j, "host", myHostname);
  addJsonStr(j, "fw", FW_VERSION);
  j += F("\"apActive\":");
  j += apActive ? F("true,") : F("false,");
  j += F("\"apOff\":");
  j += cfg.apOff ? F("true,") : F("false,");
  j += F("\"connected\":");
  j += sta ? F("true") : F("false");
  j += F(",\"rssi\":");
  j += sta ? WiFi.RSSI() : 0;
  j += F(",\"uptime\":");
  j += uptimeSec;
  j += F(",\"wlanUptime\":");
  j += sta ? uptimeSec - staConnectedAt : 0;
  j += '}';
  sendJson(j);
}

// Asynchronous scan: first call starts it, client polls until results are ready
void handleScan()
{
  int n = WiFi.scanComplete();
  // Scan aborts STA connection attempt - postpone it, the page polls again
  if (n == WIFI_SCAN_FAILED && connecting) n = WIFI_SCAN_RUNNING;
  if (n == WIFI_SCAN_FAILED) {
    WiFi.scanNetworks(true);
    n = WIFI_SCAN_RUNNING;
  }
  if (n == WIFI_SCAN_RUNNING) {
    sendJson(F("{\"scanning\":true}"));
    return;
  }
  String j = F("{\"scanning\":false,\"nets\":[");
  for (int i = 0; i < n; i++) {
    if (i) j += ',';
    j += F("{\"ssid\":\"");
    j += jsonEscape(WiFi.SSID(i).c_str());
    j += F("\",\"rssi\":");
    j += WiFi.RSSI(i);
    j += F(",\"secure\":");
    j += WiFi.encryptionType(i) != ENC_TYPE_NONE ? F("true") : F("false");
    j += '}';
  }
  j += F("]}");
  WiFi.scanDelete();
  sendJson(j);
}

void handleWifiSave()
{
  String n = server.arg("n"), p = server.arg("p");
  if (n.length() > 32 || p.length() > 64) {
    server.send(400, "text/plain", "SSID max 32, password max 64 characters");
    return;
  }
  Serial.println("wifi save");
  strlcpy(cfg.ssid, n.c_str(), sizeof(cfg.ssid));
  strlcpy(cfg.pass, p.c_str(), sizeof(cfg.pass));
  saveConfig();
  redirect("/wifi");
  WiFi.disconnect();
  connecting = false;
  connectRequested = cfg.ssid[0] != 0;
}

void handleApSave()
{
  cfg.apOff = server.arg("apoff") == "1";
  saveConfig();
  redirect("/wifi");
}

void handleLabelSave()
{
  bool reset = server.hasArg("reset");
  for (uint8_t i = 0; i < 8; i++) {
    String v = reset ? String() : server.arg("l" + String(i));
    v.trim();
    setLabel(i, v);
  }
  saveConfig();
  redirect("/labels");
}

void handleStatic(PGM_P content, const char *type)
{
  server.sendHeader("Cache-Control", "max-age=600");
  server.send_P(200, type, content);
}

void handleNotFound()
{
  if (captivePortal()) return;
  sendNoCache();
  server.send(404, "text/plain", "File Not Found");
}

//*********************************************

void connectWifi()
{
  Serial.print("Connecting as wifi client to ");
  Serial.println(cfg.ssid);
  WiFi.disconnect();
  WiFi.begin(cfg.ssid, cfg.pass);   // non-blocking, result is checked in wifiTask()
  lastConnectTry = millis();
  connecting = true;
}

void connectFinished(bool failed, unsigned long retryIn)
{
  connecting = false;
  nextConnectAt = millis() + retryIn;
  if (failed) {
    WiFi.disconnect();   // stop STA channel scanning, it makes SoftAP unstable
    Serial.printf("Connection failed, retry in %lu s\n", retryIn / 1000);
  }
}

void wifiTask()
{
  bool haveSsid = cfg.ssid[0] != 0;
  bool scanBusy = WiFi.scanComplete() == WIFI_SCAN_RUNNING;   // begin() during scan fails
  if (connectRequested && !scanBusy) {
    connectRequested = false;
    if (haveSsid) connectWifi();
  }
  wl_status_t s = WiFi.status();
  if ((s == WL_CONNECTED) != staUp) {
    staUp = s == WL_CONNECTED;
    staChangedAt = millis();
  }
  if (s != wifiStatus) {   // WLAN status change
    Serial.print("Status: ");
    Serial.println(s);
    wifiStatus = s;
    if (s == WL_CONNECTED) {
      Serial.print("Connected to ");
      Serial.println(cfg.ssid);
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());
      digitalWrite(LED_pin, HIGH);
      staConnectedAt = uptimeSec;
    } else {
      digitalWrite(LED_pin, LOW);
    }
  }
  if (connecting) {
    if (s == WL_CONNECTED) {
      connecting = false;
    } else if (s == WL_NO_SSID_AVAIL || s == WL_CONNECT_FAILED || s == WL_WRONG_PASSWORD) {
      connectFinished(true, RECONNECT_INTERVAL);
    } else if (s == WL_IDLE_STATUS && millis() - lastConnectTry > 500) {
      Serial.println("Connection attempt aborted");
      connectFinished(false, RETRY_ABORTED);
    } else if (millis() - lastConnectTry > CONNECT_TIMEOUT) {
      connectFinished(true, RECONNECT_INTERVAL);
    }
  } else if (haveSsid && !scanBusy && s != WL_CONNECTED && (long)(millis() - nextConnectAt) >= 0) {
    connectWifi();
  }
}

void startAP()
{
  if (apActive) return;
  WiFi.softAPConfig(apIP, apIP, netMsk);
  WiFi.softAP(apSsid, softAP_password);
  apActive = true;
  Serial.print("SoftAP on: ");
  Serial.println(apSsid);
}

void stopAP()
{
  if (!apActive) return;
  WiFi.softAPdisconnect(true);
  apActive = false;
  Serial.println("SoftAP off");
}

void apTask()
{
  unsigned long since = millis() - staChangedAt;
  if (cfg.apOff && cfg.ssid[0]) {
    if (staUp && since > AP_OFF_DELAY) stopAP();
    else if (!staUp && since > AP_ON_DELAY) startAP();
  } else {
    startAP();
  }
}

void uptimeTask()
{
  static unsigned long last = 0;
  while (millis() - last >= 1000) {
    last += 1000;
    uptimeSec++;
  }
}

void stateSaveTask()
{
  if (stateDirty && millis() - stateDirtyAt > STATE_SAVE_DELAY) saveConfig();
}

//*********************************************

void setup(void)
{
  delay(100);
  pinMode(PA_pin, OUTPUT);
  digitalWrite(PA_pin, LOW);
  pinMode(LED_pin, OUTPUT);
  digitalWrite(LED_pin, LOW);
  pinMode(OE_pin, OUTPUT);
  digitalWrite(OE_pin, HIGH);   // 74HC595 outputs disabled until data is loaded
  pinMode(dataPin, OUTPUT);
  digitalWrite(dataPin, LOW);
  pinMode(latchPin, OUTPUT);
  digitalWrite(latchPin, LOW);
  pinMode(clockPin, OUTPUT);
  digitalWrite(clockPin, LOW);
  delay(200);
  SPI.begin();
  // SPI.begin() switches GPIO12 to HSPI MISO, but here it is OE of 74HC595 - make it GPIO again
  pinMode(OE_pin, OUTPUT);
  digitalWrite(OE_pin, HIGH);

  Serial.begin(115200);
  Serial.println();
  EEPROM.begin(512);
  loadConfig();
  applyOutputs();   // restore last direction/Rload/PA
  digitalWrite(OE_pin, LOW);

  snprintf(apSsid, sizeof(apSsid), "K9AY_Loop_%06X", ESP.getChipId());

  Serial.print("Configuring access point...");
  WiFi.persistent(false);
  WiFi.setAutoReconnect(false);   // reconnection is handled in wifiTask()
  WiFi.mode(WIFI_AP_STA);
  WiFi.hostname(myHostname);
  startAP();   // always on after power-on, apTask() may switch it off later
  delay(500); // Without delay I've seen the IP address blank
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());

  /* Setup the DNS server redirecting all the domains to the apIP */
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "*", apIP);

  if (MDNS.begin(myHostname)) {
    MDNS.addService("http", "tcp", 80);
    Serial.printf("mDNS: http://%s.local\n", myHostname);
  }

  server.on("/", handleRoot);
  server.on("/generate_204", handleRoot);
  server.on("/fwlink", handleRoot);
  server.on("/style.css", HTTP_GET, []() { handleStatic(STYLE_CSS, "text/css"); });
  server.on("/app.js", HTTP_GET, []() { handleStatic(APP_JS, "application/javascript"); });
  onAuth("/switch", HTTP_GET, handleSwitch);
  onAuth("/wifi", HTTP_GET, handleWifi);
  onAuth("/labels", HTTP_GET, handleLabels);
  onAuth("/wifisave", HTTP_POST, handleWifiSave);
  onAuth("/labelsave", HTTP_POST, handleLabelSave);
  onAuth("/apsave", HTTP_POST, handleApSave);
  onAuth("/set", HTTP_POST, handleSet);
  onAuth("/api/state", HTTP_GET, handleState);
  onAuth("/api/info", HTTP_GET, handleInfo);
  onAuth("/api/scan", HTTP_GET, handleScan);
  server.onNotFound(handleNotFound);
  server.begin(); // Web server start
  Serial.println("HTTP server started");

  connectRequested = cfg.ssid[0] != 0; // Request WLAN connect if there is a SSID
}

void loop(void)
{
  uptimeTask();
  wifiTask();
  apTask();
  dnsServer.processNextRequest();
  MDNS.update();
  server.handleClient();
  stateSaveTask();
  delay(1);
}
