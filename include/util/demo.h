#pragma once

#include <optional>
#include <arpa/inet.h>

#include "util/application.h"
#include "data/rower.h"

#define DEFAULT_IP   "127.0.0.1"
#define DEFAULT_SERVER_PORT 8001
/**************************************************************************
 * Demo ::
 * This is an application created by extend the application class.
 * This demo class is what required in milestone 3, which contains three functions:
 * produce, count and summarize.
 * 1. Producer:
 * Stores a dataframe constrcuted from an array of doubles,
 * and store a dataframe of the sum of previous DataFrame.
 * 2. Counter:
 * Waits and retrieves the large dataframe of doubles from the producer.
 * After finding it, sum the large dataframe and store that sum as a local dataframe.
 * 3. Summarizer:
 * Retreive the sum of the producer (the expected value for the sum of the initially
 * constructed dataframe), and the sum the counter found, and compares them to ensure
 * they both have the same value and no corruption on the network occured.*/
class Demo : public Application {
private:
    /*
     * assigning the application with a mode, which could be among the 5 following types:
     * none, server, producer, counter and summarizer.
     */
    enum Mode {
        NONE,
        SERVER,
        PRODUCER,
        COUNTER,
        SUMMARIZER
    };

    /*
     * This sum rower is used to sum each row/
     */
    class SumRower : public Rower {
    public:
        /* the sum that this rower has calculated so far*/
        double _sum;

        /* creates a new empty sum rower */
        SumRower();

        /* the rower is taking the row that is going to be edited*/
        bool accept(Row& r) override;

        /** Once traversal of the data frame is complete the rowers that were
      split off will be joined.  There will be one join per split. The
      original object will be the last to be called join on. The join method
      is reponsible for cleaning up memory. */
        void join(std::shared_ptr<Rower> other) override;

        /* clone the object */
        std::shared_ptr<Object> clone() const override;

        /* check if the object is equal*/
        bool equals(const Object* other) const override;

        /* return the hash of the function*/
        size_t hash() const override;
    };

    /** The IP of this application */
    const char *_ip;

    /** The IP of server */
    const char *_server_ip;

    /** The port of the server */
    in_port_t _server_port;

    /** the current mode that is app is on */
    Mode _mode;

    /**
     * Stores a dataframe constrcuted from an array of doubles,
     * and store a dataframe of the sum of previous DataFrame.
     */
    void _producer();

    /**
     * Waits and retrieves the large dataframe of doubles from the producer.
     * After finding it, sum the large dataframe and store that sum as a local dataframe.
     */
    void _counter();

    /**
     * Retreive the sum of the producer (the expected value for the sum of the initially
     * constructed dataframe), and the sum the counter found, and compares them to ensure
     * they both have the same value and no corruption on the network occured.
     */
    void _summarizer();

protected:
    /**
     * The extended method from application starts the application
     */
    void _run() override;
    /**
     * Explain the usage of each with providing the flag name.
     * @param bin_name the flag that client wants to get explained.
     */
    void _help(const char* bin_name) const override;

public:
    /**
     * creates an empty demo application.
     */
    Demo();

    /**
     * This method will take inputs from the client, it will switch the mode based on
     * the command that is provided, for the current application's help method.
     * @param argc the postion of the argument
     * @param argv the character of the argument
     */
    void parse_arguments(int argc, char** argv) override;
};
