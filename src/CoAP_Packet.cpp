//
// Created by rafstach on 07/04/2019.
//

#include "CoAP_Packet.h"

//Parse idea comes from nodemcu core, for more details go to README.md
int CoAP_Packet::parse(const uint8_t *buf, size_t buflen) {
    int rc;

    if (0 != (rc = parseHeader(buf, buflen)))
        return rc;

    if (0 != (rc = parseToken(buf, buflen)))
        return rc;
    optionsNumber = MAX_OPTIONS;
    if (0 != (rc = parseOptionsAndPayload(buf, buflen)))
        return rc;

    return 0;
}

int CoAP_Packet::parseHeader(const uint8_t *buf, size_t buflen) {
    if (buflen < 4)
        return COAP_ERR_HEADER_TOO_SHORT;
    header.ver = (buf[0] & 0xC0) >> 6;
    if (header.ver != COAP_VERSION)
        return COAP_ERR_VERSION_NOT_1;
    header.type = (buf[0] & 0x30) >> 4;
    header.tkl = buf[0] & 0x0F;
    header.code = buf[1];
    header.id = ((uint16_t) buf[3] << 8) | buf[2];

    return 0;
}

int CoAP_Packet::parseToken(const uint8_t *buf, size_t buflen) {
    if (header.tkl == 0) {
        token.p = NULL;
        token.len = 0;
        return 0;
    } else if (header.tkl <= 8) {
        if (4U + header.tkl > buflen)
            return COAP_ERR_TOKEN_TOO_SHORT;
        token.p = buf + 4;
        token.len = header.tkl;
        return 0;
    } else {
        return COAP_ERR_TOKEN_TOO_SHORT;
    }
}

int CoAP_Packet::parseOption(PacketOption *option, uint16_t *runningDelta, const uint8_t **buf, size_t buflen) {
    const uint8_t *p = *buf;
    uint8_t headlen = 1;
    uint16_t len, delta;

    if (buflen < headlen)
        return COAP_ERR_OPTION_TOO_SHORT_FOR_HEADER;

    delta = (p[0] & 0xF0) >> 4;
    len = p[0] & 0x0F;

    if (delta == 13) {
        headlen++;
        if (buflen < headlen)
            return COAP_ERR_OPTION_TOO_SHORT_FOR_HEADER;
        delta = p[1] + 13;
        p++;
    } else if (delta == 14) {
        headlen += 2;
        if (buflen < headlen)
            return COAP_ERR_OPTION_TOO_SHORT_FOR_HEADER;
        delta = ((p[1] << 8) | p[2]) + 269;
        p += 2;
    } else if (delta == 15)
        return COAP_ERR_OPTION_DELTA_INVALID;

    if (len == 13) {
        headlen++;
        if (buflen < headlen)
            return COAP_ERR_OPTION_TOO_SHORT_FOR_HEADER;
        len = p[1] + 13;
        p++;
    } else if (len == 14) {
        headlen += 2;
        if (buflen < headlen)
            return COAP_ERR_OPTION_TOO_SHORT_FOR_HEADER;
        len = ((p[1] << 8) | p[2]) + 269;
        p += 2;
    } else if (len == 15)
        return COAP_ERR_OPTION_LEN_INVALID;

    if ((p + 1 + len) > (*buf + buflen))
        return COAP_ERR_OPTION_TOO_BIG;

    option->num = delta + *runningDelta;
    option->buf.p = p + 1;
    option->buf.len = len;

    *buf = p + 1 + len;
    *runningDelta += delta;

    return 0;
}

int CoAP_Packet::parseOptionsAndPayload(const uint8_t *buf, size_t buflen) {
    size_t optionIndex = 0;
    uint16_t delta = 0;
    const uint8_t *p = buf + 4 + header.tkl;
    const uint8_t *end = buf + buflen;
    int rc;

    if (p > end)
        return COAP_ERR_OPTION_OVERRUNS_PACKET;

    // 0xFF is payload marker
    while ((optionIndex < optionsNumber) && (p < end) && (*p != COAP_PAYLOAD_MARKER)) {
        if (0 != (rc = parseOption(&options[optionIndex], &delta, &p, end - p)))
            return rc;
        optionIndex++;
    }
    optionsNumber = optionIndex;

    if (p + 1 < end && *p == COAP_PAYLOAD_MARKER) {
        payload.p = p + 1;
        payload.len = end - (p + 1);
    } else {
        payload.p = NULL;
        payload.len = 0;
    }

    return 0;
}

int CoAP_Packet::getResourceUri(char* uribuf, size_t* urilen){
    int index = 0;
    for (int i = 0; i < optionsNumber; i++) {
        if (options[i].num == COAP_URI_PATH && options[i].buf.len > 0) {
            if (index > 0) {
                uribuf[index++] = '/';
            }
            memcpy(&uribuf[index], options[i].buf.p, options[i].buf.len);
            index+=options[i].buf.len;
        }
    }
    if( index > 0){
        uribuf[index] = '\0';
        *urilen = index;
        return 0;
    } else {
        *urilen = 0;
        return 1;
    }
}

int CoAP_Packet::serialize(uint8_t *buf, size_t* buflen){
    size_t index = 0;
    buf[index++] = (header.ver << 6) | (header.type << 4) | (header.tkl);
    buf[index++] = header.code;
    memcpy(&buf[index], &header.id, 2);
    index +=2;
    memcpy(&buf[index], token.p, token.len);
    index += token.len;
    int lastOptNumber = 0;
    for(int i =0 ; i< optionsNumber ; i++){
        if(options[i].num < lastOptNumber)
            return 1;
        buf[index++] = ((options[i].num - lastOptNumber) << 4 ) | options[i].buf.len;
        lastOptNumber = options[i].num;
        memcpy(&buf[index], options[i].buf.p, options[i].buf.len);
        index+= options[i].buf.len;
    }
    if(payload.len > 0){
        buf[index++] = COAP_PAYLOAD_MARKER;
        memcpy(&buf[index], payload.p, payload.len);
        index += payload.len;
    }
    *buflen = index;
    return 0;
}