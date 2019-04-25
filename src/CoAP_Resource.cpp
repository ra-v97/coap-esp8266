//
// Created by rafstach on 07/04/2019.
//

#include "CoAP_Resource.h"

CoAP_Resource::CoAP_Resource(){
    active = false;
}

void CoAP_Resource::initialize(String uri,uint8_t* content, size_t bufSize){
    initialize(uri, content, bufSize, (Callback)&CoAP_Resource::defaultCallback, (Callback)&CoAP_Resource::defaultCallback, (Callback)&CoAP_Resource::defaultCallback, (Callback)&CoAP_Resource::defaultCallback);
}

void CoAP_Resource::initialize(String uri,uint8_t* content, size_t bufSize, Callback getCallback ){
    initialize(uri, content, bufSize, getCallback, (Callback)&CoAP_Resource::defaultCallback, (Callback)&CoAP_Resource::defaultCallback, (Callback)&CoAP_Resource::defaultCallback);
}

void CoAP_Resource::initialize(String uri,uint8_t* content, size_t bufSize, Callback getCallback, Callback postCallback){
    initialize(uri, content, bufSize, getCallback, postCallback, (Callback)&CoAP_Resource::defaultCallback, (Callback)&CoAP_Resource::defaultCallback);
}

void CoAP_Resource::initialize(String uri,uint8_t* content, size_t bufSize, Callback getCallback, Callback postCallback, Callback putCallback){
    initialize(uri, content, bufSize, getCallback, postCallback, putCallback, (Callback)&CoAP_Resource::defaultCallback);
}

void CoAP_Resource::initialize(String uri, uint8_t* content, size_t bufSize, Callback getCallback, Callback postCallback, Callback putCallback, Callback deleteCallback){
    this->state = 0;
    this->uri =uri;
    memcpy(this->data,content, bufSize);
    this->bufSize = bufSize;
    this->getCallback = getCallback;
    this->putCallback = putCallback;
    this->postCallback = postCallback;
    this->deleteCallback = deleteCallback;
    this->active = true;
    this->modified = false;
    this->ackFlag = COAP_NONCON;
    this->observersCount = 0;
}

bool CoAP_Resource::isModified(){
    return modified;
}

bool CoAP_Resource::isActive(){
    return active;
}

int CoAP_Resource::addObserver(uint8_t* observerToken, uint8_t observerTokenLength, IPAddress observerIP, uint16_t observerPort){
    for(int index = 0; index<MAX_OBSERVERS; index++) {
        if ((observers[index].observerIP == observerIP && observers[index].observerPort == observerPort) || !observers[index].active) {
            memcpy(observers[index].observerToken,observerToken, observerTokenLength);
            observers[index].observerTokenLength = observerTokenLength;
            observers[index].observerIP = observerIP;
            observers[index].observerPort = observerPort;
            observers[index].active = true;
            observersCount++;
            return 0;
        }
    }
    return 1;
}

int CoAP_Resource::removeObserver(IPAddress observerIP, uint16_t observerPort){
    for(int i = 0 ; i< MAX_OBSERVERS ; i++){
        if(observers[i].observerIP == observerIP && observers[i].observerPort == observerPort){
            observers[i].active = false;
            observersCount--;
        }
    }
    return 0;
}

uint16_t CoAP_Resource::getNotificationMessageId(){
    return notificationMessageId++;
}

void CoAP_Resource::notifyObservers(){
    modified = false;
    for(int i = 0; i < observersCount; i++){
        if(observers[i].active){
            CoAP_Packet response;
            response.header.ver = COAP_VERSION;
            response.header.type = ackFlag;
            response.header.code = COAP_CONTENT;
            response.header.id = notificationMessageId;

            response.payload.len = bufSize;
            memcpy(response.contentParseBuff, data, bufSize);
            response.payload.p = response.contentParseBuff;

            response.optionsNumber = 0;
            response.options[response.optionsNumber].buf.p =&observers[i].state;
            observers[i].state++;
            response.options[response.optionsNumber].buf.len = 1;
            response.options[response.optionsNumber].num = COAP_OBSERVE;
            response.optionsNumber++;

            char optionBuffer2[2];
            optionBuffer2[0] = ((uint16_t)COAP_TEXT_PLAIN & 0xFF00) >> 8;
            optionBuffer2[1] = ((uint16_t)COAP_TEXT_PLAIN & 0x00FF) ;
            response.options[response.optionsNumber].buf.p = (uint8_t *)optionBuffer2;
            response.options[response.optionsNumber].buf.len = 2;
            response.options[response.optionsNumber].num = COAP_CONTENT_FORMAT;
            response.optionsNumber++;

            notificationMessageId = (notificationMessageId+1)%5000;
        }
    }
}

void CoAP_Resource::defaultCallback(){
    Serial.println("Default callback call");
}

