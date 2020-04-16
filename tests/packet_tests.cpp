#include <cstring> 

#include "catch.hpp"

#include "network/packet.h"

SCENARIO("We can packet and unpack a packet") {
    GIVEN("A Packet containing some data.") {
        Packet packet;
        packet.type = Packet::Type::CHAR_MSG;
        std::string hw("Hello World!");
        packet.value.insert(packet.value.end(), hw.begin(), hw.end());

        THEN("We can serialize and unserialize without data loss") {
            std::vector<uint8_t> serialized = packet.pack();
            Packet unserialized;
            REQUIRE(unserialized.unpack(serialized.data(), serialized.size()) == serialized.size()); 
            REQUIRE(packet.equals(&unserialized));
        }
    }
}

SCENARIO("We can partially unpack a buffer using a packet"){
    GIVEN("Data split accross multiple too short buffers"){
        uint64_t len = 12;
        Packet::Type type = Packet::Type::CHAR_MSG;
        uint8_t b1[13] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 'H', 'E', 'L', 'L'};
        memcpy(b1, &type, sizeof(type)); // write type in
        memcpy(b1 + 1, &len, sizeof(len)); // write len in
        uint8_t b2[8] = {'O', ' ', 'W', 'O', 'R', 'L', 'D', '\0'};
        THEN("We can unpack it in multiple partial unpacks"){
            Packet packet;
            int pos = 0;
            bool finished = false;
            pos = packet.partial_unpack(true, b1, 13, finished);
            REQUIRE(pos == 13);
            REQUIRE(!finished);
            pos += packet.partial_unpack(false, b2, 8, finished);
            REQUIRE(pos == 21);
            REQUIRE(finished);

            REQUIRE(packet.type == Packet::Type::CHAR_MSG);
            REQUIRE(packet.value.size() == 12);
            REQUIRE(strcmp((const char *)packet.value.data(), "HELLO WORLD") == 0);
        }
    }
}
