#include <ESP8266WiFi.h>        // 本程序使用 ESP8266WiFi库
#include <ESP8266WiFiMulti.h>   //  ESP8266WiFiMulti库
#include <ESP8266WebServer.h>   //  ESP8266WebServer库
#include <FS.h>
#include "AudioFileSourceICYStream.h"
#include "AudioFileSourceBuffer.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2SNoDAC.h"
ESP8266WiFiMulti wifiMulti;     // 建立ESP8266WiFiMulti对象,对象名称是 'wifiMulti'

ESP8266WebServer esp8266_server(80);// 建立网络服务器对象，该对象用于响应HTTP请求。监听端口（80）
/*int input1=D1;//一号灯带
  int input2=D2;//二号灯带*/

void setup(void) {
  Serial.begin(9600);   // 启动串口通讯

  pinMode(D1, OUTPUT); //1 引脚
  pinMode(D2, OUTPUT); //2 引脚

  wifiMulti.addAP("nova 10z", "12345678"); // 代码编写者手机AP
  wifiMulti.addAP("bmsgiotwifi", "WLAN@958$29852"); // 字节元物联网wifi名
  wifiMulti.addAP("Hi-iot", "88888888"); // 环境查找是否有这里列出的WiFi ID。如果有
  //wifiMulti.addAP("TP-LINK_402", "13793238156");
  Serial.println("Connecting ...");                            // 则尝试使用此处存储的密码进行连接。

  int i = 0;
  while (wifiMulti.run() != WL_CONNECTED) {  // 此处的wifiMulti.run()是重点。通过wifiMulti.run()，NodeMCU将会在当前
    delay(1000);                             // 环境中搜索addAP函数所存储的WiFi。如果搜到多个存储的WiFi那么NodeMCU
    Serial.print(i++); Serial.print(' ');    // 将会连接信号最强的那一个WiFi信号。
  }                                          // 一旦连接WiFI成功，wifiMulti.run()将会返回“WL_CONNECTED”。这也是
  // 此处while循环判断是否跳出循环的条件。

  // WiFi连接成功后将通过串口监视器输出连接成功信息
  Serial.println('\n');
  Serial.print("Connected to ");
  Serial.println(WiFi.SSID());              // 通过串口监视器输出连接的WiFi名称
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());           // 通过串口监视器输出ESP8266-NodeMCU的IP

  esp8266_server.begin();                           // 启动网站服务
  esp8266_server.on("/", HTTP_GET, handleRoot);     // 设置服务器根目录即'/'的函数'handleRoot'
  esp8266_server.on("/open1", HTTP_POST, open1);     // 设置服务器根目录即'/'的函数'handleRoot'
  esp8266_server.on("/close1", HTTP_POST, close1);  // 设置处理LED控制请求的函数'handleLED'
  esp8266_server.on("/open2", HTTP_POST, open2);  // 设置处理LED控制请求的函数'handleLED'
  esp8266_server.on("/close2", HTTP_POST, close2);  // 设置处理LED控制请求的函数'handleLED'
  esp8266_server.on("/closeall", HTTP_POST, closeall);  // 设置处理LED控制请求的函数'handleLED'
  esp8266_server.on("/music", HTTP_GET, playmusic);
  esp8266_server.on("/appopen1", HTTP_GET, open1);
  esp8266_server.on("/appclose1", HTTP_GET, close1);
  esp8266_server.on("/appopen2", HTTP_GET, open2);
  esp8266_server.on("/appclose2", HTTP_GET, close2);
  esp8266_server.on("/getinfo.js", HTTP_GET, getinfo);
  esp8266_server.onNotFound(handleNotFound);        // 设置处理404情况的函数'handleNotFound'

  Serial.println("HTTP esp8266_server started");//  告知用户ESP8266网络服务功能已经启动
}

void loop(void) {
  esp8266_server.handleClient();                     // 检查http服务器访问
}

/*设置服务器根目录即'/'的函数'handleRoot'
  该函数的作用是每当有客户端访问NodeMCU服务器根目录时，
  NodeMCU都会向访问设备发送 HTTP 状态 200 (Ok) 这是send函数的第一个参数。
  同时NodeMCU还会向浏览器发送HTML代码，以下示例中send函数中第三个参数，
  也就是双引号中的内容就是NodeMCU发送的HTML代码。该代码可在网页中产生LED控制按钮。
  当用户按下按钮时，浏览器将会向NodeMCU的/LED页面发送HTTP请求，请求方式为POST。
  NodeMCU接收到此请求后将会执行handleLED函数内容*/
void getinfo(){
    esp8266_server.send(200, "text/javascript", "var info=\"xxxx 物联网道具\";var cnum=2");

  
}
void handleRoot() {
  esp8266_server.send(200, "text/html", "<meta http-equiv='content-type' content='text/html; charset=utf-8'><center><h1>xxxx 物联网灯光控制器</h1>按下下面的按钮,控制道具的灯光</p><form action=\"/open1\" method=\"POST\"><input type=\"submit\" value=\"枪头灯光开\"></form><br><form action=\"/close1\" method=\"POST\"><input type=\"submit\" value=\"枪头灯光关\"></form><br><form action=\"/open2\" method=\"POST\"><input type=\"submit\" value=\"枪尾灯光开\"></form><br><form action=\"/close2\" method=\"POST\"><input type=\"submit\" value=\"枪尾灯光关\"></form><br><form action=\"/stop\" method=\"POST\"><input type=\"submit\" value=\"结束程序重启开发板\"></form><form action=\"\">道具播放网易云的音乐,输入网易云音乐ID:<input type=\"text\" id=\"musicid\" /></form><button onclick='playmusic(musicid.value)' >播放</button><script>function playmusic(musicid) { alert(\"按键点击！\"); var xmlhttp; if (musicid.length==0) { alert(\"请输入音乐ID！\"); return; } if (window.XMLHttpRequest) { // IE7+, Firefox, Chrome, Opera, Safari 浏览器执行代码 xmlhttp=new XMLHttpRequest(); } else {// IE6, IE5 浏览器执行代码 xmlhttp=new ActiveXObject(\"Microsoft.XMLHTTP\"); } var requestString = \"/music?config=on\" + \"&mid=http://music.163.com/song/media/outer/url?id=\"+musicid+\".mp3\";xmlhttp.open(\"GET\", requestString, true);xmlhttp.send(null);}</script></center>");}

      //处理控制请求的函数
      void open1() {
        pinMode(D1, OUTPUT); //1 引脚
        pinMode(D2, OUTPUT); //2 引脚
        digitalWrite(D1, HIGH);
        digitalWrite(D2, LOW);

        esp8266_server.sendHeader("Location", "/");         // 跳转回页面根目录
        esp8266_server.send(303);                           // 发送Http相应代码303 跳转
      }

      //处理控制请求的函数
      void close1() {
        pinMode(D1, OUTPUT); //1 引脚
        pinMode(D2, OUTPUT); //2 引脚
        digitalWrite(D1, LOW);
        digitalWrite(D2, LOW);

        esp8266_server.sendHeader("Location", "/");         // 跳转回页面根目录
        esp8266_server.send(303);                           // 发送Http相应代码303 跳转
      }

      //处理控制请求的函数
      void open2() {
        pinMode(D1, OUTPUT); //1 引脚
        pinMode(D2, OUTPUT); //2 引脚
        digitalWrite(D1, LOW);
        digitalWrite(D2, HIGH);

        esp8266_server.sendHeader("Location", "/");         // 跳转回页面根目录
        esp8266_server.send(303);                           // 发送Http相应代码303 跳转
      }

      //处理控制请求的函数
      void close2() {
        pinMode(D1, OUTPUT); //1 引脚
        pinMode(D2, OUTPUT); //2 引脚
        digitalWrite(D2, LOW);


        esp8266_server.sendHeader("Location", "/");         // 跳转回页面根目录
        esp8266_server.send(303);                           // 发送Http相应代码303 跳转
      }

      //处理控制请求的函数
      void closeall() {
        pinMode(D1, OUTPUT); //1 引脚
        pinMode(D2, OUTPUT); //2 引脚
        digitalWrite(D1, LOW);
        digitalWrite(D2, LOW);


        esp8266_server.sendHeader("Location", "/");         // 跳转回页面根目录
        esp8266_server.send(303);                           // 发送Http相应代码303 跳转
      }
      void MDCallback(void *cbData, const char *type, bool isUnicode, const char *string)
{
  const char *ptr = reinterpret_cast<const char *>(cbData);
  (void) isUnicode; // Punt this ball for now
  // Note that the type and string may be in PROGMEM, so copy them to RAM for printf
  char s1[32], s2[64];
  strncpy_P(s1, type, sizeof(s1));
  s1[sizeof(s1)-1]=0;
  strncpy_P(s2, string, sizeof(s2));
  s2[sizeof(s2)-1]=0;
  Serial.printf("METADATA(%s) '%s' = '%s'\n", ptr, s1, s2);
  Serial.flush();
}

// Called when there's a warning or error (like a buffer underflow or decode hiccup)
void StatusCallback(void *cbData, int code, const char *string)
{
  const char *ptr = reinterpret_cast<const char *>(cbData);
  // Note that the string may be in PROGMEM, so copy it to RAM for printf
  char s1[64];
  strncpy_P(s1, string, sizeof(s1));
  s1[sizeof(s1)-1]=0;
  Serial.printf("STATUS(%s) '%d' = '%s'\n", ptr, code, s1);
  Serial.flush();
}
      //音乐播放
     /* void playmusic() {
        String musicid;
          if (esp8266_server.hasArg("mid")) {
            musicid = esp8266_server.arg("mid");
          }
          Serial.printf(musicid.c_str());
        const char* URL = musicid.c_str();
        Serial.printf("/nCode OK/n");
       /* os_free(musicid);
        Serial.printf("MusicIDTemp Free OK/n");
        AudioGeneratorMP3 *mp3;
        AudioFileSourceICYStream *file;
        AudioFileSourceBuffer *buff;
        AudioOutputI2SNoDAC *out;
        audioLogger = &Serial;
        //file = new AudioFileSourceICYStream("http://music.163.com/song/media/outer/url?id=" + musicid.c_str() + ".mp3");
        file->RegisterMetadataCB(MDCallback, (void*)"ICY");
        buff = new AudioFileSourceBuffer(file, 2048);
        buff->RegisterStatusCB(StatusCallback, (void*)"buffer");
        out = new AudioOutputI2SNoDAC();
        mp3 = new AudioGeneratorMP3();
        mp3->RegisterStatusCB(StatusCallback, (void*)"mp3");
        mp3->begin(buff, out);
        static int lastms = 0;
        if (mp3->isRunning()) {
          if (millis() - lastms > 1000) {
            lastms = millis();
            Serial.printf("Running for %d ms...\n", lastms);
            Serial.flush();
          }
          if (!mp3->loop()) mp3->stop();
        } else {
          Serial.printf("MP3 done\n");
          delay(1000);
        }
      }*/
      void playmusic() {
    String musicid;
    if (esp8266_server.hasArg("mid")) {
        musicid = esp8266_server.arg("mid");
    }
    //Serial.printf("%s\n", musicid.c_str());
    AudioGeneratorMP3 *mp3;
    AudioFileSourceICYStream *file;
    AudioFileSourceBuffer *buff;
    AudioOutputI2SNoDAC *out;
    audioLogger = &Serial;
    String url = "http://music.163.com/song/media/outer/url?id=" + musicid + ".mp3";
    esp8266_server.close();
    file = new AudioFileSourceICYStream(url.c_str());
    file->RegisterMetadataCB(MDCallback, (void*)"ICY");
    buff = new AudioFileSourceBuffer(file, 4096);
    buff->RegisterStatusCB(StatusCallback, (void*)"buffer");
    out = new AudioOutputI2SNoDAC();
    mp3 = new AudioGeneratorMP3();
    mp3->RegisterStatusCB(StatusCallback, (void*)"mp3");
    mp3->begin(buff, out);
    static int lastms = 0;
    if (mp3->isRunning()) {
        if (millis() - lastms > 1000) {
            lastms = millis();
            Serial.printf("Running for %d ms...\n", lastms);
            Serial.flush();
        }
        esp8266_server.begin(); 
        if (!mp3->loop()) mp3->stop();
    } else {
        Serial.printf("MP3 done\n");
        delay(1000);
    }
}


        // 设置处理404情况的函数'handleNotFound'
        void handleNotFound() {
          esp8266_server.send(404, "text/plain", "404: Not found"); // 发送 HTTP 状态 404 (未找到页面) 并向浏览器发送文字 "404: Not found"
        }
