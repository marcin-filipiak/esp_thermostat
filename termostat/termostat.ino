//PLYTKA NodeMCU

#include <OneWire.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h> //https://tttapa.github.io/ESP8266/Chap10%20-%20Simple%20Web%20Server.html
#include <DallasTemperature.h>

struct {
  char temp[11] = "";  //temperatura
  char his[11] = "";   //histereza
  char pompon[11] = ""; //wlaczenie pompy
  char pompoff[11] = ""; //wylaczenie pompy
  char ssid[32] = "";  //nazwa sieci wifi
  char pass[32] = "";  //haslo sieci wifi
} data;

int settemp = 26; //zmienna int zadanej temperatury
int sethis = 2; //zmienna int histerezy
int setpompon = 40; //zmienna wlaczenia pompy
int setpompoff = 38; //zmienna wylaczenia pompy
int ticker = 0;   //zmienna licznika czasu

//porownanie dwoch float czy sa sobie rowne
bool comparison_float(float a, float b) {
  if (fabsf(a - b) < 0.0001f) {
    return true;
  }
  else {
    return false;
  }
}

////pobranie danych z uart do tablicy////
void read_uart(char *r) {
  char z = ' ';
  char index = 0;

  do {
    if (Serial.available() > 0) {
      z = Serial.read();
      if (z != '\n') {
        r[index] = z;
        index ++;
      }
    }
  } while (z != '\n');
  r[index + 1] = '\0';
}

#include <EEPROM.h>
#include "include/wifi.cpp"
#include "include/config.cpp"

#define ONE_WIRE_BUS D1
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);


#define pinHeater D2
#define pinPomp D3

ESP8266WebServer server(80);

////////////menu///////////////////////
void help() {
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("s - skanuj sieci");
  Serial.println("c - konfiguruj siec");
  Serial.println("o - czytaj konfiguracje");
  Serial.println("j - polacz z siecia");
  Serial.println("h - wyswietl to menu");
}

////////pobranie temperatury///////
float getTemperature() {
  float temp;

  do {
    DS18B20.requestTemperatures();
    temp = DS18B20.getTempCByIndex(0);
    delay(100);
  } while (comparison_float(temp, 85.0) || comparison_float(temp, (-127.0)));

  return temp;
}

///////strona glowna//////////////
void handleRoot() {
  float t = getTemperature();
  String html = "<html>";
  html += "<head>";
  html += "<link rel=\"stylesheet\" href=\"https://code.jquery.com/ui/1.12.1/themes/base/jquery-ui.css\">";
  html += "<script src=\"https://code.jquery.com/jquery-1.12.4.js\"></script>";
  html += "<script src=\"https://code.jquery.com/ui/1.12.1/jquery-ui.js\"></script>";
  html += "<script>$( function() {$( \"#a\" ).accordion(); } );</script>";
  html += "</head>";
  html += "<body>";
  html += "<div id=\"a\">";
  html += "<h3>Stan</h3>";
  html += "<div>";
  html += String(t) + "&deg;C";
  if (digitalRead(pinHeater) == true) {
    html += "<br/>Grzalka ON";
  }
  else {
    html += "<br/>Grzalka OFF";
  }
  if (digitalRead(pinPomp) == true) {
    html += "<br/>Pompa ON";
  }
  else {
    html += "<br/>Pompa OFF";
  }
  html += "</div>";
  html += "<h3>Konfiguracja</h3>";
  html += "<div>";
  html += "<form method=\"post\" action=\"SET\">";
  html += "<table>";
  html += "<tr>";
  html += "<td>Temperatura:</td><td><input type=\"text\" name=\"sett\" value=\"" + String(settemp) + "\" size=\"4\"/>&deg;C</td>";
  html += "<td>Histereza:</td><td><input type=\"text\" name=\"seth\" value=\"" + String(sethis) + "\" size=\"4\"/>&deg;C</td>";
  html += "</tr>";
  html += "<tr>";
  html += "<td>Pompa OFF:</td><td><input type=\"text\" name=\"setpompoff\" value=\"" + String(setpompoff) + "\" size=\"4\"/>&deg;C</td>";
  html += "<td>Pompa ON:</td><td><input type=\"text\" name=\"setpompon\" value=\"" + String(setpompon) + "\" size=\"4\"/>&deg;C</td>";
  html += "</tr>";
  html += "</table>";
  html += "<input type=\"submit\" value=\"Zapisz\"/>";
  html += "</form>";
  html += "</div>";
  html += "</div>";
  html += "</body>";
  html += "</html>";
  server.send ( 200, "text/html", html );
}

/////zapisanie ustawien//////////
void handleSet() {

  //pobranie z posta zmiennej temperatury
  settemp = (server.arg("sett")).toInt();
  //pobranie z posta zmiennej histerezy
  sethis = (server.arg("seth")).toInt();

  //pobranie z posta zmiennych wlaczenia pompy
  setpompon = (server.arg("setpompon")).toInt();
  setpompoff = (server.arg("setpompoff")).toInt();

  //dane dla eeprom temperatury
  String s =   server.arg("sett");
  clear_array(data.temp, 11);
  s.toCharArray(data.temp, 5);

  //dane dla eeprom histerezy
  String h =   server.arg("seth");
  clear_array(data.his, 11);
  h.toCharArray(data.his, 5);

  //dane dla eeprom pompy
  String pn =   server.arg("setpompon");
  clear_array(data.pompon, 11);
  pn.toCharArray(data.pompon, 5);

  String pf =   server.arg("setpompoff");
  clear_array(data.pompoff, 11);
  pf.toCharArray(data.pompoff, 5);

  //zapisanie do eeprom ustawien
  EEPROM.put(100, data);
  EEPROM.commit();

  server.sendHeader("Location", "/");
  server.send(303);
}

/////////ustawienia esp//////////
void setup() {
  Serial.begin(115200);
  EEPROM.begin(512); //ile pamieci

  read_config();
  //TODO: laczyc tylko jesli SSID podane
  connect_wifi();
  help();

  //odczyt zapisanej temperatury
  settemp = (String(data.temp)).toInt();

  //odczyt zapisanej histerezy
  sethis = (String(data.his)).toInt();

  setpompon = (String(data.pompon)).toInt();
  setpompoff = (String(data.pompoff)).toInt();

  //grzalka
  pinMode(pinHeater, OUTPUT);
  digitalWrite(pinHeater, LOW);

  //pompa
  pinMode(pinPomp, OUTPUT);
  digitalWrite(pinPomp, LOW);

  server.on("/", handleRoot);
  server.on("/SET", HTTP_POST, handleSet);
  server.begin();
}


//////petla glowna///////
void loop() {
  server.handleClient();

  //jesli przyszly dane
  if (Serial.available() > 0) {
    char z[3];
    read_uart(z);

    //skanowanie sieci
    if (z[0] == 's') {
      scan();
    }
    //konfiguruj siec
    if (z[0] == 'c') {
      write_config();
    }
    //czytaj konfiguracje
    if (z[0] == 'o') {
      read_config();
    }
    //polacz
    if (z[0] == 'j') {
      connect_wifi();
    }
    //help
    if (z[0] == 'h') {
      help();
    }
  }

  delay(100);
  ticker ++;
  if (ticker > 100) {
    ticker = 0;

    float tn = getTemperature();

    //jesli tempertatura nizsza od zadanej-histereza wlacz grzanie
    if (tn < (settemp - sethis)) {
      digitalWrite(pinHeater, HIGH);
    }
    //jesli osiagnieto zadana temperature wylacz grzanie
    if (tn > settemp) {
      digitalWrite(pinHeater, LOW);
    }

    //jesli tempertatura wyzsza od zadanej wlacz pompke
    if (tn >= setpompon) {
      digitalWrite(pinPomp, HIGH);
    }
    //jesli temperatura nizsza od zadanej wylacz pompke
    if (tn <= setpompoff) {
      digitalWrite(pinPomp, LOW);
    }

  }

}
