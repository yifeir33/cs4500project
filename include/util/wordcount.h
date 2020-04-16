#pragma once

#include <memory>
#include <unordered_map>

#include "util/application.h"
#include "data/dataframe.h"
#include "data/kvstore.h"

#define DEFAULT_IP   "127.0.0.1"
#define DEFAULT_SERVER_PORT 8001

/**************************************************************************
 * WordCount  ::
 * This is an extended class of the application. The function of this class is
 * to read from a file, reads it and count the words.
 */
class WordCount : public Application {
private:
    /**
     * The mode of this word counter contains the following 4 modes:
     * None mode, server mode, reader mode, and a counter mode
     */
    enum Mode {
        NONE,
        SERVER,
        READER,
        COUNTER
    };
    /*
     * This is a rower class to help count the number on each row.
     */
    class CounterRower : public Rower {
    private:
        std::unordered_map<std::string, size_t> _word_map;

    public:
        CounterRower();

        /* the rower is taking the row that is going to be edited*/
        bool accept(Row& r) override;

        /** Once traversal of the data frame is complete the rowers that were
     split off will be joined.  There will be one join per split. The
     original object will be the last to be called join on. The join method
     is reponsible for cleaning up memory. */
        void join(std::shared_ptr<Rower> other) override;

        /**
         * print the rower.
         */
        void print() const;

        /**
         * clone the rower.
         * @return return the cloned result.
         */
        std::shared_ptr<Object> clone() const override;

        /**
         * return the hash of this class.
         * @return the hash of this object
         */
        size_t hash() const override;

        /**
         * check if is equal to the other object
         * @param other compare this object to the others.
         * @return the boolean if this object is equal to the other object
         */
        bool equals(const Object *other) const override;
    };

    /*
     * currecnt ip for this appliaction.
     */
    const char *_ip;
    /*
     * server's ip for this appliaction.
     */
    const char *_server_ip;
    /*
     * The file name that this word counter is working on.
     */
    const char *_filename;
    /**
     * The port of the server
     */
    in_port_t _server_port;
    /*
     * The mode of this word counter.
     */
    Mode _mode;
    /**
     * The key that is being tracked with the dataframe.
     */
    KVStore::Key _key;

    /**
     * reads in the file as a column, and add into dataframe
     */
    void _read_in_file() const;

    /**
     * assigning the ip and server ip.
     */
    void _server();

    /**
     * Using the count rower to count on each row.
     */
    void _counter();

    /**
     * With the provided ip and server ip, it reads the data from there.
     */
    void _reader();

protected:
    /**
     * run the application, calls on the reader, counter, server, depends on the mode that it is.
     */
    virtual void _run() override;

    /**
    * Explain the usage of each with providing the flag name.
    * @param bin_name the flag that client wants to get explained.
    */
    void _help(const char* bin_name) const override;

public:
    /**
     * Create an empty word count application.
     */
    WordCount();

    /**
    * This method will take inputs from the client, it will switch the mode based on
    * the command that is provided, for the current application's help method.
    * @param argc the postion of the argument
    * @param argv the character of the argument
    */
    void parse_arguments(int argc, char** argv) override;
};
