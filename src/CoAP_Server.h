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
#define NOTIFICATION_INTERVAL 5000
#define MAX_UDP_BUFFER_SIZE 512

class CoAP_Server {
public:
    CoAP_Server();

    CoAP_Server(int port);

    bool begin();

    void communicationLoop();

    bool resourceRegister(String uri, uint8_t* content, size_t bufSize, Callback resourceCallback);

    bool resourceUpdate(String uri, Data updatedData, ResourceType dataType);

    int updateResource(String uri,uint8_t* content, size_t bufSize);

private:
    CoAP_Resource resources[MAX_RESOURCES];

    int serverPort;

    unsigned long previousMillisTimestamp;

    void sendPacket(CoAP_Packet* packet, IPAddress ip, int port);

    void resourceDiscovery(CoAP_Packet *response);

    void handleGet(CoAP_Packet *response, String uri);

    void handlePut(String uri, CoAP_Packet *request, CoAP_Packet *response);

    void handlePost(String uri, CoAP_Packet *request, CoAP_Packet *response);

    void handleDelete(String uri, CoAP_Packet *response);

    void notificationLoop();

    int getResourceIndex(String uri);
};

#endif //COAP_ESP8266_COAP_SERVER_H
