//
// Created by rafstach on 07/04/2019.
//
#ifndef COAP_ESP8266_COAP_SERVER_H
#define COAP_ESP8266_COAP_SERVER_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#include "CoAP_Observer.h"
#include "CoAP_Resource.h"
#include "CoAP_Packet.h"

//CoAP server attributes
#define COAP_DEFAULT_PORT 5683

//CoAP server configuration
#define MAX_RESOURCES 20
#define NOTIFICATION_INTERVAL 500
#define MAX_UDP_BUFFER_SIZE 512

class CoAP_Server {
public:
    CoAP_Server();

    CoAP_Server(int port);

    bool begin();

    void communicationLoop();

    bool resourceRegister(String uri, uint8_t *content, size_t bufSize, Callback resourceCallback);

    bool resourceRegister(String uri, uint8_t *content, size_t bufSize, Callback getCallback, Callback postCallback);

    bool resourceRegister(String uri, uint8_t *content, size_t bufSize);

    int deleteResource(String uri);

    int updateResource(String uri, uint8_t *content, size_t bufSize);

    int updateResource(String uri, int value);

    int updateResource(String uri, float value);

    int getResourceValueString(String uri, char* buffer, size_t bufSize, size_t* outputSize);

    int getResourceValueInt(String uri);

    float getResourceValueFloat(String uri);

private:
    CoAP_Resource resources[MAX_RESOURCES];

    int serverPort;

    unsigned long previousMillisTimestamp;

    uint8_t receiveUdpBuffer[MAX_UDP_BUFFER_SIZE];

    uint8_t responseUdpBuffer[MAX_UDP_BUFFER_SIZE];

    size_t busyResponseMemorySize;

    char resourceUriBuf[COAP_MAX_URI_SIZE];

    size_t resourceUriSize;

    void sendPacket(CoAP_Packet *packet, IPAddress ip, int port);

    void handleResourceDiscovery(CoAP_Packet *response);

    void handleGet(String uri, CoAP_Packet *response);

    void handlePut(String uri, CoAP_Packet *request, CoAP_Packet *response);

    void handlePost(String uri, CoAP_Packet *request, CoAP_Packet *response);

    void handleDelete(String uri, CoAP_Packet *response);

    void handlePing(CoAP_Packet *response);

    void notificationLoop();

    int getResourceIndex(String uri);

    void responseTypeHandle(CoAP_Packet *request, CoAP_Packet *response);

    int setMessageContent(CoAP_Packet *response, uint8_t *content, size_t contentLength, COAP_CONTENT_TYPE type);
};

#endif //COAP_ESP8266_COAP_SERVER_H
