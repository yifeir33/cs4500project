#pragma once

#include <memory>
#include <unordered_map>

#include "util/application.h"
#include "data/dataframe.h"
#include "data/kvstore.h"

#define DEFAULT_IP   "127.0.0.1"
#define DEFAULT_SERVER_PORT 8001

class WordCount : public Application {
private:
    enum Mode {
        NONE,
        SERVER,
        READER,
        COUNTER
    };

    class CounterRower : public Rower {
    private:
        std::unordered_map<std::string, size_t> _word_map;

    public:
        CounterRower();

        bool accept(Row& r) override;

        void join(std::shared_ptr<Rower> other) override;

        void print() const;

        std::shared_ptr<Object> clone() const override;

        size_t hash() const override;

        bool equals(const Object *other) const override;
    };

    const char *_ip;
    const char *_server_ip;
    const char *_filename;
    in_port_t _server_port;
    Mode _mode;
    KVStore::Key _key;


    void _read_in_file() const;

    void _server();

    void _counter();

    void _reader();

    void _summarizer();
protected:
    virtual void _run() override;

    void _help(const char* bin_name) const override;

public:
    WordCount();

    void parse_arguments(int argc, char** argv) override;
};
