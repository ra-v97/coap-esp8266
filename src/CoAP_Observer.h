//
// Created by rafstach on 07/04/2019.
//

#ifndef COAP_ESP8266_COAP_OBSERVER_H
#define COAP_ESP8266_COAP_OBSERVER_H

#include <ESP8266WiFi.h>
#include <stdint.h>

class CoAP_Observer {
public:
    CoAP_Observer();
    CoAP_Observer(uint8_t observerToken, uint8_t observerTokenLength, IPAddress observerIP, uint16_t observerPort);

    uint8_t observerToken;
    uint8_t observerTokenLength;
    IPAddress observerIP;
    uint16_t observerPort;
    uint8_t state = 0;
    bool active;
};

#endif //COAP_ESP8266_COAP_OBSERVER_H
