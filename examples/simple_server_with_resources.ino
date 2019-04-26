/*
 * Turn on Soft AP mode on ESP8266-01s and test communication.
 * For testing you need to connect available Wi-Fi and start sending CoAP requests.
 */

#include <ESP8266CoAP.h>
#include <ESP8266WiFi.h>

const char* ssid = "ESP8266WiFi";
const char* password = "balancerobot"; //be careful password have to starts with letter and must be longer than 8.

CoAP_Server server(5683);

void getCallback(){
  Serial.println("Received get request");
}

void postCallback(){
  char buf[20];
  size_t len;
  server.getResourceValueString("Temperature", buf, 20, &len);
  Serial.print("Received post: ");
  Serial.println(buf);
}

 void setup() {
  yield();
  delay(1000);
  Serial.begin(115200);
  Serial.println();
  Serial.println("Access point starting...");

  bool status = WiFi.softAP(ssid,password);

  if(status){
    IPAddress hostIp = WiFi.softAPIP();

    Serial.print("WiFi is working, network name: ");
    Serial.println(ssid);
    Serial.print("Host IP Address: ");
    Serial.println(hostIp);
    Serial.println("");
  }else{
    Serial.println("Error occurred");
  }

  server.resourceRegister("Message", (uint8_t*)"Hello world!", 12, getCallback);
  server.resourceRegister("Temperature", (uint8_t*)"", 0, getCallback, postCallback);
  server.begin();
}

unsigned long lastTimestamp = 0;
int temperature = 20;

void loop() {
  unsigned long timestamp = millis();
  if(timestamp - lastTimestamp > 5000){
    lastTimestamp = timestamp;
    server.updateResource("Temperature",temperature++);
    Serial.println(server.getResourceValueInt("Temperature"));
  }

  server.communicationLoop();
}