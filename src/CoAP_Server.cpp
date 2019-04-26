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
    int receivedUdpPacketLength = Udp.parsePacket();

    if (receivedUdpPacketLength) {
        receivedUdpPacketLength =
                Udp.read(receiveUdpBuffer,
                         receivedUdpPacketLength > MAX_UDP_BUFFER_SIZE ? MAX_UDP_BUFFER_SIZE : receivedUdpPacketLength);

        CoAP_Packet request;
        request.parse(receiveUdpBuffer, receivedUdpPacketLength);

        request.getResourceUri(resourceUriBuf, &resourceUriSize);

        String uri = String(resourceUriBuf);

        CoAP_Packet response;
        busyResponseMemorySize = 0;
        response.header.ver = COAP_VERSION;
        response.header.id = request.header.id;
        response.header.tkl = request.header.tkl;
        response.token = request.token;
        response.optionsNumber = 0;

        switch (request.header.code) {
            case COAP_EMPTY:
                if (request.header.type == COAP_CON) {
                    handlePing(&response);
                    sendPacket(&response, Udp.remoteIP(), Udp.remotePort());
                } else if (request.header.type == COAP_RESET) {
                    for (int i = 0; i < MAX_RESOURCES; i++) {
                        if (resources[i].isActive()) {
                            resources[i].removeObserver(Udp.remoteIP(), Udp.remotePort());
                        }
                    }
                } else {
                    if (request.header.type == COAP_ACK) {
                        // TODO "COAP_ACK received handle"
                    } else {
                        // TODO "COAP_EMPTY handle error"
                    }
                }
                break;

            case COAP_GET:
                responseTypeHandle(&request, &response);
                if (uri.equalsIgnoreCase(String(DISCOVERY_RESOURCE_PATH))) {
                    handleResourceDiscovery(&response);
                } else {
                    handleGet(uri, &response);

                    for (int i = 0; i < request.optionsNumber; i++) {
                        if (request.options[i].num == COAP_OBSERVE) {
                            int index = getResourceIndex(uri);
                            if (index >= 0) {
                                resources[index].addObserver(Udp.remoteIP(), Udp.remotePort(),
                                                             (uint8_t *) request.token.p, request.token.len);
                            }
                            break;
                        }
                    }
                }
                sendPacket(&response, Udp.remoteIP(), Udp.remotePort());
                break;

            case COAP_PUT:
                responseTypeHandle(&request, &response);
                handlePut(uri, &request, &response);
                sendPacket(&response, Udp.remoteIP(), Udp.remotePort());
                break;

            case COAP_POST:
                responseTypeHandle(&request, &response);
                handlePost(uri, &request, &response);
                sendPacket(&response, Udp.remoteIP(), Udp.remotePort());
                break;

            case COAP_DELETE:
                response.header.type = COAP_NONCON;
                handleDelete(uri, &response);
                sendPacket(&response, Udp.remoteIP(), Udp.remotePort());
                break;

            default:
                // TODO "Unknown message code handle."
                break;
        }
    }
    notificationLoop();
}

void CoAP_Server::handleResourceDiscovery(CoAP_Packet *response) {
    String strRes;
    for (int i = 0; i < MAX_RESOURCES; i++) {
        if (resources[i].isActive()) {
            strRes += "</";
            strRes += resources[i].uri;
            strRes += ">;";
            strRes += resources[i].uri;
            strRes += ";rt=";
            strRes += "\"";
            strRes += "observe";
            strRes += "\"";

            strRes += ";";
            strRes += "ct=";
            strRes += "0";
            strRes += ";";
            strRes += "title=\"";
            strRes += "observable resource";
            strRes += "\"";
            strRes += ",";
        }
    }
    setMessageContent(response, (uint8_t *) strRes.c_str(), strRes.length());
}

void CoAP_Server::handleGet(String uri, CoAP_Packet *response) {
    int resourceIndex = getResourceIndex(uri);
    if (resourceIndex >= 0) {
        resources[resourceIndex].getCallback();
        setMessageContent(response, resources[resourceIndex].data, resources[resourceIndex].dataSize);
    } else {
        response->payload.p = NULL;
        response->payload.len = 0;
        response->header.code = COAP_NOT_FOUND;
    }
}

void CoAP_Server::handlePut(String uri, CoAP_Packet *request, CoAP_Packet *response) {
    int resourceIndex = getResourceIndex(uri);
    if (resourceIndex >= 0) {
        resources[resourceIndex].putCallback();
        response->payload.p = NULL;
        response->payload.len = 0;
        response->header.code = COAP_METHOD_NOT_ALLOWED;
    } else {
        resourceRegister(uri, (uint8_t *) request->payload.p, request->payload.len);

        setMessageContent(response, request->payload.p, request->payload.len);
        response->header.code = COAP_CREATED;
    }
}

void CoAP_Server::handlePost(String uri, CoAP_Packet *request, CoAP_Packet *response) {
    int resourceIndex = getResourceIndex(uri);
    if (resourceIndex >= 0) {
        updateResource(uri, (uint8_t *) request->payload.p, request->payload.len);
    } else {
        resourceRegister(uri, (uint8_t *) request->payload.p, request->payload.len);
    }
    resources[resourceIndex].postCallback();

    setMessageContent(response, (uint8_t *) request->payload.p, request->payload.len);
    response->header.code = COAP_CHANGED;
}

void CoAP_Server::handleDelete(String uri, CoAP_Packet *response) {
    int resourceIndex = getResourceIndex(uri);
    if (resourceIndex >= 0) {
        resources[resourceIndex].deleteCallback();
        resources[resourceIndex].deactivate();
        response->payload.p = NULL;
        response->payload.len = 0;
        response->header.code = COAP_DELETED;
    } else {
        response->payload.p = NULL;
        response->payload.len = 0;
        response->header.code = COAP_NOT_FOUND;
    }
    response->optionsNumber = 0;
}

void CoAP_Server::handlePing(CoAP_Packet *response) {
    response->header.type = COAP_RESET;
    response->header.code = COAP_EMPTY_MESSAGE;
    response->optionsNumber = 0;
    response->payload.len = 0;
    response->payload.p = NULL;
}

int CoAP_Server::updateResource(String uri, uint8_t *content, size_t bufSize) {
    int index = getResourceIndex(uri);
    if (index >= 0 && resources[index].isActive()) {
        return resources[index].updateResource(content, bufSize);
    }
    return 1;
}

int CoAP_Server::getResourceValueString(String uri, char *buffer, size_t bufSize, size_t *outputSize) {
    int index = getResourceIndex(uri);
    if (index >= 0 && resources[index].isActive() && bufSize > resources[index].dataSize) {
        *outputSize = resources[index].dataSize;
        memcpy(buffer, (char *) resources[index].data, resources[index].dataSize);
        buffer[*outputSize] = '\0';
        return 0;
    }
    return 1;
}

int CoAP_Server::updateResource(String uri, int value) {
    int index = getResourceIndex(uri);
    if (index >= 0 && resources[index].isActive()) {
        uint8_t bytes[4];
        bytes[0] = ((uint32_t) value >> 24) & 0xFF;
        bytes[1] = ((uint32_t) value >> 16) & 0xFF;
        bytes[2] = ((uint32_t) value >> 8) & 0xFF;
        bytes[3] = (uint32_t) value & 0xFF;
        return resources[index].updateResource(bytes, 4);
    }
    return 1;
}

int CoAP_Server::updateResource(String uri, float value) {
    int index = getResourceIndex(uri);
    if (index >= 0 && resources[index].isActive()) {
        uint8_t bytes[4];
        bytes[0] = ((uint32_t) value >> 24) & 0xFF;
        bytes[1] = ((uint32_t) value >> 16) & 0xFF;
        bytes[2] = ((uint32_t) value >> 8) & 0xFF;
        bytes[3] = (uint32_t) value & 0xFF;
        return resources[index].updateResource(bytes, 4);
    }
    return 1;
}

int CoAP_Server::getResourceValueInt(String uri) {
    int index = getResourceIndex(uri);
    if (index >= 0 && resources[index].isActive()) {
        return resources[index].getIntValue();
    }
    return 0;
}

float CoAP_Server::CoAP_Server::getResourceValueFloat(String uri) {
    int index = getResourceIndex(uri);
    if (index >= 0 && resources[index].isActive()) {
        return resources[index].getFloatValue();
    }
    return 0.0;
}

void CoAP_Server::notificationLoop() {
    unsigned long timestamp = millis();
    if (timestamp - previousMillisTimestamp >= NOTIFICATION_INTERVAL) {
        previousMillisTimestamp = timestamp;

        CoAP_Packet response;
        busyResponseMemorySize = 0;
        response.header.ver = COAP_VERSION;
        response.header.code = COAP_CONTENT;

        for (int o = 0; o < MAX_RESOURCES; o++) {
            if (resources[o].shouldNotifyObservers()) {
                resources[o].setNotified();

                response.header.type = resources[o].getNotificationMessageType();
                response.header.id = resources[o].getNotificationMessageId();
                response.optionsNumber = 0;

                response.options[response.optionsNumber].num = COAP_OBSERVE;
                response.options[response.optionsNumber].buf.p = &responseUdpBuffer[busyResponseMemorySize];
                busyResponseMemorySize += OBSERVER_STATE_SIZE;
                response.options[response.optionsNumber].buf.len = OBSERVER_STATE_SIZE;
                response.optionsNumber++;

                response.options[response.optionsNumber].buf.p = (uint8_t *) resources[o].uri.c_str();
                response.options[response.optionsNumber].buf.len = resources[o].uri.length();
                response.options[response.optionsNumber].num = COAP_URI_PATH;
                response.optionsNumber++;

                setMessageContent(&response, resources[o].data, resources[o].dataSize);

                IPAddress observerIP;
                uint16_t observerPort;
                uint16_t observerState;
                uint8_t token[MAX_TOKEN_SIZE];
                uint8_t tokenLength;

                for (int i = 0; i < MAX_OBSERVERS; i++) {
                    if (resources[o].observers[i].getObserverInfo(
                            &observerIP, &observerPort, token, &tokenLength, &observerState) == 0) {
                        response.header.tkl = tokenLength;
                        response.token.p = token;
                        response.token.len = tokenLength;
                        response.options[0].buf.p[0] = (uint8_t)(observerState >> 8);
                        response.options[0].buf.p[1] = (uint8_t) observerState;

                        sendPacket(&response, observerIP, observerPort);
                    }
                }
            }
        }
    }
}

void CoAP_Server::sendPacket(CoAP_Packet *packet, IPAddress ip, int port) {
    uint8_t outputBuffer[MAX_UDP_BUFFER_SIZE];
    size_t outputMessageSize;
    (*packet).serialize(outputBuffer, &outputMessageSize);
    Udp.beginPacket(ip, port);
    Udp.write(outputBuffer, outputMessageSize);
    Udp.endPacket();
}

int CoAP_Server::getResourceIndex(String uri) {
    for (int i = 0; i < MAX_RESOURCES; i++) {
        if (resources[i].isActive() && resources[i].uri.equalsIgnoreCase(uri)) {
            return i;
        }
    }
    return 1;
}

bool CoAP_Server::resourceRegister(String uri, uint8_t *content, size_t bufSize, Callback resourceCallback) {
    for (int i = 0; i < MAX_RESOURCES; i++) {
        if (!resources[i].isActive()) {
            resources[i].initialize(uri, content, bufSize, resourceCallback, resourceCallback, resourceCallback,
                                    resourceCallback);
            return true;
        }
    }
    return false;
}

bool CoAP_Server::resourceRegister(String uri, uint8_t *content, size_t bufSize) {
    for (int i = 0; i < MAX_RESOURCES; i++) {
        if (!resources[i].isActive()) {
            resources[i].initialize(uri, content, bufSize);
            return true;
        }
    }
    return false;
}

bool CoAP_Server::resourceRegister(String uri, uint8_t *content, size_t bufSize, Callback getCallback,
                                   Callback postCallback) {
    for (int i = 0; i < MAX_RESOURCES; i++) {
        if (!resources[i].isActive()) {
            resources[i].initialize(uri, content, bufSize, getCallback, postCallback);
            return true;
        }
    }
    return false;
}


void CoAP_Server::responseTypeHandle(CoAP_Packet *request, CoAP_Packet *response) {
    response->header.type = request->header.type;
    response->header.type = request->header.type;
}

int CoAP_Server::setMessageContent(CoAP_Packet *response, uint8_t *content, size_t contentLength) {
    if (contentLength > COAP_MAX_PAYLOAD_SIZE) {
        return 1;
    }

    response->payload.p = &responseUdpBuffer[busyResponseMemorySize];
    memcpy(response->payload.p, content, contentLength);
    response->payload.len = contentLength;
    busyResponseMemorySize += contentLength;

    response->header.code = COAP_CONTENT;

    response->options[response->optionsNumber].buf.p = &responseUdpBuffer[busyResponseMemorySize];
    busyResponseMemorySize += 2;
    response->options[response->optionsNumber].buf.p[0] = ((uint16_t) COAP_TEXT_PLAIN & 0xFF00) >> 8;
    response->options[response->optionsNumber].buf.p[1] = ((uint16_t) COAP_TEXT_PLAIN & 0x00FF);
    response->options[response->optionsNumber].buf.len = 2;
    response->options[response->optionsNumber].num = COAP_CONTENT_FORMAT;
    response->optionsNumber++;

    return 0;
}