#include <Wire.h>         //IIC通信
#include <DHT.h>          //调用温湿度传感器的库文件
#include <Adafruit_GFX.h> //引入驱动0.96寸OLED所需的库
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128  //设置OLED宽度,128像素
#define SCREEN_HEIGHT 64  //设置OLED高度,64像素
#define D 9              //定义温度传感器引脚位
#define OLED_RESET 7      //自定义重置引脚,是Adafruit_SSD1306库文件所必需的

#define JDQPin 4          //执行器（继电器）引脚
#define  flowPin  10      //脉冲引脚

float shi_du;            //定义空气湿度变量
float wen_du;            //定义温度变量
int tu_rang;             //定义土壤湿度变量
int guang_zhao;          //定义光照强度变量

int X;                    //用于存储高电平脉冲的持续时间
int Y;                    //用于存储低电平脉冲的持续时间
float shijian = 0;        //用于存储脉冲的总时间（高电平+低电平）
float pinlv = 0;          //用于存储脉冲的频率
float shui = 0;           //用于存储每秒流过的水量（升）
float TOTAL = 0;          //用于累计总水量
float LS = 0;             //用于存储每次循环流过的水量（升/秒）

int SV,PV;                //位式控制变量

DHT dht(D, DHT11);        //设置DHT11模块读取结果变量dht,实例化（引脚，类型）
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);//设置屏幕的大小、地址、复位引脚，必要时软件复位

/******执行模块******/
void water1(){
    digitalWrite(JDQPin,HIGH);
    delay(1000);//打开1s继电器
    digitalWrite(JDQPin,LOW);
    delay(10000);//延时10s渗透
}
void water2(){
    digitalWrite(JDQPin,HIGH);
    delay(2000);//打开2s继电器
    digitalWrite(JDQPin,LOW);
    delay(10000);//延时10s渗透
}
/*******************/

//显示函数
void words_display()
{
    display.clearDisplay();         //清除屏幕
    display.setTextColor(WHITE);    //设置字体颜色,白色可见
    display.setTextSize(1.5);       //设置字体大小
    display.setCursor(0, 0);        //设置oled中显示的位置
    display.print("SoilHumidity:"); //土壤湿度显示
    display.println(tu_rang);//***********************
    display.setCursor(0, 10);
    display.print("Temperature:");  //温度显示
    display.println(wen_du);
    display.setCursor(0, 20);
    display.print("AirHumidity:");  //空气湿度显示
    display.println(shi_du);
    display.setCursor(0, 30);
    display.print("LightIntensity:");//光照强度显示
    display.println(guang_zhao);//************************
    display.setCursor(0, 40);
    display.print("TotalWater:");   //灌溉量显示
    display.println(TOTAL);
}

/******初始化设置******/
void setup() 
{
  Wire.begin();//初始化Wire库
  Serial.begin(9600);//设置波特率（必须与接收端一致）
  
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);//初始化OLED并设置其IIC地址为0x3C
  dht.begin();//dht初始化

  pinMode(A0, INPUT);//土壤湿度

  pinMode(A1, INPUT);//光照强度

  pinMode(JDQPin,OUTPUT);//继电器引脚初始化
}
/*********************/

void loop() 
{   
/******数据采集模块******/
    //读取土壤湿度数据范围0~1023
    tu_rang = analogRead(A0);
    delay(1000);            //延时1秒更新数据

    //读取温湿度数据
    shi_du = dht.readHumidity();    //读dht11湿度数据
    wen_du = dht.readTemperature();  //读dht11温度(摄氏度)
    delay(1000);                     //延时1秒更新数据

    //读取光照强度模拟数据
    guang_zhao = analogRead(A1); 
    delay(1000);        //延时1秒更新数据

    //读取灌溉量模拟数据
    X = pulseIn(flowPin, HIGH); //读取高电平脉冲的持续时间
    Y = pulseIn(flowPin, LOW);  //读取低电平脉冲的持续时间
    shijian = X + Y;                  //计算总脉冲时间
    pinlv = 1000000 / shijian;    //计算脉冲频率（单位：Hz）
    shui = pinlv / 7.5;       //根据YF-S201的规格，每7.5Hz代表每分钟1升水
    LS = shui / 60;               //将每分钟的流量转换为每秒的流量
    TOTAL = TOTAL + LS;            //累加总流量
    delay(1000); 
/***********************/ 

/******算法模块******/
    PV = tu_rang;  //实际值
    SV = 400;       //目标值，定制化方案，可更改变量
    if(PV < SV){    //没达到目标值开始准备浇水
      if(PV > 0 && PV < 341){ //根据实际值细致化浇水    
            water2();//严重缺水，浇水2s    
            }else{
              if(PV > 341 && PV < 682){
                water1();//缺水，浇水1s
              }else{
              if(PV > 682 && PV < 1023){//程度湿
                digitalWrite(JDQPin,LOW);
                  }
                }
            }
    }else{          //高于目标值不浇水
      digitalWrite(JDQPin, LOW);
    }



 /******************/  

/******显示模块******/
    words_display(); //display显示函数
    display.display();//更新屏幕
 /******************/     

    //发送到串口
    Serial.println("************************");

    Serial.print("AirHumidity:");
    Serial.println(shi_du);

    Serial.print("Temperature:");
    Serial.println(wen_du);

    Serial.print("SoilHumidity:");
    Serial.println(tu_rang);//

    Serial.print("LightIntensity:");
    Serial.println(guang_zhao);//

    Serial.print("TotalWater:");
    Serial.println(TOTAL);

    Serial.println("************************");

       
}

