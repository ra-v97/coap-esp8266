  /*
 * Turn on Soft AP mode on ESP8266-01s and test communication.
 * Example let you control state of ESP8266 board led using CoAP resources.
 * To turn on led send post request with "On" message. "Off" post message turns off the led.
 */

#include <ESP8266CoAP.h>
#include <ESP8266WiFi.h>

#define BUILTIN_LED1 1 //GPIO1
#define GPIO2 2 //GPIO2

const char* ssid = "ESP8266WiFi";
const char* password = "balancerobot"; //be careful password have to starts with letter and must be longer than 8.

CoAP_Server server(5683);

bool ledState;

void getCallback(){
}

void postCallback(){
  char buf[5];
  size_t len;
  server.getResourceValueString("Led", buf, 5, &len);
  String state = String(buf);

  if (state.compareTo("On") == 0){
    digitalWrite(BUILTIN_LED1,LOW);
    ledState = true;
  }else if(state.compareTo("Off") == 0) {
    digitalWrite(BUILTIN_LED1,HIGH);
    ledState = false;
  }
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
  }else{
    // Error occured
  }

  server.resourceRegister("Led", (uint8_t*)"Off", 12, getCallback, postCallback);
  pinMode(BUILTIN_LED1,OUTPUT);
  digitalWrite(BUILTIN_LED1,HIGH);
  server.begin();
}

void loop() {
  server.communicationLoop();
}