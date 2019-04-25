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

uint16_t CoAP_Resource::getNotificationMessageId(){
    return notificationMessageId++;
}

void CoAP_Resource::defaultCallback(){}

