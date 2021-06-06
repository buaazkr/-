
/**********************************************************************
项目名称/Project          : 基于nodemcu的温湿度检测云平台模型
程序名称/Program name     : nodemcu端程序
作者/Author              : 高等理工学院 则坤睿（18231053）                             
***********************************************************************/
 
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WebServer.h>
#include <ESP8266_Seniverse.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <DHT.h>
#include <Wire.h>
#include <ThreeWire.h>  
#include <RtcDS1302.h>
#include <SoftwareSerial.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "OLED.h"
#include <stdlib.h>

const char* ssid     = "future";       // 连接WiFi名（此处使用future为示例）
                                            // 请将连接的WiFi名填入引号中
const char* password = "123456789";          // 连接WiFi密码（此处使用123456789为示例）
                                          
#define buttonPin D3
ESP8266WiFiMulti wifiMulti;
ESP8266WebServer esp8266_server(80);
bool pinState;

// 心知天气API请求所需信息
String reqUserKey = "S2bYNzDJSDP8pWdDD";   // 私钥
String reqLocation = "beijing";            // 城市
String reqUnit = "c";                      // 摄氏/华氏
 
WeatherNow weatherNow;  // 建立WeatherNow对象用于获取心知天气信息

//网络时间获取
#define CLK 12
#define DIO 14
String time1="";
bool dian=false;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "ntp1.aliyun.com",60*60*8, 30*60*1000);


//DHT11参数
#define DHTPIN 2 
DHT dht(DHTPIN,DHT11);
char str_h[10];
char str_t[10];
char str_time[10];

//DS18B20参数
#define ONE_WIRE_BUS 14 // DS18B20 on NodeMCU pin D4 
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);

//nodemcu软串口
SoftwareSerial mySerial(4, 0); //RX,TX

//OLED显示器
// display(SDA, SCL);
OLED display(13, 12);


void setup(){
  Serial.begin(9600); 
  mySerial.begin(9600);         
  Serial.println("");
  Serial.println("Here we go!");
  
  //启动OLED显示器
  display.begin();
  display.print("Here we go!");
  delay(3*1000); 
  display.clear();
  
  wifiMulti.addAP(ssid, password);
  int i = 0;                                 
  while (wifiMulti.run() != WL_CONNECTED) {  
    delay(1000);                             
    Serial.print(i++); Serial.print(' ');    
  }                                          
                                             
 
  // WiFi连接成功后将通过串口监视器输出连接成功信息 
  Serial.println('\n');                     
  Serial.print("Connected to ");            
  Serial.println(WiFi.SSID());              
  Serial.print("IP address:\t");           
  Serial.println(WiFi.localIP());         
  
//--------"启动网络服务功能"程序部分开始--------
  esp8266_server.begin();                 
  esp8266_server.on("/", handleRoot);      
  esp8266_server.onNotFound(handleNotFound);        
//--------"启动网络服务功能"程序部分结束--------
  Serial.println("HTTP esp8266_server started");
  //connectWiFi();    // 连接wifi
 
  // 配置心知天气请求信息
  weatherNow.config(reqUserKey, reqLocation, reqUnit);
  timeClient.begin();

  //启动DHT11温度模块
  dht.begin();
  Serial.println("DHT_OK");
   
  //启动DS18B20温度传感器模块
  DS18B20.begin();
  Serial.println("DS18B20_OK");
}
 
void loop(){
  esp8266_server.handleClient();
  if(weatherNow.update()){  // 更新天气信息
    Serial.println(F("======Weahter Info======"));
    Serial.print("Server Response: ");
    Serial.println(weatherNow.getServerCode()); // 获取服务器响应码
    Serial.print(F("Weather Now: "));
    Serial.print(weatherNow.getWeatherText());  // 获取当前天气（字符串格式）
    Serial.print(F(" "));
    Serial.println(weatherNow.getWeatherCode());// 获取当前天气（整数格式）
    Serial.print(F("Temperature: "));
    Serial.println(weatherNow.getDegree());     // 获取当前温度数值
    Serial.print(F("Last Update: "));
    Serial.println(weatherNow.getLastUpdate()); // 获取服务器更新天气信息时间
    Serial.println(F("========================"));     
  } else {    // 更新失败
    Serial.println("Update Fail...");   
    Serial.print("Server Response: ");          // 输出服务器响应状态码供用户查找问题
    Serial.println(weatherNow.getServerCode()); // 心知天气服务器错误代码说明可通过以下网址获取
  }                                             // https://docs.seniverse.com/api/start/error.html

  Serial.println("ok");
  
  //DHT获取温度和湿度
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  Serial.print("Current humidity = ");
  Serial.print(h);
  Serial.print("% ");
  Serial.print("temperature = ");
  Serial.print(t);
  Serial.println("C ");

  //DS18B20获取温度
  float t1;
  DS18B20.requestTemperatures(); 
  t1 = DS18B20.getTempCByIndex(0);

  //温度求平均值，显示温湿度
  t = (t + t1)/2;
  dtostrf(h,3,2,str_h);
  dtostrf(t,3,2,str_t);
  display.print("Humidity:",0,0);
  display.print(str_h,1,5);
  display.print("Temperature:",2,0);
  display.print(str_t,3,5);
  Serial.println("OK");
  
  //软串口输出网络时间，用于同步本地时间
  mySerial.print(time1);

  //网络时间的获取与显示
  timeClient.update();
  time1=timeClient.getFormattedTime();
  Serial.println(time1);
  display.print("Web time:",4,0);
  time1.toCharArray(str_time,sizeof(str_time));
  display.print(str_time,5,0);

  //WIFI IP地址显示
  int buf[4];
  char IP[15];
  IPAddress ip;
  ip = WiFi.localIP();
  String temp0 = "";
  temp0 = String(WiFi.localIP().toString());
  temp0.toCharArray(IP,15);
  display.print("Web server IP:",6,0);
  display.print(IP,7,0);
  delay(300);
}

void handleRoot() {   //处理网站根目录“/”的访问请求 
  String displayHandT;                   // 存储按键状态的字符串变量
  displayHandT += "Humidity";
  displayHandT += str_h;
  displayHandT += "\n";
  displayHandT += "Temperature";
  displayHandT += str_t;
  
  esp8266_server.send(200, "text/html", sendHTML(str_h,str_t)); 
}
 
// 设置处理404情况的函数'handleNotFound'
void handleNotFound(){                                        
  esp8266_server.send(404, "text/plain", "404: Not found");   
}

//温度湿度数据网络显示
String sendHTML(String str_h,String str_t){
  
  String htmlCode = "<!DOCTYPE html> <html>\n";
  htmlCode +="<head><meta http-equiv='refresh' content='1'/>\n";
  htmlCode +="<title>Temperature and Humidity detection system 1.0 </title>\n";
  htmlCode +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  htmlCode +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  htmlCode +="</style>\n";
  htmlCode +="</head>\n";
  htmlCode +="<body>\n";
  htmlCode +="<h1>Temperature and Humidity detection system 1.0</h1>\n";
  
  htmlCode +="<p>Humidity: ";
  htmlCode +="<p>"+str_h ;
  htmlCode +="<p>Temperature: ";
  htmlCode +="<p>"+str_t ;
    
  htmlCode +="</body>\n";
  htmlCode +="</html>\n";
  
  return htmlCode;
}

// 连接WiFi
void connectWiFi(){
  WiFi.begin(ssid, password);                
  Serial.print("Connecting to ");            
  Serial.print(ssid); Serial.println(" ..."); 
  int i = 0;                                 
  while (WiFi.status() != WL_CONNECTED) { 
    delay(1000);                                                 
    Serial.print(i++); Serial.print(' '); 
  }                                         
                                                                                            
  Serial.println("");                         
  Serial.println("Connection established!");   
  Serial.print("IP address:    ");             
  Serial.println(WiFi.localIP());              
}
