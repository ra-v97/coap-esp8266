//
// Created by rafstach on 07/04/2019.
//
#include "CoAP_Resource.h"

CoAP_Resource::CoAP_Resource(){
    active = false;
    modified = false;
    observersCount = 0;
    ackFlag = COAP_NONCON;
    notificationMessageId = START_OBSERVE_MESSAGE_ID;
}

void defaultCallback(){}

void CoAP_Resource::initialize(String uri, uint8_t* content, size_t bufSize){
    initialize(uri, content, bufSize, (Callback)&defaultCallback, (Callback)&defaultCallback, (Callback)&defaultCallback, (Callback)&defaultCallback);
}

void CoAP_Resource::initialize(String uri,uint8_t* content, size_t bufSize, Callback getCallback ){
    initialize(uri, content, bufSize, getCallback, (Callback)&defaultCallback, (Callback)&defaultCallback, (Callback)&defaultCallback);
}

void CoAP_Resource::initialize(String uri,uint8_t* content, size_t bufSize, Callback getCallback, Callback postCallback){
    initialize(uri, content, bufSize, getCallback, postCallback, (Callback)&defaultCallback, (Callback)&defaultCallback);
}

void CoAP_Resource::initialize(String uri,uint8_t* content, size_t bufSize, Callback getCallback, Callback postCallback, Callback putCallback){
    initialize(uri, content, bufSize, getCallback, postCallback, putCallback, (Callback)&defaultCallback);
}

void CoAP_Resource::initialize(String uri, uint8_t* content, size_t bufSize, Callback getCallback, Callback postCallback, Callback putCallback, Callback deleteCallback){
    this->uri =uri;
    memcpy(this->data, content, bufSize);
    this->dataSize = bufSize;
    this->getCallback = getCallback;
    this->putCallback = putCallback;
    this->postCallback = postCallback;
    this->deleteCallback = deleteCallback;
    this->active = true;
    this->modified = false;
    this->observersCount = 0;
}

bool CoAP_Resource::shouldNotifyObservers(){
    return active && modified && observersCount > 0;
}

bool CoAP_Resource::isActive(){
    return active;
}

void CoAP_Resource::deactivate(){
    active = false;
}

void CoAP_Resource::setNotified(){
    modified = false;
}

int CoAP_Resource::addObserver(IPAddress observerIP, uint16_t observerPort, uint8_t* observerToken, size_t observerTokenLength){
    for(int i = 0 ; i < MAX_OBSERVERS ; i++) {
        if(observers[i].activate(observerIP, observerPort, observerToken, observerTokenLength) == 0){
            observersCount++;
            return 0;
        }
    }
    return 1;
}

int CoAP_Resource::removeObserver(IPAddress observerIP, uint16_t observerPort){
    for(int i = 0 ; i < MAX_OBSERVERS ; i++){
        if(observers[i].compare(observerIP, observerPort)){
            observers[i].deactivate();
            observersCount--;
        }
    }
    return 0;
}

int CoAP_Resource::updateResource(uint8_t *content, size_t bufSize){
    if(bufSize > COAP_MAX_PAYLOAD_SIZE){
        return 1;
    }
    memcpy(data, content, bufSize);
    dataSize = bufSize;
    modified = true;
    return 0;
}

uint8_t CoAP_Resource::getNotificationMessageType(){
    return ackFlag;
}

uint16_t CoAP_Resource::getNotificationMessageId(){
    notificationMessageId+=1;
    return (notificationMessageId>>8) | (notificationMessageId<<8);
}

int  CoAP_Resource::getIntValue(){
    return (int)((uint32_t)data[3] | (uint32_t)data[2] << 8 | (uint32_t)data[1] << 16 | (uint32_t)data[0] << 24);
}

float CoAP_Resource::getFloatValue(){
    return (float)((uint32_t)data[3] | (uint32_t)data[2] << 8 | (uint32_t)data[1] << 16 | (uint32_t)data[0] << 24);
}

