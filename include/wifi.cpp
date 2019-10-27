////////////////skanowanie sieci/////////////////
void scan () {

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  
  Serial.println("scan start");

  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0)
    Serial.println("");
  else
  {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i)
    {
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE)?" ":"*");
      delay(10);
    }
  }
  Serial.println("");
}

////////////polaczenie z wifi/////////////////
void connect_wifi(){
  
      char x=0;
      WiFi.begin(data.ssid, data.pass);
      
      Serial.print("Connecting to "+ String(data.ssid) +"\n");
      
      while (WiFi.status() != WL_CONNECTED && x<7) {
        Serial.print("*");
        delay(1000);
        x++;
      }
      if (x<7 && WiFi.status() == WL_CONNECTED){
        Serial.print("\nConnected\n");
      }
      else {
        Serial.print("\nConnection Error\n");
      }
      delay(2000);
}
