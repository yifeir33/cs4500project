#pragma once

#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <netinet/ip.h>
#include <vector>

#include "util/object.h"

// types
/* #define REGISTER        0x01 */
/* #define DEREGISTER      0x02 */
/* #define CLIENT_UPDATE   0x03 */
/* #define CHAR_MSG        0x04 */
/* #define ASK_FOR_ID      0x05 */
/* #define ID              0x06 */
/* #define */ 
/* #define KEEP_ALIVE      0xFD */
/* #define ERROR_MSG       0xFE */
/* #define SHUTDOWN        0xFF */


enum ParseResult {
    ParseError = -1,
    Success = 0,
    Response = 1,
};

class Packet : public Object {
private:
    size_t remaining_len;

public:
    enum Type : uint8_t {
        NONE           = 0x00,
        REGISTER       = 0x01,
        DEREGISTER     = 0x02,
        CLIENT_UPDATE  = 0x03,
        CHAR_MSG       = 0x04,
        ASK_FOR_ID     = 0x05,
        ID             = 0x06,
        VALUE_REQUEST  = 0x07,
        VALUE_RESPONSE = 0x08,
        KEY_LIST       = 0x09,
        KEEP_ALIVE     = 0xFD,
        ERROR_MSG      = 0xFE,
        SHUTDOWN       = 0xFF
    };
    Type type;
    std::vector<uint8_t> value;

    Packet();
    Packet(const Packet&) = default;
    Packet(Packet&&) = default;

    size_t get_size() const;

    std::vector<uint8_t> pack() const;

    int unpack(uint8_t *buffer, size_t buflen);

    size_t partial_unpack(bool front, uint8_t *buffer, size_t buflen, bool& finished);

    size_t hash() const override;

    bool equals(const Object* other) const override;

    std::shared_ptr<Object> clone() const override;
};
