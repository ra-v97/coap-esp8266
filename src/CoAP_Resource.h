//
// Created by rafstach on 07/04/2019.
//
#ifndef COAP_ESP8266_COAP_RESOURCE_H
#define COAP_ESP8266_COAP_RESOURCE_H

#include "CoAP_Observer.h"
#include "CoAP_Packet.h"

//CoAP resource configuration
#define MAX_OBSERVERS 10
#define DISCOVERY_RESOURCE_PATH ".well-known/core"
#define START_OBSERVE_MESSAGE_ID 100

//callback function type definition
typedef void (*Callback)();


class CoAP_Resource {
public:
    String uri;

    uint8_t data[COAP_MAX_PAYLOAD_SIZE];

    size_t dataSize;

    CoAP_Observer observers[MAX_OBSERVERS];

    Callback getCallback;

    Callback putCallback;

    Callback postCallback;

    Callback deleteCallback;

    CoAP_Resource();

    void initialize(String uri,uint8_t* content, size_t bufSize);

    void initialize(String uri,uint8_t* content, size_t bufSize, Callback getCallback );

    void initialize(String uri,uint8_t* content, size_t bufSize, Callback getCallback, Callback postCallback);

    void initialize(String uri,uint8_t* content, size_t bufSize, Callback getCallback, Callback postCallback, Callback putCallback);

    void initialize(String uri,uint8_t* content, size_t bufSize, Callback getCallback, Callback postCallback, Callback putCallback, Callback deleteCallback);

    int addObserver(IPAddress observerIP, uint16_t observerPort, uint8_t* observerToken, size_t observerTokenLength);

    int removeObserver(IPAddress observerIP, uint16_t observerPort);

    int updateResource(uint8_t *content, size_t bufSize);

    bool isActive();

    void deactivate();

    void setNotified();

    bool shouldNotifyObservers();

    int getIntValue();

    float getFloatValue();

    uint8_t getNotificationMessageType();

    uint16_t getNotificationMessageId();

private:
    bool active;

    bool modified;

    int observersCount;

    uint8_t ackFlag;

    uint16_t notificationMessageId;
};

#endif //COAP_ESP8266_COAP_RESOURCE_H
