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

    CoAP_Resource();
    void initialize(String uri,Data data, ResourceType resourceType);
    void initialize(String uri,Data data, ResourceType resourceType, Callback getCallback );
    void initialize(String uri,Data data, ResourceType resourceType, Callback getCallback, Callback postCallback);
    void initialize(String uri,Data data, ResourceType resourceType, Callback getCallback, Callback postCallback, Callback putCallback);
    void initialize(String uri,Data data, ResourceType resourceType, Callback getCallback, Callback postCallback, Callback putCallback, Callback deleteCallback);

    int updateResource(Data updatedData, ResourceType dataType);

    int addObserver(uint8_t observerToken, uint8_t observerTokenLength, IPAddress observerIP, uint16_t observerPort);
    int removeObserver(IPAddress observerIP, uint16_t observerPort);

    bool isModified();
    bool isActive();
    void notifyObservers();

    Callback getCallback;
    Callback putCallback;
    Callback postCallback;
    Callback deleteCallback;

    void defaultCallback();
    void getResourceBytes(uint8_t *buf, size_t *buflen);

private:
    Data resourceData;
    ResourceType type;
    size_t resourceSize;

    uint8_t ackFlag;

    bool modified;
    bool active;

    CoAP_Observer observers[MAX_OBSERVERS];
    int observersCount;

    int notificationMessageId = 200;
};

#endif //COAP_ESP8266_COAP_RESOURCE_H
