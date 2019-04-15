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
                    sendPacket(response,Udp.remoteIP(),Udp.remotePort());
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
                }
                sendPacket(response,Udp.remoteIP(), Udp.remotePort());
                break;
            case COAP_PUT:
                if (packet.header.type == COAP_NONCON) {
                    handlePut(&response);
                } else if (packet.header.type == COAP_CON) {
                    handlePut(&response);
                } else {
                    Serial.println("COAP_PUT handle error");
                }
                break;
            case COAP_POST:
                if (packet.header.type == COAP_NONCON) {
                    handlePost(&response);
                } else if (packet.header.type == COAP_CON) {
                    handlePost(&response);
                } else {
                    Serial.println("COAP_POST handle error");
                }
                break;
            case COAP_DELETE:
                if (packet.header.type == COAP_NONCON) {
                    handleDelete(&response);
                } else if (packet.header.type == COAP_CON) {
                    handleDelete(&response);
                } else {
                    Serial.println("COAP_DELETE handle error");
                }
                break;
            default:
                Serial.println("Unknown message code.");
        }
    }
    notificationLoop();
}

void CoAP_Server::resourceDiscovery(CoAP_Packet *response){

    String strRes;
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
        response->payload.len = 0;
        resources[resourceIndex].getResourceBytes(response->contentParseBuff,&response->payload.len);
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

void CoAP_Server::handlePut(CoAP_Packet *response){

}

void CoAP_Server::handlePost(CoAP_Packet *response){

}

void CoAP_Server::handleDelete(CoAP_Packet *response){

}

void  CoAP_Server::notificationLoop() {
    unsigned long timestamp = millis();
    if(timestamp - previousMillisTimestamp >= NOTIFICATION_INTERVAL ){
        previousMillisTimestamp = timestamp;
        for(int o = 0; o < MAX_RESOURCES ; o++){
            if(resources[o].isActive() && resources[o].isModified()){
                resources[o].notifyObservers();
            }
        }
    }
}

void CoAP_Server::sendPacket(CoAP_Packet packet, IPAddress ip, int port){
    uint8_t outputBuffer[MAX_UDP_BUFFER_SIZE];
    size_t outputMessageSize;
    packet.serialize(outputBuffer,&outputMessageSize);
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

bool CoAP_Server::resourceRegister(String uri, Data data, ResourceType resourceType, Callback resourceCallback){
    for(int i =0 ; i < MAX_RESOURCES ;i++){
       if(!resources[i].isActive()){
           resources[i].initialize(uri,data, resourceType,resourceCallback,resourceCallback,resourceCallback,resourceCallback);
           return true;
       }
    }
    return false;
}