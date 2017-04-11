/*
  License TBD Christian Moll
*/

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>

#include <DHT.h>
//#include <MQTTClient.h>

#define DHTTYPE DHT22
#define DHTPIN 4

#define FORCE_DEEPSLEEP

const char* host = "IP-address"; // will also be used on shiftr.io
const char* ssid = "ssid";
const char* password = "pass";

void callback(char* topic, byte* payload, unsigned int length);

WiFiClient net;
//MQTTClient mqtt;
PubSubClient mqttClient(net);

DHT dht(DHTPIN, DHTTYPE);

unsigned int batt;
double battV;

void connect();

void setup(void){
 
  dht.begin();

  Serial.begin(115200);
  Serial.println();
  Serial.println("Booting Sketch...");
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, password);

  connect();
  Serial.println("ready!");
  
//  if (mqttClient.subscribe("myhome/living/reconfig")){
//    Serial.println("Successfully subscribed");
//  }
}

void loop(void){
  if(!mqttClient.connected()) {
    Serial.println("mqtt not connected");
    connect();
  }
  Serial.println("mqtt connected");
  mqttClient.loop();
    
  batt = analogRead(A0);
  battV = mapDouble(batt, 0, 1023, 0.0, 6.6);
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  
  // Check if any reads failed and exit early (to try again).
  if (!isnan(h) || !isnan(t)) {
    mqttClient.publish("myhome/living/temp", String(t).c_str(), true);
    mqttClient.publish("myhome/living/humidity", String(h).c_str(), true);
    mqttClient.publish("myhome/living/batt", String(battV).c_str(), true);
    mqttClient.publish("myhome/living/battRaw", String(batt).c_str(), true);
    mqttClient.publish("myhome/living/resetReason", ESP.getResetReason().c_str(), true);
  }
  
//  delay(10000); // time to make reprogramming possible
    
  #ifdef FORCE_DEEPSLEEP
    Serial.println("Force deepsleep 15min!");
    ESP.deepSleep(15 * 60 * 1000000);
    delay(100);
  #endif
  //handle deep sleep depending on battV
//  if (battV < 3.3) {
//    ESP.deepSleep(10 * 1000000); //send IFTTT low_bat warning
//    delay(100);
//  } else if (battV < 4.0) {
//    ESP.deepSleep(10 * 1000000);
//    delay(100);
//  }
}

void connect() {
  while(WiFi.waitForConnectResult() != WL_CONNECTED){
    WiFi.begin(ssid, password);
    Serial.println("WiFi failed, retrying.");
  }

  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  mqttClient.setServer(host, 1883);
  mqttClient.setCallback (callback);

  while (!mqttClient.connect("esp8266-living")) {
    Serial.print(".");
  }
  Serial.println("\nconnected!");
}


void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println("incoming: ");
  Serial.printf(topic);
  Serial.printf(" - ");
  for (int i = 0; i < sizeof(payload)+1;i++){
   Serial.write(payload[i]); 
  }
  Serial.println();
}

double mapDouble(double x, double in_min, double in_max, double out_min, double out_max)
{
  double temp = (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
  temp = (int) (4*temp + .5);
  return (double) temp/4;
}
