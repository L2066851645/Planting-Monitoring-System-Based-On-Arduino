//ESP8266串口接收数据上传Blinker
#define BLINKER_WIFI
#include <Blinker.h>//blinker库函数

//Wi-Fi网络配置
char auth[] = "1c4ddb120ae9";
char ssid[] = "showme13";
char pswd[] = "2066851645";

//定义采集的数据变量
float AirHumi;
float Temper;
int SoilHumi;
int LightIn;
float TOTAL;

//新建组件对象(对象名+组件名)
BlinkerNumber AHUMI("Airhumitidy");   //空气湿度组件
BlinkerNumber TEMPE("Temperature");   //温度组件
BlinkerNumber SHUMI("Soilhumidity");  //土壤湿度组件
BlinkerNumber LIGHT("Lightintensity");//光照强度组件
BlinkerNumber SHUI("guangai");        //灌溉量组件
BlinkerSlider SLIDE("humidset");      //湿度值设置

//心跳包函数每59s返回一次，进行状态查询
void heartbeat() {//反馈的内容
    AHUMI.print(AirHumi);
    TEMPE.print(Temper);
    SHUMI.print(SoilHumi);
    LIGHT.print(LightIn);
    SHUI.print(TOTAL);
}

//回调函数存储湿度到云端（组件名+变量）
void dataStorage(){
    Blinker.dataStorage("Airhumidity",AirHumi);
    Blinker.dataStorage("Temperature",Temper);
    Blinker.dataStorage("Soilhumidity",SoilHumi);
    Blinker.dataStorage("Lightintensity",LightIn);
    Blinker.dataStorage("guangai",TOTAL);
  }

//开启组件实时数据显示（组件名+变量）
void rtData(){
    Blinker.sendRtData("Airhumidity",AirHumi);
    Blinker.sendRtData("Temperature",Temper);
    Blinker.sendRtData("Soilhumidity",SoilHumi);
    Blinker.sendRtData("Lightintensity",LightIn);
    Blinker.sendRtData("guangai",TOTAL);
    Blinker.printRtData();//调用rtdata，官方写法，实时显示数据
}

//注册滑块回调函数
void SLIDE_CALLBACK(int32_t value){
  BLINKER_LOG("get silder data",value);//调试信息，获取滑块值
  Serial.println("SHU");
  Serial.println(value);//字符串发送到串口
  Serial.println("ZI");
}

void setup() {
    Serial.begin(115200);              //初始化串口
    BLINKER_DEBUG.stream(Serial);      //串口调试信息
    BLINKER_DEBUG.debugAll();          //串口调试信息  
    Blinker.begin(auth, ssid, pswd);   //初始化blinker  
    Blinker.attachHeartbeat(heartbeat);//注册一个心跳包
    Blinker.attachRTData(rtData);      //实时数据显示声明
    Blinker.attachDataStorage(dataStorage);//关联回调函数，开启历史数据存储
    SLIDE.attach(SLIDE_CALLBACK);       //app中输入框组件触发并发送到设备端时将触发该组件注册的回调函数
}

void loop() {
   Blinker.run();//保持网络连接并接受处理到的数据
    //查找串口接收缓冲区数据，储存并上传到数据框
    if(Serial.available()>0){
        if(Serial.findUntil("ST","AH") == 1){
            AirHumi = Serial.parseFloat();
            AHUMI.print(AirHumi);
        }
        if(Serial.findUntil("AH","AT") == 1){
            Temper = Serial.parseFloat();
            TEMPE.print(Temper);
        }
        if(Serial.findUntil("AT","SH") == 1){
            SoilHumi = Serial.parseInt();
            SHUMI.print(SoilHumi);
        }
        if(Serial.findUntil("SH","LI") == 1){
            LightIn = Serial.parseInt();
            LIGHT.print(LightIn);
        }
        if(Serial.findUntil("LI","EN") == 1){
             TOTAL= Serial.parseFloat();
            SHUI.print(TOTAL);
        }
    }
  Blinker.delay(2000);  //延时两秒更新数据
}
