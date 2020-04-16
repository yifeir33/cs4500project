#pragma once

#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <netinet/ip.h>
#include <vector>

#include "util/object.h"


/** Class which represents a TLV packet used to simplify the logic
 * of reading from and writing a packet over the network. */
class Packet : public Object {
private:
    /** Private value storing the remaing length of the packet that needs to
     * be read in, for when the read buffer is too small to read in the entire packet
     * at once. Allows for multiple calls to partial_unpack to be used to read
     * in packets where the read buffer is too small. */
    uint64_t remaining_len;

public:
    /** Enum representing the type field of the packet. */
    enum Type : uint8_t {
        /** There is no type associated with the packet; used when
         * the packet is instantiated before meaning is assigned. */
        NONE           = 0x00,
        /** The packet's meaning is to register with the server node. */
        REGISTER       = 0x01,
        /** The packet's meaning is to deregister with the server node. */
        DEREGISTER     = 0x02,
        /** The packet's meaning is a list of clients on the network from
         * the server node. */
        CLIENT_UPDATE  = 0x03,
        /** The packet contains a string. */
        CHAR_MSG       = 0x04,
        /** The packet is request for the other end to identify themselves. */
        ASK_FOR_ID     = 0x05,
        /** The packet identifies the sender to the receiver (often by address) */
        ID             = 0x06,
        /** The packet is a request for a value from the receiver's local KVStore. */
        VALUE_REQUEST  = 0x07,
        /** The packet is a response for a value from the sender's local KVStore. */
        VALUE_RESPONSE = 0x08,
        /** The packet is a list of keys whose values are stored in the sender's
         * local KVStore */
        KEY_LIST       = 0x09,
        /** Packet used to update a watchdog timer. */
        KEEP_ALIVE     = 0xFC,
        /** Packet containing a string representing an error message. */
        ERROR_MSG      = 0xFD,
        /** Packet signifying the connection is shutting down. */
        SHUTDOWN       = 0xFE,
        /** Packet signifying either a request for the entire network to shut
         * down (from a client), or that the network IS shutting down (from
         * a server). */
        TEARDOWN       = 0XFF
    };
    /** The type field of the packet. */
    Type type;
    /** The value field of the packet, in byte form. As a vector, it also
     * stores the length field. */
    std::vector<uint8_t> value;

    /** Constructors. */
    Packet();
    Packet(const Packet&) = default;
    Packet(Packet&&) = default;

    /** Returns the length of the packet. */
    size_t get_size() const;

    /** Packs the packet into TLV form and returns the vector containing that
     * representation. */
    std::vector<uint8_t> pack() const;

    /** Given a pointer to a buffer containing a packet and the length of said
     * buffer, attempts to read the data from the buffer into this instance of
     * a packet. Returns the number of bytes read in on success, and -1 on failure. 
     * If the packet fails to read in the entire packet, the internals are reset. */
    int unpack(uint8_t *buffer, size_t buflen);

    /** Given a pointer to a buffer containng a packet and the length of said buffer,
     * attempts to read in as much of the data as that buffer contains of the packet,
     * storing the amount left to be read in. Returns the number of bytes read in. To
     * read in a packet larger than the buffer storing it, this method should be
     * called multiple times, with the data being read in inbetween calls so
     * that it is continous, even though it is longer than the buffer. */
    size_t partial_unpack(bool front, uint8_t *buffer, size_t buflen, bool& finished);

    /** Returns the hashcode of this packet. */
    size_t hash() const override;

    /** Tests for equality by values between this packet and another. */
    bool equals(const Object* other) const override;

    /** Clones this packet and returns a pointer to the clone. */
    std::shared_ptr<Object> clone() const override;
};
