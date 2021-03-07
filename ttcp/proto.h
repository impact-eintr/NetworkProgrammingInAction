#ifndef PROTO_H__
#define PROTO_H__

#include <stdint.h>

struct SessionMessage{
    int32_t number;
    int32_t length;
}__attribute__((__packed__));

struct payloadMessage{
    int32_t length;
    char data[0];
}__attribute__((__packed__));

#endif
