//
// Created by rafstach on 25/04/2019.
//

#ifndef COAP_ESP8266_COAP_OBSERVER_H
#define COAP_ESP8266_COAP_OBSERVER_H

#include <ESP8266WiFi.h>
#include <stdint.h>

#define MAX_TOKEN_SIZE 8
#define OBSERVER_STATE_SIZE 2

class CoAP_Observer {
public:
    CoAP_Observer();
    int activate(IPAddress observerIP, uint16_t observerPort, uint8_t* observerToken, uint8_t observerTokenLength);
    void deactivate();
    int getObserverInfo(IPAddress* observerIP, uint16_t* observerPort, uint8_t* observerToken, uint8_t* observerTokenLength, uint16_t* observerState);
    bool compare(IPAddress observerIP, uint16_t observerPort);

private:
    IPAddress ip;
    uint16_t port;

    uint8_t token[MAX_TOKEN_SIZE];
    uint8_t tokenLength;

    uint16_t state;
    bool active;

    bool canBeActivated(IPAddress observerIP, uint16_t observerPort);
};

#endif //COAP_ESP8266_COAP_OBSERVER_H
