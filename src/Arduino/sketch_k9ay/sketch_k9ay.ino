/*
K9AY loop control box
ESP12 module
Irkutsk, December 2022
ra0sms@bk.ru

27052025 - Update for web-interface (CSS)
*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>
#include <SPI.h>
//**********************************************
const char *softAP_ssid = "K9AY_Loop_sn044";
const char *softAP_password = "1234567890";
const char *myHostname = "esp8266";
//*********************************************
void buildXML ();
void saveLabels ();
void saveCredentials ();
void CheckStat ();
void handleXML ();
void buildJavascript ();
void handleSwitch ();
void handleRoot ();
void handleLabel (); 
void handleLabelSave ();
void loadLabels ();
void handleWifi ();
void handleNotFound ();
void handleWifiSave ();
void loadCredentials ();
void turnOffAll ();
void connectWifi ();
 //********************************************
char ssid[32] = "";
char password[32] = "";
char label0[32] ="N";
char label1[32] ="NE";
char label2[32] ="E";
char label3[32] ="SE";
char label4[32] ="S";
char label5[32] ="SW";
char label6[32] ="W";
char label7[32] ="NW";
char label8[32] ="";
char label300[32] = "300";
char label390[32] = "390";
char label430[32] = "430";
char label470[32] = "470";
char label510[32] = "510";
char label560[32] = "560";
char labelpa[32] = "+15dB";
String currentlabel ="";
String currentRload ="";
String currentPa =" ";
String webPage = "";
String webPage2 = "";
String ourPage = "";
String Page3="";
String javaScript, XML;
//********************************************* 
int PA_pin = 4;
int LED_pin = 5;
int OE_pin = 12;
/***SPI*/
int latchPin = 15;
int clockPin = 14;
int dataPin = 13;
/*******/
char stat[8] =  "";
int tim = 0;
int sec = 0;
int minute = 0;
int hour = 0;
int day = 0;
int flagAP=0;
int flag_off=0;
int out1=0;
int out2=0;
//*********************************************
const byte DNS_PORT = 53;
DNSServer dnsServer;
ESP8266WebServer server(80);
IPAddress apIP(192, 168, 4, 1);
IPAddress netMsk(255, 255, 255, 0);
boolean connect;
unsigned long lastConnectTry = 0;
unsigned int status = WL_IDLE_STATUS;
//*********************************************

void CheckStat ()
{
  currentlabel ="";
  if (stat[0] == '1') currentlabel+=String(label0)+ " ";
  if (stat[1] == '1') currentlabel+=String(label1)+ " ";
  if (stat[2] == '1') currentlabel+=String(label2)+ " ";
  if (stat[3] == '1') currentlabel+=String(label3)+ " ";
  if (stat[4] == '1') currentlabel+=String(label4)+ " ";
  if (stat[5] == '1') currentlabel+=String(label5)+ " ";
  if (stat[6] == '1') currentlabel+=String(label6)+ " ";
  if (stat[7] == '1') currentlabel+=String(label7);
}

void handleXML() {
  buildXML();
  server.send(200, "text/xml", XML);
}


void buildJavascript() {
  javaScript = R"(
<SCRIPT>
var xmlHttp = createXmlHttpObject();

function createXmlHttpObject() {
  if (window.XMLHttpRequest) {
    xmlHttp = new XMLHttpRequest();
  } else {
    xmlHttp = new ActiveXObject('Microsoft.XMLHTTP');
  }
  return xmlHttp;
}

function process() {
  if (xmlHttp.readyState == 0 || xmlHttp.readyState == 4) {
    xmlHttp.open('PUT', 'xml', true);
    xmlHttp.onreadystatechange = handleServerResponse;
    xmlHttp.send(null);
  }
  setTimeout('process()', 100);
}

function handleServerResponse() {
  if (xmlHttp.readyState == 4 && xmlHttp.status == 200) {
    var xmlResponse = xmlHttp.responseXML;
    var xmldoc = xmlResponse.getElementsByTagName('response');
    var message = xmldoc[0].firstChild.nodeValue;
    document.getElementById('runtime').innerHTML = message;
  }
}
</SCRIPT>
)";
}

void buildXML() {
  XML = "<?xml version='1.0'?><response>";
  XML += currentlabel + " " + currentRload + " " + currentPa;
  XML += "</response>";
}


void handleSwitch() {
  buildJavascript();
  digitalWrite(clockPin, LOW);

  String webPage = String(R"(
<!DOCTYPE HTML>
<html>
<head>
  <title>K9AY Antenna Controller</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    :root {
      --primary: #3498db;
      --secondary: #2ecc71;
      --danger: #e74c3c;
      --dark: #2c3e50;
      --light: #ecf0f1;
    }
    
    body {
      font-family: 'Roboto', sans-serif;
      background-color: #f5f7fa;
      margin: 0;
      padding: 20px;
      color: var(--dark);
    }
    
    .container {
      max-width: 800px;
      margin: 0 auto;
      background: white;
      border-radius: 10px;
      box-shadow: 0 4px 12px rgba(0,0,0,0.1);
      padding: 25px;
    }
    
    h1 {
      color: var(--primary);
      text-align: center;
      margin-bottom: 25px;
    }
    
    h2 {
      color: var(--dark);
      margin-top: 20px;
    }
    
    .status-display {
      background: var(--light);
      padding: 15px;
      border-radius: 8px;
      text-align: center;
      font-size: 18px;
      margin-bottom: 20px;
      font-weight: 500;
    }
    
    .control-panel {
      display: flex;
      flex-direction: column;
      gap: 15px;
    }
    
    .direction-grid {
      display: grid;
      grid-template-columns: repeat(2, 1fr);
      gap: 10px;
      justify-items: center;
    }
    
    .btn {
      border: none;
      color: white;
      padding: 10px 15px;
      text-align: center;
      text-decoration: none;
      display: inline-block;
      font-size: 16px;
      font-weight: 500;
      border-radius: 6px;
      cursor: pointer;
      transition: all 0.3s ease;
      min-width: 80px;
      box-shadow: 0 2px 5px rgba(0,0,0,0.1);
    }
    
    .btn:hover {
      transform: translateY(-2px);
      box-shadow: 0 4px 8px rgba(0,0,0,0.15);
    }
    
    .btn:active {
      transform: translateY(0);
    }
    
    .btn-direction {
      background-color: var(--primary);
    }
    
    .btn-direction.active {
      background-color: var(--secondary);
      box-shadow: 0 0 0 3px rgba(46, 204, 113, 0.3);
    }
    
    .btn-rload {
      background-color: #9b59b6;
    }
    
    .btn-rload.active {
      background-color: var(--secondary);
      box-shadow: 0 0 0 3px rgba(142, 68, 173, 0.3);
    }
    
    .btn-pa {
      background-color: #f39c12;
    }
    
    .btn-pa.active {
      background-color: var(--secondary);
      box-shadow: 0 0 0 3px rgba(230, 126, 34, 0.3);
    }
    
    .btn-off {
      background-color: var(--danger);
    }
    
    .options-grid {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(80px, 1fr));
      gap: 5px;
    }
    
    .footer {
      text-align: center;
      margin-top: 30px;
      color: #7f8c8d;
    }
    
    .footer a {
      color: var(--primary);
      text-decoration: none;
    }
    
    .footer a:hover {
      text-decoration: underline;
    }
    
    @media (max-width: 600px) {
      .direction-grid {
        grid-template-columns: repeat(2, 1fr);
      }
    }
  </style>
</head>
)");

  webPage += javaScript;
  webPage += String(R"(
<body>
  <div class="container">
    <h1>K9AY Antenna Controller</h1>
      <div class="control-panel">
      <h2>Direction Control</h2>
      <div class="direction-grid">
        <a href="setN"><button class="btn btn-direction)") + 
        (currentlabel == "N" ? " active\"" : "\"") + String(R"(>N</button></a>
        <a href="setNW"><button class="btn btn-direction)") + 
        (currentlabel == "NW" ? " active\"" : "\"") + String(R"(>NW</button></a>
        <a href="setE"><button class="btn btn-direction)") + 
        (currentlabel == "E" ? " active\"" : "\"") + String(R"(>E</button></a>
        <a href="setNE"><button class="btn btn-direction)") + 
        (currentlabel == "NE" ? " active\"" : "\"") + String(R"(>NE</button></a>
        <a href="setW"><button class="btn btn-direction)") + 
        (currentlabel == "W" ? " active\"" : "\"") + String(R"(>W</button></a>
        <a href="setSW"><button class="btn btn-direction)") + 
        (currentlabel == "SW" ? " active\"" : "\"") + String(R"(>SW</button></a>
        <a href="setS"><button class="btn btn-direction)") + 
        (currentlabel == "S" ? " active\"" : "\"") + String(R"(>S</button></a>
        <a href="setSE"><button class="btn btn-direction)") + 
        (currentlabel == "SE" ? " active\"" : "\"") + String(R"(>SE</button></a>
      </div>
      
      <h2>Rload Settings</h2>
      <div class="options-grid">
        <a href="set300"><button class="btn btn-rload)") + 
        (currentRload == " 300 Ohm" ? " active\"" : "\"") + String(R"(>300</button></a>
        <a href="set390"><button class="btn btn-rload)") + 
        (currentRload == " 390 Ohm" ? " active\"" : "\"") + String(R"(>390</button></a>
        <a href="set430"><button class="btn btn-rload)") + 
        (currentRload == " 430 Ohm" ? " active\"" : "\"") + String(R"(>430</button></a>
        <a href="set470"><button class="btn btn-rload)") + 
        (currentRload == " 470 Ohm" ? " active\"" : "\"") + String(R"(>470</button></a>
        <a href="set510"><button class="btn btn-rload)") + 
        (currentRload == " 510 Ohm" ? " active\"" : "\"") + String(R"(>510</button></a>
        <a href="set560"><button class="btn btn-rload)") + 
        (currentRload == " 560 Ohm" ? " active\"" : "\"") + String(R"(>560</button></a>
      </div>
      
      <h2>Preamp Settings</h2>
      <div style="display: flex; gap: 10px;">
        <a href="setPa"><button class="btn btn-pa)") + 
        (currentPa == " +15dB" ? " active\"" : "\"") + String(R"(>PREAMP ON</button></a>
        <a href="resetPa"><button class="btn btn-off)") + 
        (currentPa == " " ? " active\"" : "\"") + String(R"(>PREAMP OFF</button></a>
        <a href="resetAll"><button class="btn btn-off">TURN ALL OFF</button></a>
      </div>
    </div>
    
    <div class="footer">
      <a href="/">Return to Home Page</a>
    </div>
  </div>
</body>
</html>
)");

  server.send(200, "text/html", webPage);
}




void handleRoot() {
  // Запрещаем кэширование страницы
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  
  
  // Формируем HTML страницу
  String Page = String(R"(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>K9AY Loop Controller</title>
  <style>
    :root {
      --primary: #3498db;
      --secondary: #2ecc71;
      --dark: #2c3e50;
      --light: #ecf0f1;
    }
    
    body {
      font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
      line-height: 1.6;
      color: var(--dark);
      max-width: 800px;
      margin: 0 auto;
      padding: 20px;
      background-color: #f5f7fa;
    }
    
    header {
      text-align: center;
      margin-bottom: 30px;
    }
    
    h1 {
      color: var(--primary);
      margin-bottom: 10px;
    }
    
    .status-card {
      background: white;
      border-radius: 8px;
      padding: 20px;
      margin-bottom: 20px;
      box-shadow: 0 2px 10px rgba(0,0,0,0.1);
    }
    
    .nav-links {
      display: flex;
      gap: 15px;
      margin-bottom: 20px;
    }
    
    .nav-links a {
      display: inline-block;
      padding: 10px 20px;
      background-color: var(--primary);
      color: white;
      text-decoration: none;
      border-radius: 5px;
      transition: background-color 0.3s;
    }
    
    .nav-links a:hover {
      background-color: #2980b9;
    }
    
  </style>
</head>
<body>
  <header>
    <h1>K9AY RX Loop Controller</h1>
  </header>
  
  <div class="status-card">
)");

  // Добавляем информацию о подключении
  if (server.client().localIP() == apIP) {
    Page += F("<p>🔵 Connected via Soft AP: <strong>");
    Page += softAP_ssid;
    Page += F("</strong></p>");
  } else {
    Page += F("<p>🟢 Connected to WiFi network: <strong>");
    Page += ssid;
    Page += F("</strong></p>");
  }

  Page += F(R"(
  </div>
  
  <div class="nav-links">
    <a href='/wifi'>WiFi Configuration</a>
    <a href='/switch'>Direction Control</a>
  </div>
  

)");
  // Завершаем страницу
  Page += F(R"(
</body>
</html>
)");

  // Отправляем страницу клиенту
  server.send(200, "text/html", Page);
}

void saveLabels() {
  EEPROM.begin(512);
  EEPROM.put(100, label0);
  EEPROM.put(100 + sizeof(label0), label1);
  EEPROM.put(100 + sizeof(label0) + sizeof(label1), label2);
  EEPROM.put(100 + sizeof(label0) + sizeof(label1) + sizeof(label2), label3);
  EEPROM.put(100 + sizeof(label0) + sizeof(label1) + sizeof(label2) + sizeof(label3), label4);
  EEPROM.put(100 + sizeof(label0) + sizeof(label1) + sizeof(label2) + sizeof(label3) + sizeof(label4), label5);
  EEPROM.put(100 + sizeof(label0) + sizeof(label1) + sizeof(label2) + sizeof(label3) + sizeof(label4) + sizeof(label5), label6);
  EEPROM.put(100 + sizeof(label0) + sizeof(label1) + sizeof(label2) + sizeof(label3) + sizeof(label4) + sizeof(label5) + sizeof(label6), label7);
  char ok[8 + 1] = "OK";
  EEPROM.put(100 + sizeof(label0) + sizeof(label1) + sizeof(label2) + sizeof(label3) + sizeof(label4) + sizeof(label5) + sizeof(label6) + sizeof(label7), ok);
  EEPROM.commit();
  EEPROM.end();
}

void handleWifi() {
  // Запрещаем кэширование страницы
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");

  // Формируем HTML страницу
  String Page = String(R"(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>WiFi Configuration - K9AY Controller</title>
  <style>
    :root {
      --primary: #3498db;
      --secondary: #2ecc71;
      --danger: #e74c3c;
      --dark: #2c3e50;
      --light: #ecf0f1;
    }
    
    body {
      font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
      line-height: 1.6;
      color: var(--dark);
      max-width: 800px;
      margin: 0 auto;
      padding: 20px;
      background-color: #f5f7fa;
    }
    
    h1 {
      color: var(--primary);
      text-align: center;
      margin-bottom: 20px;
    }
    
    .status-card {
      background: white;
      border-radius: 8px;
      padding: 20px;
      margin-bottom: 20px;
      box-shadow: 0 2px 10px rgba(0,0,0,0.1);
    }
    
    .btn {
      border: none;
      color: white;
      padding: 8px 16px;
      text-align: center;
      text-decoration: none;
      display: inline-block;
      font-size: 14px;
      font-weight: 500;
      border-radius: 6px;
      cursor: pointer;
      transition: all 0.3s ease;
      min-width: 60px;
    }
    
    .btn-on {
      background-color: var(--secondary);
    }
    
    .btn-off {
      background-color: var(--danger);
    }
    
    .btn:hover {
      opacity: 0.9;
      transform: translateY(-1px);
    }
    
    table {
      width: 100%;
      border-collapse: collapse;
      margin: 15px 0;
      background: white;
      border-radius: 8px;
      overflow: hidden;
      box-shadow: 0 2px 10px rgba(0,0,0,0.1);
    }
    
    th, td {
      padding: 12px 15px;
      text-align: left;
      border-bottom: 1px solid #ddd;
    }
    
    th {
      background-color: var(--primary);
      color: white;
    }
    
    tr:hover {
      background-color: #f5f5f5;
    }
    
    form {
      background: white;
      padding: 20px;
      border-radius: 8px;
      box-shadow: 0 2px 10px rgba(0,0,0,0.1);
      margin: 20px 0;
    }
    
    input[type="text"],
    input[type="password"] {
      width: 100%;
      padding: 10px;
      margin: 8px 0;
      display: inline-block;
      border: 1px solid #ccc;
      border-radius: 4px;
      box-sizing: border-box;
    }
    
    input[type="submit"] {
      width: 100%;
      background-color: var(--primary);
      color: white;
      padding: 12px 20px;
      margin: 8px 0;
      border: none;
      border-radius: 4px;
      cursor: pointer;
      font-size: 16px;
    }
    
    input[type="submit"]:hover {
      background-color: #2980b9;
    }
    
    .ap-status {
      display: inline-block;
      padding: 4px 8px;
      border-radius: 4px;
      font-weight: bold;
    }
    
    .ap-on {
      background-color: var(--secondary);
      color: white;
    }
    
    .ap-off {
      background-color: var(--danger);
      color: white;
    }
    
    .footer {
      text-align: center;
      margin-top: 20px;
    }
    
    .footer a {
      color: var(--primary);
      text-decoration: none;
    }
    
    .footer a:hover {
      text-decoration: underline;
    }
  </style>
</head>
<body>
  <h1>WiFi Configuration</h1>
  
  <div class="status-card">
)");

  // Добавляем информацию о подключении
  if (server.client().localIP() == apIP) {
    Page += F("<p>🔵 Connected via <strong>Soft AP</strong>: ");
    Page += softAP_ssid;
    Page += F("</p>");
  } else {
    Page += F("<p>🟢 Connected to <strong>WiFi Network</strong>: ");
    Page += ssid;
    Page += F("</p>");
  }

  // Добавляем uptime
  Page += F("<p>Uptime: <strong>");
  Page += String(day) + "d " + String(hour) + "h " + String(minute) + "m " + String(sec) + "s";
  Page += F("</strong></p>");
  
  // Добавляем управление AP
  /*Page += F(R"(
    <p>
      <a href="APon" class="btn btn-on">ON</a>
      <a href="APoff" class="btn btn-off">OFF</a>
      Access Point Status: <span class="ap-status )");
  Page += (flag_off == 0 ? "ap-on\">ON" : "ap-off\">OFF");*/
  Page += F(R"(</span>
    </p>
  </div>
  
  <div class="status-card">
    <h3>SoftAP Configuration</h3>
    <table>
      <tr><th>Parameter</th><th>Value</th></tr>
      <tr><td>SSID</td><td>)");
  Page += softAP_ssid;
  Page += F(R"(</td></tr>
      <tr><td>IP Address</td><td>)");
  Page += WiFi.softAPIP().toString();
  Page += F(R"(</td></tr>
    </table>
  </div>
  
  <div class="status-card">
    <h3>WLAN Configuration</h3>
    <table>
      <tr><th>Parameter</th><th>Value</th></tr>
      <tr><td>SSID</td><td>)");
  Page += ssid;
  Page += F(R"(</td></tr>
      <tr><td>IP Address</td><td>)");
  Page += WiFi.localIP().toString();
  Page += F(R"(</td></tr>
    </table>
  </div>
  
  <div class="status-card">
    <h3>Available Networks</h3>
    <p>Click refresh if networks are missing</p>
    <table>
      <tr><th>Network Name</th><th>Signal</th></tr>
)");

  // Сканируем сети WiFi
  Serial.println("scan start");
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  
  if (n > 0) {
    for (int i = 0; i < n; i++) {
      Page += F("<tr><td>");
      Page += WiFi.SSID(i);
      if (WiFi.encryptionType(i) != ENC_TYPE_NONE) {
        Page += F(" <small>(secured)</small>");
      }
      Page += F("</td><td>");
      Page += WiFi.RSSI(i);
      Page += F(" dBm</td></tr>");
    }
  } else {
    Page += F("<tr><td colspan=\"2\">No WiFi networks found</td></tr>");
  }

  Page += F(R"(
    </table>
  </div>
  
  <form method='POST' action='wifisave'>
    <h3>Connect to Network</h3>
    <label for="n">Network SSID:</label>
    <input type="text" id="n" name="n" placeholder="Enter network name" required>
    
    <label for="p">Password:</label>
    <input type="password" id="p" name="p" placeholder="Enter password">
    
    <input type="submit" value="Connect">
  </form>
  
  <div class="footer">
    <a href="/">← Return to Home Page</a>
  </div>
</body>
</html>
)");

  server.send(200, "text/html", Page);
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.send(404, "text/plain", message);
}

void handleWifiSave() {
  Serial.println("wifi save");
  server.arg("n").toCharArray(ssid, sizeof(ssid) - 1);
  server.arg("p").toCharArray(password, sizeof(password) - 1);
  server.sendHeader("Location", "wifi", true);
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.send(302, "text/plain", "");    // Empty content inhibits Content-length header so we have to close the socket ourselves.
  server.client().stop(); // Stop is needed because we sent no content length
  saveCredentials();
  connect = strlen(ssid) > 0; // Request WLAN connect with new credentials if there is a SSID
}

void loadCredentials() {
  EEPROM.begin(512);
  EEPROM.get(0, ssid);
  EEPROM.get(0 + sizeof(ssid), password);
  char ok[2 + 1];
  EEPROM.get(0 + sizeof(ssid) + sizeof(password), ok);
  EEPROM.end();
  if (String(ok) != String("OK")) {
    ssid[0] = 0;
    password[0] = 0;
  }
  Serial.println("Recovered credentials:");
  Serial.println(ssid);
  Serial.println(strlen(password) > 0 ? "********" : "<no password>");
}

/** Store WLAN credentials to EEPROM */
void saveCredentials() {
  EEPROM.begin(512);
  EEPROM.put(0, ssid);
  EEPROM.put(0 + sizeof(ssid), password);
  char ok[2 + 1] = "OK";
  EEPROM.put(0 + sizeof(ssid) + sizeof(password), ok);
  EEPROM.commit();
  EEPROM.end();
}

void turnOffAll()
{
  currentlabel="";
  currentRload="";
  currentPa="";
  stat[0]=stat[1]=stat[2]=stat[3]=stat[4]=stat[5]=stat[6]=stat[7]='0';
  out1=0;
  out2=0;
  digitalWrite(latchPin, LOW);
  SPI.transfer(0);
  SPI.transfer(0);
  digitalWrite(latchPin, HIGH);
  digitalWrite(PA_pin, LOW);
}
 
void setup(void){
  delay(100);
  pinMode(PA_pin, OUTPUT);
  digitalWrite(PA_pin, LOW);
  pinMode(LED_pin, OUTPUT);
  digitalWrite(LED_pin, LOW);
  pinMode(OE_pin, OUTPUT);
  digitalWrite(OE_pin, HIGH);
  pinMode(dataPin, OUTPUT);
  digitalWrite(dataPin, LOW);
  pinMode(latchPin, OUTPUT);
  digitalWrite(latchPin, LOW);
  pinMode(clockPin, OUTPUT);
  digitalWrite(clockPin, LOW);
  delay(200);
  digitalWrite(OE_pin, LOW);
  SPI.begin();
  digitalWrite(latchPin, LOW);
  SPI.transfer(0);
  SPI.transfer(0);
  digitalWrite(latchPin, HIGH);
  pinMode(OE_pin, OUTPUT);
  digitalWrite(OE_pin, LOW);
  digitalWrite(clockPin, LOW);
  delay(100);
  Serial.begin(115200);
  Serial.println();
  Serial.print("Configuring access point...");
  /* You can remove the password parameter if you want the AP to be open. */
  WiFi.softAPConfig(apIP, apIP, netMsk);
  WiFi.softAP(softAP_ssid, softAP_password);
  delay(500); // Without delay I've seen the IP address blank
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());

  /* Setup the DNS server redirecting all the domains to the apIP */
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "*", apIP);
 
  server.on("/", handleRoot);
  server.on("/wifi", handleWifi);
  server.on("/switch", handleSwitch);
  server.on("/wifisave", handleWifiSave);
  server.on("/generate_204", handleRoot);
  server.on("/fwlink", handleRoot);  
  server.on("/xml", handleXML);
  server.onNotFound(handleNotFound);
  server.begin(); // Web server start
  Serial.println("HTTP server started");
  loadCredentials(); // Load WLAN credentials from network
  connect = strlen(ssid) > 0; // Request WLAN connect if there is a SSID
  dnsServer.processNextRequest();
  server.handleClient();
  
  server.on("/APon", [](){
    if (flag_off==1)
    {
        WiFi.softAPConfig(apIP, apIP, netMsk);
        WiFi.softAP(softAP_ssid, softAP_password);
        delay(500); 
        flag_off=0;
    }
    Page3 += "<script> document.location.href = \"/wifi\"</script>";
    server.send(200, "text/html", Page3);
    handleWifi(); 
  });

  server.on("/APoff", [](){
    flag_off=1;
    WiFi.softAPdisconnect(true);
    Page3 += "<script> document.location.href = \"/wifi\"</script>";
    server.send(200, "text/html", Page3);
    handleWifi();
  });
  
  server.on("/setN", [](){
    /*turnOffAll();*/
    out2=0b00001000;
    currentlabel=String(label0);
    webPage += "<script> document.location.href = \"/switch\"</script>";
    server.send(200, "text/html", webPage);
    digitalWrite(latchPin, LOW);
    SPI.transfer(out1);
    SPI.transfer(out2);
    digitalWrite(latchPin, HIGH);
    digitalWrite(clockPin, LOW);
    handleSwitch();
  });
  server.on("/resetN", [](){
    /*turnOffAll();*/
    currentlabel="OFF";
    webPage += "<script> document.location.href = \"/switch\"</script>";
    server.send(200, "text/html", webPage);
    handleSwitch();
  });
  server.on("/setNE", [](){
    /*turnOffAll();*/
    out2=0b00001001;
    currentlabel=String(label1);
    webPage += "<script> document.location.href = \"/switch\"</script>";
    server.send(200, "text/html", webPage);
    digitalWrite(latchPin, LOW);
    SPI.transfer(out1);
    SPI.transfer(out2);
    digitalWrite(latchPin, HIGH);
    handleSwitch();
  });
  server.on("/resetNE", [](){
    /*turnOffAll();*/
    currentlabel="OFF";
    webPage += "<script> document.location.href = \"/switch\"</script>";
    server.send(200, "text/html", webPage);
    handleSwitch();
  });
  server.on("/setE", [](){
    /*turnOffAll();*/
    out2=0b00001100;
    currentlabel=String(label2);
    webPage += "<script> document.location.href = \"/switch\"</script>";
    server.send(200, "text/html", webPage);
    digitalWrite(latchPin, LOW);
    SPI.transfer(out1);
    SPI.transfer(out2);
    digitalWrite(latchPin, HIGH);
    handleSwitch();
  });
  server.on("/resetE", [](){
    /*turnOffAll();*/
    currentlabel="OFF";
    webPage += "<script> document.location.href = \"/switch\"</script>";
    server.send(200, "text/html", webPage);
    handleSwitch();
  });
  server.on("/setSE", [](){
    /*turnOffAll();*/
    out2=0b00000010;
    currentlabel=String(label3);
    webPage += "<script> document.location.href = \"/switch\"</script>";
    server.send(200, "text/html", webPage);
    digitalWrite(latchPin, LOW);
    SPI.transfer(out1);
    SPI.transfer(out2);
    digitalWrite(latchPin, HIGH);
    handleSwitch();
  });
  server.on("/resetSE", [](){
   /*turnOffAll();*/
    currentlabel="OFF";
    webPage += "<script> document.location.href = \"/switch\"</script>";
    server.send(200, "text/html", webPage);
    handleSwitch();
  });
  server.on("/setS", [](){
    /*turnOffAll();*/
    out2=0b00000000;
    currentlabel=String(label4);
    webPage += "<script> document.location.href = \"/switch\"</script>";
    server.send(200, "text/html", webPage);
    digitalWrite(latchPin, LOW);
    SPI.transfer(out1);
    SPI.transfer(out2);
    digitalWrite(latchPin, HIGH);
    handleSwitch();
  });
  server.on("/resetS", [](){
    /*turnOffAll();*/
    currentlabel="OFF";
    webPage += "<script> document.location.href = \"/switch\"</script>";
    server.send(200, "text/html", webPage);
    handleSwitch();
  });
  server.on("/setSW", [](){
    /*turnOffAll();*/
    out2=0b00000001;
    currentlabel=String(label5);
    webPage += "<script> document.location.href = \"/switch\"</script>";
    server.send(200, "text/html", webPage);
    digitalWrite(latchPin, LOW);
    SPI.transfer(out1);
    SPI.transfer(out2);
    digitalWrite(latchPin, HIGH);
    handleSwitch();
  });
  server.on("/resetSW", [](){
    /*turnOffAll();*/
    currentlabel="OFF";
    server.send(200, "text/html", webPage);
    webPage += "<script> document.location.href = \"/switch\"</script>";
    handleSwitch();
  });
  server.on("/setW", [](){
    /*turnOffAll();*/
    out2=0b00000100;
    currentlabel=String(label6);
    webPage += "<script> document.location.href = \"/switch\"</script>";
    server.send(200, "text/html", webPage);
    digitalWrite(latchPin, LOW);
    SPI.transfer(out1);
    SPI.transfer(out2);
    digitalWrite(latchPin, HIGH);
    handleSwitch();
  });
  server.on("/resetW", [](){
    /*turnOffAll();*/
    currentlabel="OFF";
    webPage += "<script> document.location.href = \"/switch\"</script>";
    server.send(200, "text/html", webPage);
    handleSwitch();
  });
  server.on("/setNW", [](){
   /*turnOffAll();*/
    out2=0b00001010;
    currentlabel=String(label7);
    webPage += "<script> document.location.href = \"/switch\"</script>";
    server.send(200, "text/html", webPage);
    digitalWrite(latchPin, LOW);
    SPI.transfer(out1);
    SPI.transfer(out2);
    digitalWrite(latchPin, HIGH);
    handleSwitch();
  });
  server.on("/resetNW", [](){
    /*turnOffAll();*/
    currentlabel="OFF";
    webPage += "<script> document.location.href = \"/switch\"</script>";
    server.send(200, "text/html", webPage);
    handleSwitch();
  });
    server.on("/resetAll", [](){
    turnOffAll();
    currentlabel="OFF";
    webPage += "<script> document.location.href = \"/switch\"</script>";
    server.send(200, "text/html", webPage);
    handleSwitch();
  });
  server.on("/set300", [](){
    currentRload=" " + String(label300)+ " Ohm";
    webPage += "<script> document.location.href = \"/switch\"</script>";
    server.send(200, "text/html", webPage);
    out1 = 0b00000001;
    digitalWrite(latchPin, LOW);
    SPI.transfer(out1);
    SPI.transfer(out2);
    digitalWrite(latchPin, HIGH);
    handleSwitch();
  });
  server.on("/set390", [](){
    currentRload=" " + String(label390)+ " Ohm";
    webPage += "<script> document.location.href = \"/switch\"</script>";
    server.send(200, "text/html", webPage);
    out1 = 0b00000010;
    digitalWrite(latchPin, LOW);
    SPI.transfer(out1);
    SPI.transfer(out2);
    digitalWrite(latchPin, HIGH);
    handleSwitch();
  });
  server.on("/set430", [](){
    currentRload=" " + String(label430)+ " Ohm";
    webPage += "<script> document.location.href = \"/switch\"</script>";
    server.send(200, "text/html", webPage);
    out1 = 0b00000100;
    digitalWrite(latchPin, LOW);
    SPI.transfer(out1);
    SPI.transfer(out2);
    digitalWrite(latchPin, HIGH);
    handleSwitch();
  });
  server.on("/set470", [](){
    currentRload=" " + String(label470)+ " Ohm";
    webPage += "<script> document.location.href = \"/switch\"</script>";
    server.send(200, "text/html", webPage);
    out1 = 0b00001000;
    digitalWrite(latchPin, LOW);
    SPI.transfer(out1);
    SPI.transfer(out2);
    digitalWrite(latchPin, HIGH);
    handleSwitch();
  });
  server.on("/set510", [](){
    currentRload=" " + String(label510)+ " Ohm";
    webPage += "<script> document.location.href = \"/switch\"</script>";
    server.send(200, "text/html", webPage);
    out1 = 0b00010000;
    digitalWrite(latchPin, LOW);
    SPI.transfer(out1);
    SPI.transfer(out2);
    digitalWrite(latchPin, HIGH);
    handleSwitch();
  });
  server.on("/set560", [](){
    currentRload=" " + String(label560)+ " Ohm";
    webPage += "<script> document.location.href = \"/switch\"</script>";
    server.send(200, "text/html", webPage);
    out1 = 0b00100000;
    digitalWrite(latchPin, LOW);
    SPI.transfer(out1);
    SPI.transfer(out2);
    digitalWrite(latchPin, HIGH);
    handleSwitch();
  });
  server.on("/setPa", [](){
    digitalWrite(PA_pin, HIGH);
    currentPa=" " + String(labelpa);
    webPage += "<script> document.location.href = \"/switch\"</script>";
    server.send(200, "text/html", webPage);
    handleSwitch();
  });
  server.on("/resetPa", [](){
    digitalWrite(PA_pin, LOW);
    currentPa=" ";
    webPage += "<script> document.location.href = \"/switch\"</script>";
    server.send(200, "text/html", webPage);
    handleSwitch();
  });
  server.begin();
  Serial.println("HTTP server started");
}

void connectWifi() {
  Serial.println("Connecting as wifi client...");
  WiFi.disconnect();
  WiFi.begin(ssid, password);
  int connRes = WiFi.waitForConnectResult();
  Serial.print("connRes: ");
  Serial.println(connRes);
}


 
void loop(void){
  digitalWrite(OE_pin, LOW);
  if (connect) {
    Serial.println("Connect requested");
    connect = false;
    connectWifi();
    lastConnectTry = millis();
  }
  {
    unsigned int s = WiFi.status();
    if (s == 0 && millis() > (lastConnectTry + 60000)) {
      connect = true;
    }
    if (status != s) { // WLAN status change
      Serial.print("Status: ");
      Serial.println(s);
      status = s;
      if (s == WL_CONNECTED) {
        /* Just connected to WLAN */
        Serial.println("");
        Serial.print("Connected to ");
        Serial.println(ssid);
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        digitalWrite(LED_pin, HIGH);
        flagAP=1;
      } else if (s == WL_NO_SSID_AVAIL) {
        digitalWrite(LED_pin, LOW);
        flagAP=0;
        WiFi.disconnect();
      }
    }
  }
  if (flagAP==1) 
  {
    tim++; 
    if (tim==1000) {sec++; tim=0;}
    if (sec==60) {minute++; sec=0;}
    if (minute==60) {hour++; minute=0;}
    if (hour==24) {day++; hour=0;}
  }
  if ((flagAP==0)&&(tim>0)) tim=sec=minute=hour=day=0;
  server.handleClient();
  delay(1);
}
