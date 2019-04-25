//
// Created by rafstach on 07/04/2019.
// Structures and enum definition based on nodemcu core which link states in README.md
//

#ifndef COAP_ESP8266_COAP_PACKET_H
#define COAP_ESP8266_COAP_PACKET_H

#include <Arduino.h>
#include <stdint.h>
#include <sys/types.h>
#include <string.h>
//CoAP Packet configuration
#define MAX_OPTIONS 16
#define MAX_URI_SIZE 100
#define MAX_PAYLOAD_SIZE 350
#define COAP_VERSION 1
#define COAP_HEADER_SIZE 4
#define COAP_OPTION_HEADER_SIZE 1
#define COAP_PAYLOAD_MARKER 0xFF
///////////////////////

//http://tools.ietf.org/html/rfc7252

//coap message types
typedef enum {
    COAP_CON = 0,
    COAP_NONCON = 1,
    COAP_ACK = 2,
    COAP_RESET = 3,
} COAP_TYPE;

//coap method values
typedef enum {
    COAP_EMPTY = 0,
    COAP_GET = 1,
    COAP_POST = 2,
    COAP_PUT = 3,
    COAP_DELETE = 4,
} COAP_METHOD;

//coap response values
typedef enum {
    COAP_EMPTY_MESSAGE = 0,
    COAP_CREATED = 65,
    COAP_DELETED = 66,
    COAP_VALID = 67,
    COAP_CHANGED = 68,
    COAP_CONTENT = 69,
    COAP_BAD_REQUEST = 128,
    COAP_UNAUTHORIZED = 129,
    COAP_BAD_OPTION = 130,
    COAP_FORBIDDEN = 131,
    COAP_NOT_FOUND = 132,
    COAP_METHOD_NOT_ALLOWED = 133,
    COAP_PRECONDITION_FAILED = 140,
    COAP_REQUEST_ENTITY_TOO_LARGE = 141,
    COAP_UNSUPPORTED_CONTENT_FORMAT = 143,
    COAP_INTERNAL_SERVER_ERROR = 160,
    COAP_NOT_IMPLEMENTED = 161,
    COAP_BAD_GATEWAY = 162,
    COAP_SERVICE_UNAVALIABLE = 163,
    COAP_GATEWAY_TIMEOUT = 164,
    COAP_PROXYING_NOT_SUPPORTED = 165
} COAP_RESPONSE_CODE;

//coap option values
typedef enum {
    COAP_IF_MATCH = 1,
    COAP_URI_HOST = 3,
    COAP_E_TAG = 4,
    COAP_IF_NONE_MATCH = 5,
    COAP_URI_PORT = 7,
    COAP_LOCATION_PATH = 8,
    COAP_URI_PATH = 11,
    COAP_CONTENT_FORMAT = 12,
    COAP_MAX_AGE = 14,
    COAP_URI_QUERY = 15,
    COAP_ACCEPT = 17,
    COAP_LOCATION_QUERY = 20,
    COAP_BLOCK_2 = 23,
    COAP_PROXY_URI = 35,
    COAP_PROXY_SCHEME = 39,
    COAP_OBSERVE = 6
} COAP_OPTION_NUMBER;

//coap content format types
typedef enum {
    COAP_TEXT_PLAIN = 0,
    COAP_APPLICATION_LINK_FORMAT = 40,
    COAP_APPLICATION_XML = 41,
    COAP_APPLICATION_OCTET_STREAM = 42,
    COAP_APPLICATION_EXI = 47,
    COAP_APPLICATION_JSON = 50
} COAP_CONTENT_TYPE;

typedef enum
{
    COAP_ERR_NONE = 0,
    COAP_ERR_HEADER_TOO_SHORT = 1,
    COAP_ERR_VERSION_NOT_1 = 2,
    COAP_ERR_TOKEN_TOO_SHORT = 3,
    COAP_ERR_OPTION_TOO_SHORT_FOR_HEADER = 4,
    COAP_ERR_OPTION_TOO_SHORT = 5,
    COAP_ERR_OPTION_OVERRUNS_PACKET = 6,
    COAP_ERR_OPTION_TOO_BIG = 7,
    COAP_ERR_OPTION_LEN_INVALID = 8,
    COAP_ERR_BUFFER_TOO_SMALL = 9,
    COAP_ERR_UNSUPPORTED = 10,
    COAP_ERR_OPTION_DELTA_INVALID = 11,
} COAP_ERROR_T;

///////////////////////

//http://tools.ietf.org/html/rfc7252#section-3
typedef struct PacketHeader {
    uint8_t ver;                /* CoAP version number */
    uint8_t type;               /* CoAP Message Type */
    uint8_t tkl;                /* Token length: indicates length of the Token field */
    uint8_t code;               /* CoAP status code. Can be request (0.xx), success reponse (2.xx),client error response (4.xx), or rever error response (5.xx). For possible values, see http://tools.ietf.org/html/rfc7252#section-12.1 */
    uint16_t id;
} PacketHeader;

typedef struct PacketBuffer {
    const uint8_t *p;
    size_t len;
} PacketBuffer;

typedef struct PacketOption {
    uint8_t num;                /* Option number. See http://tools.ietf.org/html/rfc7252#section-5.10 */
    PacketBuffer buf;           /* Option value */
} PacketOption;

class CoAP_Packet {
public:
    PacketHeader header;                     /* Header of the packet */
    PacketBuffer token;                      /* Token value, size as specified by hdr.tkl */
    uint8_t optionsNumber;                   /* Number of options */
    PacketOption options[MAX_OPTIONS];       /* Options of the packet. For possible entries see http://tools.ietf.org/html/rfc7252#section-5.10 */

    PacketBuffer payload;                    /* Payload carried by the packet */

    int parse(const uint8_t *buf, size_t buflen);
    int getResourceUri(char* uribuf, size_t* urilen);
    int serialize(uint8_t *buf, size_t* buflen);

    uint8_t contentParseBuff[MAX_PAYLOAD_SIZE];
private:
    int parseHeader(const uint8_t *buf, size_t buflen);
    int parseToken(const uint8_t *buf, size_t buflen);
    int parseOption(PacketOption *option, uint16_t *runningDelta, const uint8_t **buf, size_t buflen);
    int parseOptionsAndPayload(const uint8_t *buf, size_t buflen);
};

#endif //COAP_ESP8266_COAP_PACKET_H
