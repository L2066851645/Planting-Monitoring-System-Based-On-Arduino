#include <Wire.h>             //IIC通信库文件
#include <DHT.h>              //调用温湿度传感器的库文件
#include <Adafruit_GFX.h>     //引入驱动0.96寸OLED所需的图形库
#include <Adafruit_SSD1306.h> //SSD1306芯片的控制库

#define SCREEN_WIDTH 128      //设置OLED宽度,128像素
#define SCREEN_HEIGHT 64      //设置OLED高度,64像素
#define D 9                   //定义DHT11传感器引脚位
#define OLED_RESET 7          //自定义重置引脚,是Adafruit_SSD1306库文件所必需的
#define JDQPin 4              //执行器（继电器）引脚
#define SHUIPin 12            //定义连接到YF-S201信号输出引脚的Arduino数字输入引脚

float shi_du;                 //定义空气湿度变量
float wen_du;                 //定义温度变量
int tu_rang;                  //定义土壤湿度变量
uint16_t guang_zhao;          //定义光照强度变量
int BH1750dizhi = 0x23;       //定义GY-30的IIC地址
byte buff[2];                 //定义两个字节即十六位的数组存储GY-30的数据
int PV ,SV;

int X;                    //存储高电平脉冲的持续时间
int Y;                    //存储低电平脉冲的持续时间
float shijian;            //存储脉冲的总时间（高电平+低电平）
float pinlv;              //存储脉冲的频率
float shui;               //存储每秒流过的水量（升）
float LS;                 //存储每次循环流过的水量（升/秒）
float TOTAL;              //定义总灌溉量单位毫升
float ML;                 //单位毫升

DHT dht(D, DHT11);        //设置DHT11模块读取结果变量dht,实例化（引脚，类型）
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);//设置屏幕的大小、地址、复位引脚，必要时软件复位

/******执行模块******/
void water1(){
    digitalWrite(JDQPin,LOW);
    delay(1000);//打开1s继电器
    digitalWrite(JDQPin,HIGH);
    delay(10000);//延时10s渗透
}
void water2(){
    digitalWrite(JDQPin,LOW);
    delay(3000);//打开3s继电器
    digitalWrite(JDQPin,HIGH);
    delay(10000);//延时10s渗透
}
/*******************/

//读入传感器地址，接收传感器数据
int BH1750_Read(int dizhi)
{
  int i = 0;//用于计数接收到的数据字节数
  Wire.beginTransmission(dizhi);  //开始与BH1750设备的IIC通信，传入设备的地址
  Wire.requestFrom(dizhi, 2);     //从指定地址的设备请求2个字节的数据
  while (Wire.available())        //当IIC总线上有数据可读时，开始接收数据
  {
    buff[i] = Wire.read();        //接收一个字节（byte），从I2C总线读取一个字节（byte）的数据并存储在buff数组中
    i++;                          //增加接收数据的计数
  }
  Wire.endTransmission();         //结束IIC通信
  return i;                       //返回读取的数据字节数
}

//BH1750初始化
void BH1750_Init(int dizhi)   
{
  Wire.beginTransmission(dizhi);  //开始与BH1750设备的IIC通信，传入设备的地址
  Wire.write(0x10);               //设置传感器为持续高分辨率模式，测量时间为120ms，持续且每次测量1 lux的光照
  Wire.endTransmission();         //结束IIC通信
}

//显示函数
void oled_display()
{
    display.clearDisplay();         //清除屏幕
    display.setTextColor(WHITE);    //设置字体颜色,白色可见
    display.setTextSize(1.5);       //设置字体大小
    display.setCursor(0, 0);        //设置oled中显示的位置
    display.print("SoilHumidity:"); 
    display.print(tu_rang);         //土壤湿度显示
    display.setCursor(0, 10);
    display.print("Temperature:");  
    display.print(wen_du);          //温度显示
    display.setCursor(0, 20);
    display.print("AirHumidity:");  
    display.print(shi_du);          //空气湿度显示
    display.setCursor(0, 30);
    display.print("LightIntensity:");
    display.print(guang_zhao);      //光照强度显示
    display.setCursor(0, 40);
    display.print("TotalWater:");   
    display.print(TOTAL);           //灌溉量显示
}

//自动浇水
void zidong(){
    if(PV > 224 && PV < 344){
      digitalWrite(JDQPin,HIGH);  //继电器关闭
    }else{
      if(PV > 344 && PV < 464){
      water1();   //缺水，浇水1s
    }else{
      if(PV > 464 && PV < 584){
        water2(); //严重缺水，浇水3s
        }
      } 
    }
}

//手动浇水
void shoudong(){
    if(SV > 224 && SV < 344){     
      digitalWrite(JDQPin,HIGH);  //继电器关闭        
    }else{
      if(SV > 344 && SV < 464){
        water1();   //缺水，浇水1s
    }else{
      if(SV > 464 && SV < 584){
        water2();   //严重缺水，浇水3s
        }
      }
    }  
}

/******初始化设置******/
void setup() 
{
  Wire.begin();             //初始化Wire库
  Serial.begin(115200);     //设置波特率（必须与接收端一致）
  
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);//初始化OLED并设置其IIC地址为0x3C
  dht.begin();              //dht对象初始化

  BH1750_Init(BH1750dizhi); //BH1750地址初始化0x23

  pinMode(SHUIPin, INPUT);  //将YF-S201的信号引脚设置为输入模式

  pinMode(A0, INPUT);       //土壤湿度模拟引脚设置为输入模式

  pinMode(JDQPin,OUTPUT);   //继电器引脚初始化
  digitalWrite(JDQPin,HIGH);
  
}
/*********************/

void loop() {   
  /******数据采集模块******/
    //读取土壤湿度数据
    tu_rang = analogRead(A0);       //测量范围224-584
    delay(1000);                    //延时更新数据

    //读取温湿度数据
    shi_du = dht.readHumidity();    //读dht11湿度数据
    wen_du = dht.readTemperature(); //读dht11温度(摄氏度)
    delay(1000);                    //延时更新数据

    //读取光照强度数据
    if (2 == BH1750_Read(BH1750dizhi))  {
      if (buff[0] == 255 && buff[1] == 255){
          guang_zhao = 65535;       //光照达到最强或者数据溢出时的数据
        } else {
          guang_zhao = ((buff[0] << 8) | buff[1]) / 1.2; //BH1750芯片手册中规定的数值计算方式
        }
    }
    delay(1000);                    //延时更新数据        

    //读取灌溉量数据
    X = pulseIn(SHUIPin, HIGH); //读取高电平脉冲的持续时间
    Y = pulseIn(SHUIPin, LOW);  //读取低电平脉冲的持续时间
    shijian = X + Y;            //计算一个脉冲周期总时间
    if(shijian == 0.0){         //没有水流经过时
      TOTAL;                    //灌溉量数据暂存 
    }else{
      pinlv = 1000000.0 / shijian;          //时间单位us，计算脉冲频率（单位：Hz）
      shui = pinlv / 7.5;                   //根据YF-S201的规格，每7.5Hz代表每分钟1升水
      LS = shui / 60.0;                     //将每分钟的流量转换为每秒的流量
      ML = (shijian / 1000000.0) * LS * 1000.0;//毫升(ml)=时间(s)*流量(L/s)*1000
      TOTAL = TOTAL + ML;                   //累加总流量毫升
    }
  /***********************/ 


  /******显示模块******/
    oled_display();         //display显示函数
    display.display();      //display更新屏幕
 /******************/     

    Serial.println("ST");   //标记
    Serial.println(shi_du); //发送湿度数据到串口
    delay(1000);

    Serial.println("AH");   //标记
    Serial.println(wen_du); //发送温度数据到串口
    delay(1000);

    Serial.println("AT");     //标记
    Serial.println(tu_rang);  //发送土壤湿度数据到串口
    delay(1000);

    Serial.println("SH");           //标记
    Serial.println(guang_zhao,DEC); //以十进制发送光照数据到串口
    delay(1000);

    Serial.println("LI");   //标记
    Serial.println(TOTAL);  //发送灌溉量数据到串口
    delay(1000);

    Serial.println("EN");   //标记
    delay(1000);

  /******算法模块******/
    PV = tu_rang;     //PV实际范围224~584，划分为湿，润，干，224，344，464，584且与湿度成反比
    zidong();         //自动浇水
    if(Serial.available() > 0){//查找串口接收缓冲区的数据
      if(Serial.findUntil("SHU","ZI") == 1){//如果发现滑块反馈的数据
        SV = Serial.parseInt();//设置该值为期望值SV
        shoudong();   //手动浇水  
        }
    }
 /******************/         
}
