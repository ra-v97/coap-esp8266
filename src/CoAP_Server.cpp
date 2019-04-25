//
// Created by rafstach on 07/04/2019.
//

#include "CoAP_Server.h"

//Required instances;
WiFiUDP Udp;

CoAP_Server::CoAP_Server(int port) {
    this->serverPort = port;
}

CoAP_Server::CoAP_Server() {
    this->serverPort = COAP_DEFAULT_PORT;
}

bool CoAP_Server::begin() {
    Udp.begin(this->serverPort);
    return true;
}

void CoAP_Server::communicationLoop() {
    uint8_t udpBuffer[MAX_UDP_BUFFER_SIZE];
    int receivedUdpPacketLength = Udp.parsePacket();

    if (receivedUdpPacketLength) {
        receivedUdpPacketLength =
                Udp.read(udpBuffer,
                         receivedUdpPacketLength > MAX_UDP_BUFFER_SIZE ? MAX_UDP_BUFFER_SIZE : receivedUdpPacketLength);

        CoAP_Packet packet;
        packet.parse(udpBuffer, receivedUdpPacketLength);

        char resourceUriBuf[MAX_URI_SIZE];
        size_t resourceSize;
        packet.getResourceUri(resourceUriBuf, &resourceSize);
        String uri = String(resourceUriBuf);

        CoAP_Packet response;
        response.header.ver = COAP_VERSION;
        response.header.id = packet.header.id;
        response.header.tkl = packet.header.tkl;
        response.token = packet.token;

        switch (packet.header.code) {
            case COAP_EMPTY:
                if (packet.header.type == COAP_CON) {
                    response.header.type = COAP_RESET;
                    response.header.code = COAP_EMPTY_MESSAGE;
                    response.optionsNumber = 0;
                    response.payload.len  = 0;
                    response.payload.p = NULL;
                    sendPacket(&response,Udp.remoteIP(),Udp.remotePort());
                } else if (packet.header.type == COAP_RESET) {
                    for(int i =0 ; i< MAX_RESOURCES ; i++){
                        if(resources[i].isActive()){
                            resources[i].removeObserver(Udp.remoteIP(), Udp.remotePort());
                        }
                    }
                } else {
                    if(packet.header.type == COAP_ACK){
                        Serial.println("COAP_ACK received");
                    }else{
                        Serial.println("COAP_EMPTY handle error");
                    }
                }
                break;
            case COAP_GET:
                if (packet.header.type == COAP_NONCON) {
                    response.header.type = COAP_NONCON;
                } else if (packet.header.type == COAP_CON) {
                    response.header.type = COAP_CON;
                }
                if(uri.equalsIgnoreCase(String(DISCOVERY_RESOURCE_PATH))){
                    resourceDiscovery(&response);
                } else{
                    handleGet(&response,uri);
                    for(int i = 0 ; i < packet.optionsNumber; i++){
                        if(packet.options[i].num == COAP_OBSERVE){
                            int index = getResourceIndex(uri);
                            if(index < 0)
                                break;
                            resources[index].addObserver((uint8_t*)packet.token.p, packet.token.len, Udp.remoteIP(),Udp.remotePort());
                        }
                    }
                }
                sendPacket(&response,Udp.remoteIP(), Udp.remotePort());
                break;
            case COAP_PUT:
                if (packet.header.type == COAP_NONCON) {
                    response.header.type = COAP_NONCON;
                } else if (packet.header.type == COAP_CON) {
                    response.header.type = COAP_CON;
                }
                handlePut(uri, &packet, &response);
                sendPacket(&response,Udp.remoteIP(), Udp.remotePort());
                break;
            case COAP_POST:
                if (packet.header.type == COAP_NONCON) {
                    response.header.type = COAP_NONCON;
                } else if (packet.header.type == COAP_CON) {
                    response.header.type = COAP_CON;
                }
                handlePost(uri, &packet, &response);
                sendPacket(&response,Udp.remoteIP(), Udp.remotePort());
                break;
            case COAP_DELETE:
                response.header.type = COAP_NONCON;
                handleDelete(uri, &response);
                sendPacket(&response,Udp.remoteIP(), Udp.remotePort());
                break;
            default:
                Serial.println("Unknown message code.");
        }
    }
    notificationLoop();
}

void CoAP_Server::resourceDiscovery(CoAP_Packet *response){

    String strRes;
    strRes +="</";
    strRes +="register";
    strRes +=">;";
    strRes +="register";
    strRes += ",";

    for(int i=0;i<MAX_RESOURCES;i++){
        if(resources[i].isActive()){
            strRes +="</";
            strRes +=resources[i].uri;
            strRes +=">;";
            strRes +=resources[i].uri;
            strRes +=";rt=";
            strRes +="\"";
            strRes +="observe";
            strRes +="\"";

            strRes +=";";
            strRes +="ct=";
            strRes +="0";
            strRes +=";";
            strRes+="title=\"";
            strRes +="observable resource";
            strRes+="\"";
            strRes +=",";
        }
    }
    response->payload.len = strRes.length();
    memcpy(response->contentParseBuff,strRes.c_str(),response->payload.len);
    response->payload.p = response->contentParseBuff;

    response->header.code=COAP_CONTENT ;
    response->optionsNumber=0;
    char optionBuffer[2];
    optionBuffer[0] = ((uint16_t)COAP_APPLICATION_LINK_FORMAT & 0xFF00) >> 8;
    optionBuffer[1] = ((uint16_t)COAP_APPLICATION_LINK_FORMAT & 0x00FF) ;
    response->options[response->optionsNumber].buf.p = (uint8_t *)optionBuffer;
    response->options[response->optionsNumber].buf.len = 2;
    response->options[response->optionsNumber].num = COAP_CONTENT_FORMAT;
    response->optionsNumber++;
}

void CoAP_Server::handleGet(CoAP_Packet *response,String uri){
    int resourceIndex = getResourceIndex(uri);
    if(resourceIndex >=0){
        resources[resourceIndex].getCallback();
        response->payload.len = resources[resourceIndex].bufSize;
        memcpy(response->contentParseBuff, resources[resourceIndex].data, resources[resourceIndex].bufSize);
        response->payload.p = response->contentParseBuff;
        response->header.code = COAP_CONTENT;
    }else{
        response->payload.p=NULL;
        response->payload.len=0;
        response->header.code=COAP_NOT_FOUND;
    }
    response->optionsNumber=0;

    char optionBuffer[2];
    optionBuffer[0] = ((uint16_t)COAP_TEXT_PLAIN  & 0xFF00) >> 8;
    optionBuffer[1] = ((uint16_t)COAP_TEXT_PLAIN  & 0x00FF) ;
    response->options[response->optionsNumber].buf.p = (uint8_t *)optionBuffer;
    response->options[response->optionsNumber].buf.len = 2;
    response->options[response->optionsNumber].num = COAP_CONTENT_FORMAT;
    response->optionsNumber++;
}

void CoAP_Server::handlePut(String uri, CoAP_Packet *request, CoAP_Packet *response) {
    int resourceIndex = getResourceIndex(uri);
    if (resourceIndex >= 0) {
        resources[resourceIndex].putCallback();
        response->payload.p = NULL;
        response->payload.len = 0;
        response->header.code = COAP_METHOD_NOT_ALLOWED;
    } else {
        resourceRegister(uri,(uint8_t*)request->payload.p, request->payload.len, (Callback)&CoAP_Resource::defaultCallback);

        response->payload.len = request->payload.len;
        memcpy(response->contentParseBuff, request->payload.p, request->payload.len);
        response->payload.p = response->contentParseBuff;
        response->header.code = COAP_CREATED;

        response->optionsNumber=0;
        char optionBuffer[2];
        optionBuffer[0] = ((uint16_t) COAP_TEXT_PLAIN & 0xFF00) >> 8;
        optionBuffer[1] = ((uint16_t) COAP_TEXT_PLAIN & 0x00FF);
        response->options[response->optionsNumber].buf.p = (uint8_t *) optionBuffer;
        response->options[response->optionsNumber].buf.len = 2;
        response->options[response->optionsNumber].num = COAP_CONTENT_FORMAT;
        response->optionsNumber++;
    }
}

void CoAP_Server::handlePost(String uri, CoAP_Packet *request, CoAP_Packet *response){
    int resourceIndex = getResourceIndex(uri);
    if (resourceIndex >= 0) {
        updateResource(uri, (uint8_t*)request->payload.p,request->payload.len);
    } else {
        resourceRegister(uri, (uint8_t *) request->payload.p, request->payload.len,
                         (Callback) &CoAP_Resource::defaultCallback);
    }
        resources[resourceIndex].postCallback();

        response->payload.len = request->payload.len;
        memcpy(response->contentParseBuff, request->payload.p, request->payload.len);
        response->payload.p = response->contentParseBuff;
        response->header.code = COAP_CHANGED;

        response->optionsNumber=0;
        char optionBuffer[2];
        optionBuffer[0] = ((uint16_t) COAP_TEXT_PLAIN & 0xFF00) >> 8;
        optionBuffer[1] = ((uint16_t) COAP_TEXT_PLAIN & 0x00FF);
        response->options[response->optionsNumber].buf.p = (uint8_t *) optionBuffer;
        response->options[response->optionsNumber].buf.len = 2;
        response->options[response->optionsNumber].num = COAP_CONTENT_FORMAT;
        response->optionsNumber++;
}

void CoAP_Server::handleDelete(String uri, CoAP_Packet *response){
    int resourceIndex = getResourceIndex(uri);
    if(resourceIndex >=0){
        resources[resourceIndex].deleteCallback();
        resources[resourceIndex].active = false;
        response->payload.p=NULL;
        response->payload.len=0;
        response->header.code = COAP_DELETED;
    }else{
        response->payload.p=NULL;
        response->payload.len=0;
        response->header.code=COAP_NOT_FOUND;
    }
    response->optionsNumber=0;
}

int CoAP_Server::updateResource(String uri, uint8_t* content, size_t bufSize){
    int index = getResourceIndex(uri);
    if(index < 0)
        return 1;

    if(resources[index].isActive()){
        memcpy(resources[index].data, content, bufSize);
        resources[index].bufSize = bufSize;
        resources[index].modified = true;
        return 0;
    }
    return 1;
}

void  CoAP_Server::notificationLoop() {
    unsigned long timestamp = millis();
    if(timestamp - previousMillisTimestamp >= NOTIFICATION_INTERVAL ){
        previousMillisTimestamp = timestamp;

        CoAP_Packet response;
        response.header.ver = COAP_VERSION;
        response.header.code = COAP_CONTENT;

        for(int o = 0; o < MAX_RESOURCES ; o++){
            if(resources[o].isActive() && resources[o].isModified()){
                resources[o].modified = false;
                resources[o].state++;

                response.header.type = resources[o].ackFlag;
                response.header.id = resources[o].getNotificationMessageId();

                response.payload.len = resources[o].bufSize;
                memcpy(response.contentParseBuff, resources[o].data, resources[o].bufSize);
                response.payload.p = response.contentParseBuff;

                response.optionsNumber = 0;
                uint8_t buq[1];
                buq[0]=resources[o].state;
                response.options[response.optionsNumber].buf.len = 1;
                response.options[response.optionsNumber].buf.p = buq;
                response.options[response.optionsNumber].num = COAP_OBSERVE;
                response.optionsNumber++;

                response.options[response.optionsNumber].buf.p = (const uint8_t*)resources[o].uri.c_str();
                response.options[response.optionsNumber].buf.len = resources[o].uri.length();
                response.options[response.optionsNumber].num = COAP_URI_PATH;
                response.optionsNumber++;

                char optionBuffer2[2];
                optionBuffer2[0] = ((uint16_t)COAP_TEXT_PLAIN & 0xFF00) >> 8;
                optionBuffer2[1] = ((uint16_t)COAP_TEXT_PLAIN & 0x00FF) ;
                response.options[response.optionsNumber].buf.p = (uint8_t *)optionBuffer2;
                response.options[response.optionsNumber].buf.len = 2;
                response.options[response.optionsNumber].num = COAP_CONTENT_FORMAT;
                response.optionsNumber++;

                for(int i = 0; i < MAX_OBSERVERS; i++){
                   if(resources[o].observers[i].active){
                       response.header.tkl = resources[o].observers[i].observerTokenLength;
                       response.token.p = resources[o].observers[i].observerToken;
                       response.token.len = resources[o].observers[i].observerTokenLength;
                       sendPacket(&response, resources[o].observers[i].observerIP, resources[o].observers[i].observerPort);
                   }
                }
            }
        }
    }
}

void CoAP_Server::sendPacket(CoAP_Packet* packet, IPAddress ip, int port){
    uint8_t outputBuffer[MAX_UDP_BUFFER_SIZE];
    size_t outputMessageSize;
    (*packet).serialize(outputBuffer,&outputMessageSize);
    Udp.beginPacket(ip,port);
    Udp.write(outputBuffer, outputMessageSize);
    Udp.endPacket();
}

int CoAP_Server::getResourceIndex(String uri){
    for(int i = 0 ; i < MAX_RESOURCES ; i++){
     if(resources[i].isActive() && resources[i].uri.equalsIgnoreCase(uri)){
         return i;
     }
    }
    return -1;
}

bool CoAP_Server::resourceRegister(String uri, uint8_t* content, size_t bufSize, Callback resourceCallback){
    for(int i =0 ; i < MAX_RESOURCES ;i++){
       if(!resources[i].isActive()){
           resources[i].initialize(uri,content, bufSize,resourceCallback,resourceCallback,resourceCallback,resourceCallback);
           return true;
       }
    }
    return false;
}