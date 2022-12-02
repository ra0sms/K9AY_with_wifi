/*
K9AY loop control box
ESP12 module
Irkutsk, December 2022
ra0sms@bk.ru
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
const char *softAP_ssid = "K9AY_Loop_sn001";
const char *softAP_password = "1234567890";
const char *myHostname = "esp8266";
//*********************************************
void buildXML ();
void saveLabels ();
void saveCredentials ();
void CheckStat ();
void handleXML ();
void BuildSVG ();
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
String currentPa ="";
String webPage = "";
String webPage2 = "";
String ourPage = "";
String Page3="";
String javaScript, XML, SVG;
//********************************************* 
int PA_pin = 4;
int LED_pin = 5;
int OE_pin = 12;
/***SPI*/
int latchPin = 15;
int clockPin = 14;
int dataPin = 13;
/*******/
int gpio6_pin = 12;
int gpio7_pin = 13;
int gpio8_pin = 15;
char stat[8] =  "";
int tim = 0;
int sec = 0;
int minute = 0;
int hour = 0;
int day = 0;
int flagAP=0;
int flag_off=0;
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

void BuildSVG()
{
 SVG = "<svg version=\"1.0\" xmlns=\"http://www.w3.org/2000/svg\"";
 SVG +=F("width=\"99.000000pt\" height=\"35.000000pt\" viewBox=\"0 0 790.000000 284.000000\""
 "preserveAspectRatio=\"xMidYMid meet\">"
"<g transform=\"translate(0.000000,284.000000) scale(0.100000,-0.100000)\""
"fill=\"#000000\" stroke=\"none\">"
"<path d=\"M0 1420 l0 -1420 910 0 910 0 0 1420 0 1420 -910 0 -910 0 0 -1420z "
"m1620 0 l0 -1220 -710 0 -710 0 0 1220 0 1220 710 0 710 0 0 -1220z\"/>"
"<path d=\"M392 1983 l3 -458 103 -3 102 -3 0 161 0 160 270 0 c176 0 277 -4 "
"293 -11 39 -18 47 -50 47 -184 l0 -125 106 0 106 0 -4 153 c-3 173 -15 206 "
"-80 241 l-35 20 31 21 c69 46 101 130 93 241 -8 110 -54 181 -142 221 -36 16 "
"-81 18 -468 21 l-427 3 2 -458z m789 244 c48 -37 53 -115 10 -158 l-29 -29 "
"-281 0 -281 0 0 100 0 100 283 0 c206 -1 286 -4 298 -13z\"/>"
"<path d=\"M394 1294 c-4 -14 -4 -164 -1 -332 4 -275 7 -314 25 -372 25 -76 71 "
"-127 147 -162 49 -23 57 -23 453 -26 l402 -3 0 460 0 461 -105 0 -105 0 0 "
"-366 0 -366 -258 4 c-375 5 -350 -27 -351 451 l-1 277 -100 0 c-99 0 -100 0 "
"-106 -26z\"/>"
"<path d=\"M2210 1980 l0 -460 100 0 100 0 0 315 c0 173 3 315 6 315 4 0 62 -52 "
"131 -116 123 -116 124 -117 145 -98 12 11 69 63 127 117 l106 98 3 -315 2 "
"-316 100 0 100 0 0 460 0 460 -56 0 c-96 0 -129 -16 -233 -114 -53 -50 -113 "
"-106 -133 -125 l-38 -35 -119 117 c-138 135 -175 156 -273 157 l-68 0 0 -460z\"/>"
"<path d=\"M3530 2433 c-102 -17 -166 -63 -197 -143 -15 -39 -17 -93 -21 -407 "
"l-4 -363 106 0 106 0 0 155 0 155 250 0 250 0 0 -155 0 -155 105 0 105 0 0 "
"460 0 460 -337 -1 c-186 -1 -349 -4 -363 -6z m490 -303 l0 -110 -250 0 -250 0 "
"1 78 c1 142 -1 141 282 142 l217 0 0 -110z\"/>"
"<path d=\"M4410 1980 l0 -461 353 3 c400 5 420 8 481 79 55 64 61 104 61 379 0 "
"275 -6 315 -61 379 -61 71 -81 74 -481 79 l-353 3 0 -461z m644 249 c14 -7 30 "
"-25 36 -42 13 -39 13 -375 0 -414 -17 -48 -41 -53 -265 -53 l-205 0 0 260 0 "
"260 205 0 c131 0 213 -4 229 -11z\"/>"
"<path d=\"M5680 2433 c-106 -18 -176 -72 -200 -156 -17 -60 -25 -449 -11 -533 "
"15 -83 41 -136 83 -167 66 -48 84 -51 426 -55 l322 -3 0 100 0 101 -292 0 "
"c-161 0 -298 4 -305 8 -19 12 -33 60 -33 113 l0 49 250 0 250 0 0 95 0 95 "
"-250 0 -250 0 0 44 c0 49 15 96 33 108 7 4 137 8 290 8 l277 0 0 100 0 100 "
"-282 -1 c-156 -1 -294 -4 -308 -6z\"/>"
"<path d=\"M6580 1980 l0 -460 105 0 105 0 0 460 0 460 -105 0 -105 0 0 -460z\"/>"
"<path d=\"M6980 1980 l0 -460 100 0 100 0 2 302 3 302 101 -125 c55 -68 140 "
"-173 188 -234 173 -216 202 -236 339 -243 l87 -4 0 461 0 461 -100 0 -100 0 0 "
"-301 c0 -187 -4 -298 -9 -294 -9 5 -48 52 -310 377 -56 70 -119 141 -139 158 "
"-49 40 -112 60 -194 60 l-68 0 0 -460z\"/>"
"<path d=\"M2217 1314 c-4 -4 -7 -211 -7 -461 l0 -453 105 0 105 0 0 161 0 160 "
"226 -3 226 -3 24 -28 c23 -27 24 -35 24 -158 l0 -129 105 0 105 0 0 143 c0 "
"166 -14 213 -75 248 l-36 22 34 23 c19 13 45 45 58 71 20 40 24 63 24 143 0 "
"105 -16 149 -72 202 -62 59 -84 62 -478 66 -198 2 -364 0 -368 -4z m672 -209 "
"c34 -17 41 -33 41 -89 0 -33 -6 -49 -26 -70 l-27 -26 -228 0 -229 0 0 100 0 "
"100 220 0 c174 0 227 -3 249 -15z\"/>"
"<path d=\"M3310 992 c0 -191 5 -353 11 -388 16 -90 48 -137 116 -171 l58 -28 "
"368 -3 367 -3 0 460 0 461 -105 0 -105 0 0 -366 0 -366 -217 4 c-120 3 -226 8 "
"-236 12 -44 20 -47 47 -47 391 l0 325 -105 0 -105 0 0 -328z\"/>"
"<path d=\"M4625 1315 c-5 -2 -26 -6 -46 -9 -54 -9 -126 -62 -157 -115 -25 -42 "
"-27 -55 -27 -146 0 -83 4 -107 22 -140 28 -52 57 -79 118 -107 47 -22 63 -23 "
"295 -26 135 -1 255 -5 268 -7 62 -14 82 -90 38 -142 l-24 -28 -336 -3 -336 -3 "
"0 -95 0 -95 368 3 c349 3 369 4 412 24 101 47 145 124 145 254 0 137 -51 217 "
"-164 262 -51 19 -79 22 -306 25 -289 5 -295 6 -295 77 0 28 6 48 17 57 12 11 "
"86 15 342 19 l326 5 3 75 c2 41 0 85 -3 97 l-5 23 -323 -1 c-177 0 -326 -2 "
"-332 -4z\"/>"
"<path d=\"M5653 1306 c-130 -32 -201 -138 -191 -287 6 -82 46 -161 99 -196 70 "
"-46 116 -53 349 -53 126 0 230 -4 247 -10 49 -17 68 -84 38 -127 -26 -37 -76 "
"-43 -377 -43 l-288 0 0 -95 0 -95 322 0 c370 0 407 5 482 72 58 50 80 109 81 "
"208 0 93 -17 141 -67 196 -71 77 -112 86 -397 92 -221 4 -245 6 -262 23 -26 "
"26 -25 83 1 109 19 19 33 20 320 20 l300 0 0 100 0 100 -302 -1 c-207 0 -319 "
"-5 -355 -13z\"/>"
"<path d=\"M6580 860 l0 -460 105 0 105 0 0 460 0 460 -105 0 -105 0 0 -460z\"/>"
"<path d=\"M7189 1310 c-93 -16 -158 -67 -190 -150 -10 -26 -14 -125 -17 -397 "
"l-3 -363 105 0 106 0 0 155 0 155 250 0 250 0 0 -155 0 -155 105 0 105 0 0 "
"460 0 460 -332 -1 c-183 -1 -354 -5 -379 -9z m501 -300 l0 -110 -250 0 -250 0 "
"0 75 c0 82 12 118 44 133 12 6 111 11 239 11 l217 1 0 -110z\"/>"
"</g>"
"</svg>"); 
}

void buildJavascript() {
  javaScript = "<SCRIPT>\n";
  javaScript += "var xmlHttp=createXmlHttpObject();\n";

  javaScript += "function createXmlHttpObject(){\n";
  javaScript += " if(window.XMLHttpRequest){\n";
  javaScript += "    xmlHttp=new XMLHttpRequest();\n";
  javaScript += " }else{\n";
  javaScript += "    xmlHttp=new ActiveXObject('Microsoft.XMLHTTP');\n";
  javaScript += " }\n";
  javaScript += " return xmlHttp;\n";
  javaScript += "}\n";

  javaScript += "function process(){\n";
  javaScript += " if(xmlHttp.readyState==0 || xmlHttp.readyState==4){\n";
  javaScript += "   xmlHttp.open('PUT','xml',true);\n";
  javaScript += "   xmlHttp.onreadystatechange=handleServerResponse;\n";
  javaScript += "   xmlHttp.send(null);\n";
  javaScript += " }\n";
  javaScript += " setTimeout('process()',0.1);\n";
  javaScript += "}\n";

  javaScript += "function handleServerResponse(){\n";
  javaScript += " if(xmlHttp.readyState==4 && xmlHttp.status==200){\n";
  javaScript += "   xmlResponse=xmlHttp.responseXML;\n";
  javaScript += "   xmldoc = xmlResponse.getElementsByTagName('response');\n";
  javaScript += "   message = xmldoc[0].firstChild.nodeValue;\n";
  javaScript += "   document.getElementById('runtime').innerHTML=message;\n";
  javaScript += " }\n";
  javaScript += "}\n";

  javaScript += "</SCRIPT>\n";
}

void buildXML() {
 XML = "<?xml version='1.0'?>";
  XML += "<response>";
  XML += currentlabel + " " + currentRload + " " + currentPa;
  XML += "</response>";
}


void handleSwitch ()
{
  buildJavascript();
  webPage = "<!DOCTYPE HTML> <html> <head> <title>K9AY control</title>";
  /*webPage += "<meta http-equiv=\'refresh\' content=\'3\'>";*/
  webPage += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
  webPage += "<style type=\"text/css\">";
  webPage += ".btn {background-color: #4CAF50; border: none; color: white; padding: 8px 16px; text-align: center; text-decoration: none; display: inline-block; font-size: 13px; margin: 1px 2px; -webkit-transition-duration: 0.4s; transition-duration: 0.4s; cursor: pointer;}";
  webPage += ".btn-on {background-color: white; color: black; border: 3px solid #4CAF50;}";
  webPage += ".btn-off {background-color: white; color: black; border: 3px solid #f44336;}";
  webPage += ".btn-on:active {background: green;}";
  webPage += ".btn-off:active {background: red;}";
  webPage += "</style> </head>";
  webPage += javaScript;
  webPage += "<body bgcolor=\"#c7daed\">";
  webPage += "<BODY onload='process()'>";
  webPage += "<p><font color=\"red\" face=\"Arial\"><h2>&nbsp<A id='runtime'> </A></font></h2></p>";
  webPage += "</BODY>";
  webPage += "<p><h3>&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp<a href=\"setN\"><button class=\"btn btn-on\">N&nbsp</button></a>";
  webPage += "<p><h3>&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp<a href=\"setNW\"><button class=\"btn btn-on\">NW</button></a>&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp<a href=\"setNE\"><button class=\"btn btn-on\">NE</button></a>";
  webPage += "<p><h3><a href=\"setW\"><button class=\"btn btn-on\">W&nbsp</button></a>&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp<a href=\"setE\"><button class=\"btn btn-on\">E&nbsp</button></a>";
  webPage += "<p><h3>&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp<a href=\"setSW\"><button class=\"btn btn-on\">SW</button></a>&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp<a href=\"setSE\"><button class=\"btn btn-on\">SE</button></a>";
  webPage += "<p><h3>&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp<a href=\"setS\"><button class=\"btn btn-on\">S&nbsp</button></a>";
  webPage += "<p><a href=\"resetAll\"><button class=\"btn btn-off\">OFF</button></a></h3></p>";
  webPage += "<p><h2>Rload: <a href=\"set300\"><button class=\"btn btn-on\">300</button></a>&nbsp<a href=\"set390\"><button class=\"btn btn-on\">390</button></a>&nbsp<a href=\"set430\"><button class=\"btn btn-on\">430</button></a>&nbsp<a href=\"set470\"><button class=\"btn btn-on\">470</button></a>&nbsp<a href=\"set510\"><button class=\"btn btn-on\">510</button></a>&nbsp<a href=\"set560\"><button class=\"btn btn-on\">560</button></a></h2></p>";
  webPage += "<p><h2>Preamp +15dB:<a href=\"setPa\"><button class=\"btn btn-on\">ON</button></a>&nbsp<a href=\"resetPa\"><button class=\"btn btn-off\">OFF</button></a></h2></p>";
  webPage += "<p><a href ='/'>Return to the home page</a></p>";  
  server.send(200, "text/html", webPage);
  }

void handleRoot() {
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
 /* server.send(200, "text/html", "");*/
 String Page;
 BuildSVG();
  Page =F(
    "<!DOCTYPE HTML> <html><head><title>K9AY control</title></head><body>"
    "<h1>K9AY RX Loop</h1>"
  );
  if (server.client().localIP() == apIP) {
    Page += (String("<p>You are connected through the soft AP: ") + softAP_ssid + "</p>");
  } else {
    Page += (String("<p>You are connected through the wifi network: ") + ssid + "</p>");
  }
  Page += F(
    "<p><a href='/wifi'>Config the wifi connection</a></p>"
    "<p><a href='/switch'>Switch directions</a></p>");
    Page += SVG;
    Page +="</body></html>";
 
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
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  /*server.setContentLength(CONTENT_LENGTH_UNKNOWN);*/
   

  Page3 = F(
    "<html><head></head><body>"
    "<h1>Wifi config</h1>"
  );
  if (server.client().localIP() == apIP) {
    Page3 +=(String("<p>You are connected through the soft AP: ") + softAP_ssid + "</p>");
  } else {
    Page3 +=(String("<p>You are connected through the wifi network: ") + ssid + "</p>");
    
  }
  Page3 +=("<p>Uptime - " + String(day) + " day " + String(hour) + " h " + String(minute) + " m " + String(sec) + " s ""</p>");
  Page3 += ("<p><h3><a href=\"APon\"><button class=\"btn btn-on\">ON</button></a>&nbsp;<a href=\"APoff\"><button class=\"btn btn-off\">OFF</button></a> Turn on/off Access Point</h3></p>");
  if (flag_off==1) Page3 += ("<p><h3> AP OFF</h3></p>");
  if (flag_off==0) Page3 += ("<p><h3> AP ON</h3></p>");
  Page3 += F(
    "\r\n<br />"
    "<table><tr><th align='left'>SoftAP config</th></tr>"
  );
  Page3 +=(String() + "<tr><td>SSID " + String(softAP_ssid) + "</td></tr>");
  Page3 +=(String() + "<tr><td>IP " + WiFi.softAPIP().toString() + "</td></tr>");
 
  Page3 += F(
    "</table>"
    "\r\n<br />"
    "<table><tr><th align='left'>WLAN config</th></tr>"
  );
  Page3 +=(String() + "<tr><td>SSID " + String(ssid) + "</td></tr>");
  Page3 +=(String() + "<tr><td>IP " + WiFi.localIP().toString() + "</td></tr>");
  Page3 += F(
    "</table>"
    "\r\n<br />"
    "<table><tr><th align='left'>WLAN list (refresh if any missing)</th></tr>"
  );
  Serial.println("scan start");
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n > 0) {
    for (int i = 0; i < n; i++) {
      Page3 +=(String() + "\r\n<tr><td>SSID " + WiFi.SSID(i) + String((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : " *") + " (" + WiFi.RSSI(i) + ")</td></tr>");
    }
  } else {
    Page3 +=(String() + "<tr><td>No WLAN found</td></tr>");
  }
  Page3 += F(
    "</table>"
    "\r\n<br /><form method='POST' action='wifisave'><h4>Connect to network:</h4>"
    "<input type='text' placeholder='network' name='n'/>"
    "<br /><input type='password' placeholder='password' name='p'/>"
    "<br /><input type='submit' value='Connect/Disconnect'/></form>"
    "<p><a href='/'>Return to the home page</a>.</p>"
    "</body></html>"
  );
  /*server.client().stop(); */
  server.send(200, "text/html", Page3);
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
  
}
 
void setup(void){
  delay(100);
  SPI.begin();
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
  pinMode(gpio6_pin, OUTPUT);
  digitalWrite(gpio6_pin, LOW);
  pinMode(gpio7_pin, OUTPUT);
  digitalWrite(gpio7_pin, LOW);
  pinMode(gpio8_pin, OUTPUT);
  digitalWrite(gpio8_pin, LOW);

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
    currentlabel=String(label0);
 
    webPage += "<script> document.location.href = \"/switch\"</script>";
    server.send(200, "text/html", webPage);
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
    currentlabel=String(label1);
    
    webPage += "<script> document.location.href = \"/switch\"</script>";
    server.send(200, "text/html", webPage);
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
   
    currentlabel=String(label2);
    webPage += "<script> document.location.href = \"/switch\"</script>";
    server.send(200, "text/html", webPage);
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
   
    currentlabel=String(label3);
    webPage += "<script> document.location.href = \"/switch\"</script>";
    server.send(200, "text/html", webPage);
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
 
    currentlabel=String(label4);
    webPage += "<script> document.location.href = \"/switch\"</script>";
    server.send(200, "text/html", webPage);
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
 
    currentlabel=String(label5);
    webPage += "<script> document.location.href = \"/switch\"</script>";
    server.send(200, "text/html", webPage);
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
  
    currentlabel=String(label6);
    webPage += "<script> document.location.href = \"/switch\"</script>";
    server.send(200, "text/html", webPage);
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
 
    currentlabel=String(label7);
    webPage += "<script> document.location.href = \"/switch\"</script>";
    server.send(200, "text/html", webPage);
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
    handleSwitch();
  });
  server.on("/set390", [](){
    currentRload=" " + String(label390)+ " Ohm";
    webPage += "<script> document.location.href = \"/switch\"</script>";
    server.send(200, "text/html", webPage);
    handleSwitch();
  });
  server.on("/set430", [](){
    currentRload=" " + String(label430)+ " Ohm";
    webPage += "<script> document.location.href = \"/switch\"</script>";
    server.send(200, "text/html", webPage);
    handleSwitch();
  });
  server.on("/set470", [](){
    currentRload=" " + String(label470)+ " Ohm";
    webPage += "<script> document.location.href = \"/switch\"</script>";
    server.send(200, "text/html", webPage);
    handleSwitch();
  });
  server.on("/set510", [](){
    currentRload=" " + String(label510)+ " Ohm";
    webPage += "<script> document.location.href = \"/switch\"</script>";
    server.send(200, "text/html", webPage);
    handleSwitch();
  });
  server.on("/set560", [](){
    currentRload=" " + String(label560)+ " Ohm";
    webPage += "<script> document.location.href = \"/switch\"</script>";
    server.send(200, "text/html", webPage);
    handleSwitch();
  });
  server.on("/setPa", [](){
    currentPa=" " + String(labelpa);
    webPage += "<script> document.location.href = \"/switch\"</script>";
    server.send(200, "text/html", webPage);
    handleSwitch();
  });
  server.on("/resetPa", [](){
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
        digitalWrite(gpio8_pin, HIGH);
        flagAP=1;
      } else if (s == WL_NO_SSID_AVAIL) {
        digitalWrite(gpio8_pin, LOW);
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
