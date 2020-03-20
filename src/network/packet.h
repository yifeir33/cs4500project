#pragma once

#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <netinet/ip.h>

#include "util/object.h"

#define DATA_MAX        256
#define PACKET_MAX_SIZE (1 + 2 + DATA_MAX) // Type + Length + DATA_MAX

// types
#define REGISTER        0x01
#define DEREGISTER      0x02
#define CLIENT_UPDATE   0x03
#define CHAR_MSG        0x04
#define ASK_FOR_ID      0x05
#define ID              0x06
#define KEEP_ALIVE      0xFD
#define ERROR_MSG       0xFE
#define SHUTDOWN        0xFF

enum ParseResult {
    ParseError = -1,
    Success = 0,
    Response = 1,
};

class Packet : public Object {
public:
    uint8_t type;
    uint16_t length;
    uint8_t value[DATA_MAX];

    Packet();

    size_t get_size() const;

    int pack(uint8_t *buffer, size_t buflen) const;

    size_t unpack(uint8_t *buffer, size_t buflen);

    size_t hash() const override;

    bool equals(const Object* other) const override;

    Object *clone() const override;
};
