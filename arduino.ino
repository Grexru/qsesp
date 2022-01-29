/*
 * din - цифровой вход, dout- цифровой выход
 * dht - для датчика температуры и влажности
 * sda- для дисплея, slc - для дисплея
 * ursin - ультрозвуковой датчик вход, ursout - ультрозвуковой датчик выход
 */


#include "ESP8266WiFi.h"
#include "ESP8266WebServer.h"
#include <EEPROM.h>
#include <ArduinoJson.h>
#include "Wire.h"
#include "LiquidCrystal_I2C.h"
#include "DHTesp.h"


ESP8266WebServer server(80);
LiquidCrystal_I2C lcd(0x27, 16, 2);
DHTesp dht;


long duration;
int distance;

String html = "";


const char* dig_pin[] = {"0","0","0","0","0","0","0","0","0","0","0","0","0","0","0","0","0"};
const char* dig_pin_out[] = {"0","0","0","0","0","0","0","0","0","0","0","0","0","0","0","0","0"};
int max_dig_pin = sizeof(dig_pin) / sizeof(dig_pin[0])-1;

int str_len = 0;
char char_array[5];

StaticJsonDocument<2000> data_eeprom;

const char* wifi_ssid="device";
const char* wifi_pass="123qweasdzxc";
const char* wifi_ip_s="";
const char* wifi_gateway_s="";
const char* wifi_subnet_s="";


 
void setup() {
  Serial.begin(9600);
  EEPROM.begin(2000);

  DeserializationError error = deserializeJson(data_eeprom, readEEPROM(0));
   if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
  }
  else
  {
    wifi_ssid = data_eeprom["ssid"];
    wifi_pass = data_eeprom["password"];
    wifi_ip_s = data_eeprom["local_ip"];
    wifi_gateway_s = data_eeprom["gateway"];
    wifi_subnet_s = data_eeprom["subnet"];

    IPAddress ip(ExplodeValue(wifi_ip_s,'.',0).toInt(),ExplodeValue(wifi_ip_s,'.',1).toInt(),ExplodeValue(wifi_ip_s,'.',2).toInt(),ExplodeValue(wifi_ip_s,'.',3).toInt());  //статический IP
    IPAddress gateway(ExplodeValue(wifi_gateway_s,'.',0).toInt(),ExplodeValue(wifi_gateway_s,'.',1).toInt(),ExplodeValue(wifi_gateway_s,'.',2).toInt(),ExplodeValue(wifi_gateway_s,'.',3).toInt());
    IPAddress subnet(ExplodeValue(wifi_subnet_s,'.',0).toInt(),ExplodeValue(wifi_subnet_s,'.',1).toInt(),ExplodeValue(wifi_subnet_s,'.',2).toInt(),ExplodeValue(wifi_subnet_s,'.',3).toInt());
    WiFi.config(ip, gateway, subnet); 

    
    
    for(int i = 0; i<=max_dig_pin; i++)
    {
      dig_pin[i] = data_eeprom["pin_"+String(i)];      
    }
  }


  
  
  

  for(int i = 0; i<=max_dig_pin; i++)
  {

    if(String(dig_pin[i]) == "dout" || String(dig_pin[i]) == "ursout")
    {
      pinMode(i, OUTPUT); 
      digitalWrite(i, 0);
      Serial.println(String(i)+" OUTPUT");
    }

    if(String(dig_pin[i]) == "din" || String(dig_pin[i]) == "ursin")
    {
      pinMode(i, INPUT);
      Serial.println(String(i)+" INPUT");
    }


    if(String(dig_pin[i]) == "sda")
    {
      for(int d = 0; d<=max_dig_pin; d++)
      {
        if(String(dig_pin[d]) == "slc")
        {
          Wire.begin(i, d);
          lcd.begin();
          lcd.home();
          lcd.print(""); 
        }
        
      }
    }

    if(String(dig_pin[i]) == "dht")
    {
      #define DHTpin i
      dht.setup(DHTpin, DHTesp::DHT11);
    }
  }

  
 
  
  WiFi.begin(wifi_ssid, wifi_pass);  //Connect to the WiFi network
  Serial.println("");
  Serial.println("Waiting to connect");
  while (WiFi.status() != WL_CONNECTED) {  //Wait for connection
 
    delay(500);
    Serial.print(".");
 
  }
  Serial.println("");
  
  Serial.print("Wifi ssid: ");
  Serial.println(wifi_ssid);  
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  
  server.begin(); 


  
  
 
   
  server.on("/setting", setting_esp);    
  server.on("/clear_eeprom", clearEEPROM);            
  server.on("/", in);   
  server.on("/out", out);

  
  
 
}
 
void loop() {
  server.handleClient();  

}


void setting_esp()
{

  
  html = html+"<!DOCTYPE html>";
  html = html+"<html>";
    html = html+"<body>";
      html = html+"<form action='/setting'>";
          html = html+"SETTING WI-FI<br>";
          
          html = html+"<table>";
            html = html+"<tr>";
              html = html+"<td>ssid:</td>";
              html = html+"<td><input type='text' name='ssid' value='"+wifi_ssid+"'></td>";
            html = html+"</tr>";
            html = html+"<tr>";
              html = html+"<td>password:";
              html = html+"<td><input type='text' name='password' value='"+wifi_pass+"'></td>";
            html = html+"</tr>";
            html = html+"<tr>";
              html = html+"<td>local ip:";
              html = html+"<td><input type='text' name='local_ip' value='"+wifi_ip_s+"'></td>";
            html = html+"</tr>";
            html = html+"<tr>";
              html = html+"<td>gateway:";
              html = html+"<td><input type='text' name='gateway' value='"+wifi_gateway_s+"'></td>";
            html = html+"</tr>";
            html = html+"<tr>";
              html = html+"<td>subnet:";
              html = html+"<td><input type='text' name='subnet' value='"+wifi_subnet_s+"'></td>";
            html = html+"</tr>";
        html = html+"</table>";
              
        html = html+"SETTING PIN<br>";
      
          for(int i = 0; i<=max_dig_pin; i++)
          {
            html = html+"<label><input type='text' name='pinmod_"+String(i)+"' value='"+String(dig_pin[i])+"'>pin "+String(i)+"</label><br>";
            
          } 

              

      html = html+"<input type='submit' value='Submit'>";
      html = html+"</form>";
      
      html = html+"VIEW SETTING<br>";
      html = html+readEEPROM(0);
    html = html+"</body>";
  html = html+"</html>";


  
  server.send(200, "text/html", html);
  html = "";

  if(String(server.arg("ssid"))!="")
  {
    data_eeprom["ssid"] = String(server.arg("ssid"));
    data_eeprom["password"] = String(server.arg("password")); 
    data_eeprom["local_ip"] = String(server.arg("local_ip"));
    data_eeprom["gateway"]  = String(server.arg("gateway"));
    data_eeprom["subnet"]  = String(server.arg("subnet"));
    
    for(int i=0; i<=max_dig_pin; i++)
    {
      if(server.arg("pinmod_"+String(i)))
      {
        data_eeprom["pin_"+String(i)] = server.arg("pinmod_"+String(i));
      }
    }
  
   
    
     char buffer[500]; 
     serializeJsonPretty(data_eeprom, buffer);
     writeEEPROM(0,String(buffer));
  }
   
   
   
   
  
}




 
void in() {
  
  
  for(int i = 0; i<=max_dig_pin; i++)
  {
    if(server.arg("pin_"+String(i)) != false)
    {

      //Отправка данных на цифровой пин
      if( String(dig_pin[i]) == "din" || String(dig_pin[i]) == "dout")
      {
        digitalWrite(i, server.arg("pin_"+String(i)).toInt());  
      }
      //Конец Отправка данных на цифровой пин

      //Отправка данных на дисплей
      if(String(dig_pin[i]) == "sda")
      {
         lcd.clear();
         lcd.home();
         lcd.print(server.arg("pin_"+String(i))); 
      }
      //Конец Отправка данных на дисплей
      


     
      str_len = server.arg("pin_"+String(i)).length() + 1; 
      char_array[str_len];
      server.arg("pin_"+String(i)).toCharArray(char_array, str_len);
      dig_pin_out[i] = char_array;
      
    }

    
  }

  
  server.send(200, "text/plain", "in"); 
                                  
}

void out() {

  for(int i = 0; i<=max_dig_pin; i++)
  { 

    //Обработка цифровых входов
    if(String(dig_pin[i]) == "din")
    {
      if (digitalRead(i) == HIGH) 
      {
        dig_pin_out[i] = "1";
      }
      if (digitalRead(i) == LOW) 
      {
        dig_pin_out[i] = "0";
      }
    }
    //Конец Обработка цифровых входов


    //Обработка датчика температуры и влажности
    if(String(dig_pin[i]) == "dht")
    {
       String temp = "";
       float humidity = dht.getHumidity();
       float temperature = dht.getTemperature();
       temp = String(humidity)+","+String(temperature);
       str_len = temp.length() + 1; 
       char_array[str_len];
       temp.toCharArray(char_array, str_len);
       dig_pin_out[i] = char_array;
       
    }
    //Конец Обработка датчика температуры и влажности


    //Обработка ультрозвуклвлго датчика дистанции НЕ РАБОТАЕТ
    if(String(dig_pin[i]) == "ursin")
    {
       for(int d = 0; d<=max_dig_pin; d++)
       {
          if(String(dig_pin[d]) == "ursout")
          { 
            long duration;
            int distance;
            digitalWrite(d, LOW);
            delayMicroseconds(2);
            digitalWrite(d, HIGH);
            delayMicroseconds(10);
            digitalWrite(d, LOW);
            duration = pulseIn(i, HIGH);
            distance= duration*0.034/2;
          
          }
       }
      
    }
    //Конец Обработка ультрозвуклвлго датчика дистанции
      

  }
  
  

             
  html = "{";
  for(int i = 0; i<=max_dig_pin; i++)
  {
      if(html == "{")
      {
        html = html+"\""+String(i)+"\":\""+String(dig_pin_out[i])+"\"";
      }
      else
      {
        html = html+",\""+String(i)+"\":\""+String(dig_pin_out[i])+"\"";
      }
  }
  html = html+"}";
  server.send(200, "text/plain", html); 
  html = "";                         
}







void writeEEPROM(int address, String data)
{
  int stringSize = data.length();
  for(int i=0;i<stringSize;i++)
  {
    EEPROM.write(address+i, data[i]);
  }
  EEPROM.write(address + stringSize,'\0');   //Add termination null character
  EEPROM.commit();
}


String readEEPROM(int address)
{
  char data[500]; //Max 100 Bytes
  int len=0;
  unsigned char k;
  k = EEPROM.read(address);
  while(k != '\0' && len < 500)   //Read until null character
  {
    k = EEPROM.read(address + len);
    data[len] = k;
    len++;
  }
  data[len]='\0';
  return String(data);
}

void clearEEPROM()
{
  for (int i = 0; i < 1024; ++i) {
    EEPROM.write(i, -1);
  }

  EEPROM.commit();

  server.send(200, "text/plain", "clear EEPROM");
}


String ExplodeValue(String data, char separator, int index)
{
    int found = 0;
    int strIndex[] = { 0, -1 };
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (data.charAt(i) == separator || i == maxIndex) {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i+1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}
