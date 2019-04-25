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

//possible supporting resource type;
typedef enum ResourceType {
    INT = 0,
    FLOAT = 1,
    EMPTY = 4,
} ResourceType;

//possible resource union type
typedef union Data {
    int i;
    float f;
    char byte[4];
} Data;

class CoAP_Resource {
public:
    String uri;
    uint8_t ackFlag;

    bool active;
    uint8_t state;

    CoAP_Observer observers[MAX_OBSERVERS];
    int observersCount;
    uint8_t data[MAX_PAYLOAD_SIZE];
    size_t bufSize;
    bool modified;
    CoAP_Resource();
    void initialize(String uri,uint8_t* content, size_t bufSize);
    void initialize(String uri,uint8_t* content, size_t bufSize, Callback getCallback );
    void initialize(String uri,uint8_t* content, size_t bufSize, Callback getCallback, Callback postCallback);
    void initialize(String uri,uint8_t* content, size_t bufSize, Callback getCallback, Callback postCallback, Callback putCallback);
    void initialize(String uri,uint8_t* content, size_t bufSize, Callback getCallback, Callback postCallback, Callback putCallback, Callback deleteCallback);

    int addObserver(uint8_t* observerToken, uint8_t observerTokenLength, IPAddress observerIP, uint16_t observerPort);
    int removeObserver(IPAddress observerIP, uint16_t observerPort);

    bool isModified();
    bool isActive();
    void notifyObservers();

    Callback getCallback;
    Callback putCallback;
    Callback postCallback;
    Callback deleteCallback;

    void defaultCallback();
    uint16_t getNotificationMessageId();

private:
    uint16_t notificationMessageId = START_OBSERVE_MESSAGE_ID;
};

#endif //COAP_ESP8266_COAP_RESOURCE_H
