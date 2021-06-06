/**********************************************************************
项目名称/Project          : 基于nodemcu的温湿度检测云平台模型
程序名称/Program name     : Arduino Uno端程序
作者/Author              : 高等理工学院 则坤睿（18231053）                             
***********************************************************************/

#include <Wire.h>
#include <ThreeWire.h>  
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <DS1302.h>

LiquidCrystal_I2C lcd(0x27,16,2); 
DS1302 rtc(4, 5, 6); //对应DS1302的RST,DAT,CLK
SoftwareSerial mySerial(3, 2); //RX,TX
int serial_state = 0;
char net_time[20] = "";

void initRTCTime(void)//初始化RTC时钟
{
  rtc.writeProtect(false); //关闭写保护
  rtc.halt(false); //清除时钟停止标志
  Time t(2021, 6, 6, 9, 23, 0, 7); //新建时间对象 最后参数位星期数据，周日为1，周一为2以此类推
  rtc.time(t);//向DS1302设置时间数据
}

int zhq(char a) {
    int t = 0;
    switch(a)
    {
      case '0':
      t=0;
      break;
      case '1':
      t=1;
      break;
      case '2':
      t=2;
      break;
      case '3':
      t=3;
      break;
      case '4':
      t=4;
      break;
      case '5':
      t=5;
      break;
      case '6':
      t=6;
      break;
      case '7':
      t=7;
      break;
      case '8':
      t=8;
      break;
      case '9':
      t=9;
      break;
      default:
      break;                  
    }
    return t;
}

bool judge_data(char a[]){
  for(int i=0; i<8; i++){
    if(i == 2 || i == 5){
      if(a[i] != '-'){
        return false;
      }
    }
    else{
      if(a[i]<'0')
        return false;
      if(a[i]>'9')
        return false;
    }  
  }
  return true;
}

void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);
  lcd.init(); //初始化LCD
  lcd.backlight(); //打开背光
  //initRTCTime();
}
void loop() {
  char a[20] = ""; //定义字符数组，接受来自上位机的数据
  char initial[] = "Initialing...";
  char wait[] = "Please wait...";
  if(serial_state == 0){
    lcd.setCursor(0,0);
    lcd.print(initial);
    lcd.setCursor(0,1);
    lcd.print(wait);
  }
  if(serial_state == 0){
    //while (!Serial.available());
    while (!mySerial.available()); //等待数据传送过来，若没有数据，一直等待，即执行到本行不向下执行
  }
  int i = 0;
  while (mySerial.available()) //当发现缓存中有数据时，将数据送至字符数组a中
  {
    a[i] = mySerial.read();
    i++;
    delay(3);
  }
  if(judge_data){
    int w_s = 0;
    int w_m = 0;
    int w_h = 0;
    w_s = zhq(a[7])+10*zhq(a[6]);
    w_m = zhq(a[4])+10*zhq(a[3]);
    w_h = zhq(a[1])+10*zhq(a[0]);
    if(serial_state == 0){
      rtc.writeProtect(false); //关闭写保护
      rtc.halt(false); //清除时钟停止标志
      Time t(2021, 6, 6, w_h, w_m, w_s, 7); //新建时间对象 最后参数位星期数据，周日为1，周一为2以此类推
      rtc.time(t);//向DS1302设置时间数据
      lcd.clear();
    }
    serial_state = 1;
  }
  Serial.println(a);
  Time tim = rtc.time(); //从DS1302获取时间数据
  char buf[9];
  snprintf(buf, sizeof(buf), "%02d:%02d:%02d",
           tim.hr, tim.min, tim.sec);         
  Serial.println(buf);
  
  char w[17] = "Local Time(1302):";
  lcd.setCursor(0,0);
  lcd.print(w);
  lcd.setCursor(4,1);
  lcd.print(buf);
  delay(100);
}
