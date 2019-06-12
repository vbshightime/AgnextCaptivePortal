#include <ArduinoJson.h>
#include <pgmspace.h>
#include "FS.h"
#include <EEPROM.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#define PUSH_PIN 5
#define rtcPos 65
#define resetPos 67
#define tempPos 69
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include "AgNextCaptive.h" 
#include <OneWire.h> 
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 2 // port of NodeMCU=D4
#define MESSAGE_MAX_LEN 256
OneWire oneWire(ONE_WIRE_BUS); 
DallasTemperature sensors(&oneWire);

int temp;
int sum = 0;                    // sum of samples taken
unsigned char sample_count = 0; // current sample number
float voltage = 0.0;            // calculated voltage
float avoltage = 0.0;            // calculated voltage
float avoltage1 = 0.0;
int percentValue = 0; 
int attempt;
int flag=0;
int val=1;
int set;
int zero=0;
int x = 0; 
int j=0;
String macId  = "2ef";
int percent;
int morePercent = 100;
int counter;

/* Soft AP network parameters */
IPAddress apIP(192, 168, 4, 1);
IPAddress netMsk(255, 255, 255, 0);

extern "C" {
#include "user_interface.h" // this is for the RTC memory read/write functions
}


//************** Auxillary functions******************//
ESP8266WebServer server(80);
//**********softAPconfig Timer*************//
unsigned long APTimer = 0;
unsigned long APInterval = 60000;

//*********SSID and Pass for AP**************//
const char* ssidAPConfig = "adminesp55";
const char* passAPConfig = "adminesp55";

//**********check for connection*************//
bool isConnected = true;
bool isAPConnected = true;

//**********Switch Timer******************//
unsigned long SWTimer = 0;
int SWInterval = 5000;

//************Switch Value**********// 
int SWVal = 1;

const char MESSAGE_BODY[] = "{\"deviceId\":\"%s\", \"macId\":%s, \"temp\":%d, \"batteryLevel\":%d}";

void setup() {
  Serial.begin(115200);
  pinMode(A0,INPUT);// taking input from moisture sensor
  while(!Serial);
  //SPIFFS.begin();
  EEPROM.begin(512); 
  pinMode(PUSH_PIN,INPUT);
  //pinMode(LED_BUILTIN, OUTPUT);

  //SWVal = digitalRead(PUSH_PIN);
  //while(!Serial);
   /*system_rtc_mem_read(resetPos, &set, 4);
  if(set!=1)                                 //to restart the system every time //zx
  {
    system_rtc_mem_write(resetPos, &val, 4);
    Serial.println("restarting......");
    ESP.restart();
  }*/  //val=1
  //delay(5000);
  //if(digitalRead(PUSH_PIN))
  //SWTimer = millis();
  //Serial.println();
  //Serial.println(SWVal);
  /*while((digitalRead(PUSH_PIN)==0)){
    if((millis()-SWTimer >= SWInterval)&&(digitalRead(PUSH_PIN)==0)){
             handleWebForm();
             break;
        }
        //Serial.println((millis()-SWTimer)/1000);   
    }*/  
  handleWebForm();     
  WiFi.disconnect(true);
  delay(100);  
  reconnectWiFi();
  sensors.begin(); 
  delay(10);
  

}


void loop() {
sensors.requestTemperatures(); 
int t = sensors.getTempCByIndex(0);
String deviceId = String(read_string(20,100));
Serial.print("Temperature");
Serial.println(t);
if(t==-127)
{
  Serial.print("-127 detect"); 
  system_rtc_mem_read(tempPos, &temp, 4);
  t=temp;
}
delay(100);
system_rtc_mem_write(tempPos, &t, 4);
int sensorValue = analogRead(A0);
// print out the value you read:
Serial.println(sensorValue);
percent=map(sensorValue,218,325,0,100);
if(percent>100)
percent=100;
if(percent<0)
percent=10;
Serial.println(percent);
delay(1000);
StaticJsonBuffer<200> jsonBuffer;
  JsonObject& object = jsonBuffer.createObject();
  object.set("temp",t);
  object.set("deviceId",deviceId);//qe6-1,bd6-2,ce6-3//zx
  object.set("macId","2ef");   //2we-1//2bd-2//2ce-3//2de-4//2ef-5
  if(percent>100){
      object.set("batteryLevel",morePercent);//60//50//batteryLevel//percentValue      
    }else{
           object.set("batteryLevel",percent);//60//50//batteryLevel//percentValue
      }   
  object.printTo(Serial);//print JSON document in serial terminal
  Serial.println(" "); 
  String sendtoserver;
  object.prettyPrintTo(sendtoserver);//sendtoserver is the destination where JSON document should be written. 

  HTTPClient http;    //Declare object of class HTTPClient
      
  //http.begin("http://18.216.76.115:9964/api/sensor/room- temp/save?coolnextNumber= HP01792ABC&humidity=55& temprature=22");      //Specify request destination
  //http.begin("http://192.168. 10.29:9966/api/data");      //Local
  //http.begin("http://13.59.143.40:9966/api/data");        //Public //9964 for live,9955 for testing
  //http.begin("http://3.19.52.97:9966/api/data");//zx        //production
  http.begin("http://3.19.52.97:9955/api/data");//zx        //test
  //http.begin("https://api.agnext.in:9944/api/data");        //live
  //http.begin("http://18.216.76.115:9964/api/sensor/room-temp/save/multiple-packets"); 
  http.addHeader("Content-Type" , "application/json");  //Specify content-type header
   for(attempt=0;attempt<5;attempt++)
  {
  //Serial.println("inside loop");    
  int httpCode = http.POST(sendtoserver);             //Send the request
  String payload = http.getString();                  //Get the response payload
  Serial.println(httpCode);   //Print HTTP return code
  Serial.println(payload);
  if(httpCode == HTTP_CODE_OK) {
                break;        
                }
  delay(100);
  }
   //Serial.println("outside loop");
  http.end();  //Close connection
   flag++; 
  delay(500);  
  if(flag==1)
     {
       system_rtc_mem_write(resetPos, &zero, 4);
       String delays = read_string(30,150);
       Serial.println(delays);
       Serial.println("Entering DeepSleep Mode for" + delays);
       if(delays=="120e6"){
              ESP.deepSleep(120e6);// DeepSleep Mode for 1800 sec(30min)// 60e6 --- 1 min//300e6 --- 5 min // 2100e6---35 min //zx        
        }else if(delays=="300e6"){
                ESP.deepSleep(300e6);// DeepSleep Mode for 1800 sec(30min)// 60e6 --- 1 min//300e6 --- 5 min // 2100e6---35 min //zx        
          }else if(delays =="1800e6"){
                    ESP.deepSleep(1800e6);// DeepSleep Mode for 1800 sec(30min)// 60e6 --- 1 min//300e6 --- 5 min // 2100e6---35 min //zx        
            }
     }
}
  
//----------Write to ROM-----------//
void ROMwrite(String s, String p, String id,String delays){
 s+=";";
 write_EEPROM(s,0);
 p+=";";
 write_EEPROM(p,50);
 id+=";";
 write_EEPROM(id,100);
 delays+=";";
 write_EEPROM(delays,150);
 EEPROM.commit();   
}


//***********Write to ROM**********//
void write_EEPROM(String x,int pos){
  for(int n=pos;n<x.length()+pos;n++){
  //write the ssid and password fetched from webpage to EEPROM
   EEPROM.write(n,x[n-pos]);
  }
}

  
//****************************EEPROM Read****************************//
String read_string(int l, int p){
  String temp;
  for (int n = p; n < l+p; ++n)
    {
   // read the saved password from EEPROM
     if(char(EEPROM.read(n))!=';'){
     
       temp += String(char(EEPROM.read(n)));
     }else n=l+p;
    }
  return temp;
}


//****************************Connect to WiFi****************************//
void reconnectWiFi(){
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
        String string_Ssid="";
        String string_Password="";
        string_Ssid= read_string(30,0); 
        string_Password= read_string(30,50);        
        Serial.println("ssid: "+ string_Ssid);
        Serial.println("Password: "+string_Password);               
  delay(400);
  WiFi.begin(string_Ssid.c_str(),string_Password.c_str());
  int counter = 0;
  while (WiFi.status() != WL_CONNECTED)
  {   
      delay(500);
      Serial.print(".");
      if(counter == 20){
          ESP.restart();
        }
        counter++;
  }
  Serial.print("Connected to:\t");
  Serial.println(WiFi.localIP());
}

void handleWebForm(){ 
      WiFi.mode(WIFI_AP);
      delay(100);
      WiFi.softAPConfig(apIP, apIP, netMsk);
      Serial.println(WiFi.softAP(ssidAPConfig,passAPConfig) ? "Configuring softAP" : "kya yaar not connected");    
      delay(100);
      Serial.println(WiFi.softAPIP());
      server.begin();
      server.on("/", handleDHCP); 
      server.onNotFound(handleNotFound);
      APTimer = millis();
      delay(5000);
      if(WiFi.softAPgetStationNum()>0){
      while(isConnected && millis()-APTimer<= APInterval) {
       server.handleClient();  
        }ESP.restart();
        reconnectWiFi();
      }
}

void handleDHCP(){
  if(server.args()>0){
       for(int i=0; i<=server.args();i++){
          Serial.println(String(server.argName(i))+'\t' + String(server.arg(i)));
        }
       if(server.hasArg("ssid")&&server.hasArg("passkey")&&server.hasArg("device")&&server.hasArg("sensor_list")){
          /*for (int i = 0 ; i < EEPROM.length() ; i++) {
            EEPROM.write(i, 0);
          }*/
           ROMwrite(String(server.arg("ssid")),String(server.arg("passkey")),String(server.arg("device")),String(server.arg("sensor_list")));
           isConnected =false;
        }    
    }else{
         String webString = FPSTR(HTTPHEAD);
         webString+= FPSTR(HTTPBODYSTYLE);
         webString+= FPSTR(HTTPBODY);
         webString+= FPSTR(HTTPCONTENTSTYLE);
         webString+= FPSTR(HTTPDEVICE);
         String device = String(read_string(20,100));
         webString.replace("{s}",device);
         webString+= FPSTR(HTTPFORM);
         webString+= FPSTR(HTTPLABLE1);
         webString+= FPSTR(HTTPLABLE2);
         webString+= FPSTR(HTTPLABLE3);
         webString+= FPSTR(HHTTPDELAY);
         webString+= FPSTR(HTTPSUBMIT);
         webString+= FPSTR(HTTPCLOSE);
         //File file = SPIFFS.open("/AgNextCaptive.html", "r");
         //server.streamFile(file,"text/html");
         server.send(200,"text/html",webString);
         //file.close();
      }
  }


//****************HANDLE NOT FOUND*********************//
void handleNotFound()
{
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  server.send(404, "text/plain", message);
}
