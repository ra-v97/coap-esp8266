//
// Created by rafstach on 25/04/2019.
//

#include "CoAP_Observer.h"

CoAP_Observer::CoAP_Observer() {
    active = false;
}

int CoAP_Observer::activate(IPAddress observerIP, uint16_t observerPort, uint8_t *observerToken, uint8_t observerTokenLength) {
    if(canBeActivated(observerIP, observerPort)){
        ip = observerIP;
        port = observerPort;

        if(observerTokenLength > MAX_TOKEN_SIZE){
            return 2;
        }

        memcpy(token, observerToken, observerTokenLength);
        tokenLength = observerTokenLength;
        state = 1;
        active = true;

        return 0;
    }
    return 1;
}

void CoAP_Observer::deactivate(){
    active = false;
}

int CoAP_Observer::getObserverInfo(IPAddress *observerIP, uint16_t *observerPort, uint8_t *observerToken, uint8_t *observerTokenLength, uint16_t *observerState) {
    if(!active){
        return 1;
    }

    *observerIP = ip;
    *observerPort = port;

    if (sizeof(observerToken) < this->tokenLength) {
        return 2;
    }
    memcpy(observerToken, token, tokenLength);
    *observerTokenLength = tokenLength;

    *observerState = state;
    state++;
    return 0;
}

bool CoAP_Observer::compare(IPAddress observerIP, uint16_t observerPort){
    return observerIP == ip && observerPort == port;
}

bool CoAP_Observer::canBeActivated(IPAddress observerIP, uint16_t observerPort){
    return !active || (observerIP == ip && observerPort == port);
}