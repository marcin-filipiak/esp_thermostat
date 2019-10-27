

//czyszczenie tablicy//
void clear_array(char* a, int s){
  for(int x=0;x<s;x++){
    a[x]='\0';
  }
}

////////////zapis konfiguracji///////////
void write_config(){

  char z = ' ';
  char index = 0;

  clear_array(data.ssid,32);
  clear_array(data.pass,32);

  Serial.println("Podaj SSID:");
  read_uart(data.ssid);
  Serial.println("SSID:"+String(data.ssid));


  Serial.println("Podaj haslo:");
  read_uart(data.pass);
  Serial.println("haslo:"+String(data.pass));
  

  EEPROM.put(100,data);
  EEPROM.commit();
  
}

////////////wczytaj konfiguracje/////////////
void read_config(){

  EEPROM.get(100,data); 

  Serial.println(" ");
  Serial.println("SSID:"+String(data.ssid));
  Serial.println("Haslo:"+String(data.pass));
  Serial.println("Temperatura: "+String(data.temp));
  Serial.println("Histereza: "+String(data.his));
  Serial.println("Pompa ON: "+String(data.pompon));
  Serial.println("Pompa OFF: "+String(data.pompoff));
}
