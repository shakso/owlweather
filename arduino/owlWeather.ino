#include <Wire.h>
#include <Adafruit_AMG88xx.h>
#include <Adafruit_PWMServoDriver.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

const char* ssid = "xxx";
const char* password = "xxx";

ESP8266WebServer server(80);
Adafruit_AMG88xx amg;

static const char ICACHE_FLASH_ATTR homePage[] = "<html><head><script src=\"https://ajax.googleapis.com/ajax/libs/jquery/3.3.1/jquery.min.js\"></script></head><body><script language=\"Javascript\">function GetColour(v, vmin, vmax){var cr=1; var cg=1; var cb=1; var dv; if (v < vmin) v=vmin; if (v > vmax) v=vmax; dv=vmax - vmin; if (v < (vmin + 0.25 * dv)){cr=0; cg=4 * (v - vmin) / dv;}else if (v < (vmin + 0.5 * dv)){cr=0; cb=1 + 4 * (vmin + 0.25 * dv - v) / dv;}else if (v < (vmin + 0.75 * dv)){cr=4 * (v - vmin - 0.5 * dv) / dv; cb=0;}else{cg=1 + 4 * (vmin + 0.75 * dv - v) / dv; cb=0;}cr=cr*255; cg=cg*255; cb=cb*255; return(cr+\",\"+cg+\",\"+cb);}$(document).ready(function(){setInterval(function(){$.get( \"http://192.168.1.6/getArray\", function( data ){var thermJSON=JSON.parse(data);var max=0;var min=200;for (var i=0; i <=thermJSON.length; i++){if (thermJSON[i] > max) max=thermJSON[i];if ((thermJSON[i] < min) && (thermJSON[i] > 5)) min=thermJSON[i];}for (var i=0; i <=thermJSON.length; i++){console.log();$(\"div#\" + i).css(\"background-color\", \"rgb(\" + GetColour(thermJSON[i],min,max) + \")\");}})}, 500);});</script><style type=\"text/css\">div.block{width:50px;height:50px;float:left;background-color:#ccc;}div#blurme{filter:blur(25px);}div#overflow{position:relative;overflow:hidden;width:400px;}</style><h1>Owl Weather</h1><div id=\"overflow\"><div id=\"blurMe\"><div style=\"clear:both;\"><div class=\"block\" id=\"0\"></div><div class=\"block\" id=\"1\"></div><div class=\"block\" id=\"2\"></div><div class=\"block\" id=\"3\"></div><div class=\"block\" id=\"4\"></div><div class=\"block\" id=\"5\"></div><div class=\"block\" id=\"6\"></div><div class=\"block\" id=\"7\"></div></div><div style=\"clear:both;\"><div class=\"block\" id=\"8\"></div><div class=\"block\" id=\"9\"></div><div class=\"block\" id=\"10\"></div><div class=\"block\" id=\"11\"></div><div class=\"block\" id=\"12\"></div><div class=\"block\" id=\"13\"></div><div class=\"block\" id=\"14\"></div><div class=\"block\" id=\"15\"></div></div><div style=\"clear:both;\"><div class=\"block\" id=\"16\"></div><div class=\"block\" id=\"17\"></div><div class=\"block\" id=\"18\"></div><div class=\"block\" id=\"19\"></div><div class=\"block\" id=\"20\"></div><div class=\"block\" id=\"21\"></div><div class=\"block\" id=\"22\"></div><div class=\"block\" id=\"23\"></div></div><div style=\"clear:both;\"><div class=\"block\" id=\"24\"></div><div class=\"block\" id=\"25\"></div><div class=\"block\" id=\"26\"></div><div class=\"block\" id=\"27\"></div><div class=\"block\" id=\"28\"></div><div class=\"block\" id=\"29\"></div><div class=\"block\" id=\"30\"></div><div class=\"block\" id=\"31\"></div></div><div style=\"clear:both;\"><div class=\"block\" id=\"32\"></div><div class=\"block\" id=\"33\"></div><div class=\"block\" id=\"34\"></div><div class=\"block\" id=\"35\"></div><div class=\"block\" id=\"36\"></div><div class=\"block\" id=\"37\"></div><div class=\"block\" id=\"38\"></div><div class=\"block\" id=\"39\"></div></div><div style=\"clear:both;\"><div class=\"block\" id=\"40\"></div><div class=\"block\" id=\"41\"></div><div class=\"block\" id=\"42\"></div><div class=\"block\" id=\"43\"></div><div class=\"block\" id=\"44\"></div><div class=\"block\" id=\"45\"></div><div class=\"block\" id=\"46\"></div><div class=\"block\" id=\"47\"></div></div><div style=\"clear:both;\"><div class=\"block\" id=\"48\"></div><div class=\"block\" id=\"49\"></div><div class=\"block\" id=\"50\"></div><div class=\"block\" id=\"51\"></div><div class=\"block\" id=\"52\"></div><div class=\"block\" id=\"53\"></div><div class=\"block\" id=\"54\"></div><div class=\"block\" id=\"55\"></div></div><div style=\"clear:both;\"><div class=\"block\" id=\"56\"></div><div class=\"block\" id=\"57\"></div><div class=\"block\" id=\"58\"></div><div class=\"block\" id=\"59\"></div><div class=\"block\" id=\"60\"></div><div class=\"block\" id=\"61\"></div><div class=\"block\" id=\"62\"></div><div class=\"block\" id=\"63\"></div></div></div></div></body></html>";

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

#define SERVOMIN  150 // this is the 'minimum' pulse length count (out of 4096)
#define SERVOMAX  600 // this is the 'maximum' pulse length count (out of 4096)

float pixels[AMG88xx_PIXEL_ARRAY_SIZE];
int regions[8];

float tempDiff = 2.5;
int highestRegion = 0;
int highestRegionOld = 0;
int lockoutTimeout = 0;


void getArray() {
  String returnJSON = "[";
  
  amg.readPixels(pixels);
  
  for(int i=0; i<=AMG88xx_PIXEL_ARRAY_SIZE; i++){
    returnJSON = returnJSON + pixels[i] + ",";
  }
  returnJSON = returnJSON + returnJSON.substring(1, returnJSON.length()-1) + "]";
  
  server.send(200, "text/html", returnJSON);
  
}


void handleRoot() {
  Serial.println("Handle root");
  server.send_P(200, "text/html", homePage); 
  if (server.hasArg("pos")) {
        digitalWrite(D2, LOW);  
        int position = server.arg("pos").toInt();  
        pwm.setPWM(1, 0, position);
        delay(200);
        digitalWrite(D2, HIGH);    
  }  

  if (server.hasArg("tl")) {
        digitalWrite(D2, LOW);  
        int position = server.arg("tl").toInt();  
        pwm.setPWM(2, 0, position);
        delay(500);
        digitalWrite(D2, HIGH);    
  }  
  if (server.hasArg("rl")) {
        digitalWrite(D2, LOW);  
        int position = server.arg("rl").toInt();  
        pwm.setPWM(3, 0, position);
        delay(500);
        digitalWrite(D2, HIGH);    
  }  
  if (server.hasArg("rr")) {
        digitalWrite(D2, LOW);  
        int position = server.arg("rr").toInt();  
        pwm.setPWM(4, 0, position);
        delay(500);
        digitalWrite(D2, HIGH);    
  }  
  if (server.hasArg("tr")) {
        digitalWrite(D2, LOW);  
        int position = server.arg("tr").toInt();  
        pwm.setPWM(5, 0, position);
        delay(500);
        digitalWrite(D2, HIGH);    
  }  
  return;
}

void setup() {
  pinMode(D2, OUTPUT);
  Wire.begin(D3,D4);
  Wire.setClock(400000);
  
  Serial.begin(9600);

  WiFi.hostname("owlweather");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");
  
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  if (MDNS.begin("esp8266")) {
  Serial.println("MDNS responder started");
  }
  
  server.on("/", handleRoot);
  server.on("/getArray", getArray);
  server.begin();
  
  amg.begin(0x69);
  
  pwm.begin();
  digitalWrite(D2, HIGH);
  pwm.setPWMFreq(60);  

  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  
}


void loop() {
    server.handleClient();
    ArduinoOTA.handle();

    amg.readPixels(pixels);
    float average=0;

    for(int i=0; i<=AMG88xx_PIXEL_ARRAY_SIZE; i++){
      average = average + pixels[i];
    }

    average=average/AMG88xx_PIXEL_ARRAY_SIZE;

    for(int i=0; i<=AMG88xx_PIXEL_ARRAY_SIZE; i++){
      if (pixels[i]-average > tempDiff) {
        highestRegion=i%8;
      }
    }
    
    if (highestRegion != highestRegionOld) {
      digitalWrite(D2, LOW);    
      pwm.setPWM(1, 0, 140+(highestRegion*20));
      lockoutTimeout = 4;
    }

    if (lockoutTimeout == 0) {
      digitalWrite(D2, HIGH);
    } else if (lockoutTimeout > 0) {
      lockoutTimeout--;      
    }

    highestRegionOld=highestRegion;

    delay(40);
}
