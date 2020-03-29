#pragma once

#include <optional>
#include <arpa/inet.h>

#include "util/application.h"
#include "data/rower.h"

#define DEFAULT_IP   "127.0.0.1"
#define DEFAULT_SERVER_PORT 8001

class Demo : public Application {
private:
    enum Mode {
        NONE,
        SERVER,
        PRODUCER,
        COUNTER,
        SUMMARIZER
    };

    class SumRower : public Rower {
    public:
        double _sum;

        SumRower();

        bool accept(Row& r) override;

        void join(std::shared_ptr<Rower> other) override;

        std::shared_ptr<Object> clone() const override;

        bool equals(const Object* other) const override;

        size_t hash() const override;
    };

    const char *_ip;
    const char *_server_ip;
    in_port_t _server_port;
    Mode _mode;

    void _producer();
    void _counter();
    void _summarizer();

protected:
    void _run() override;

    void _help(const char* bin_name) const override;

public:
    Demo();

    void parse_arguments(int argc, char** argv) override;
};
