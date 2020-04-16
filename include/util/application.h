#pragma once

/**************************************************************************
 * Application ::
 * This is an abstract class of all the applications, the application includes
 * a run method. This abstract class defines methods overriden in subclasses.
 * Each subclass is an specific application contains the method that an application
 * has to start with, which is run() */
class Application {
protected:
    /** starts the application */
    virtual void _run() = 0;
    /**
    * Explain the usage of each with providing the flag name.
    * @param bin_name the flag that client wants to get explained.
    */
    virtual void _help(const char* bin_name) const = 0;

public:
    /*
     * Create an empty application, with the constructor
     */
    Application() {};

    virtual ~Application() {};
    /**
     * This method will take inputs from the client, it will switch the mode based on
     * the command that is provided, for the current application's help method.
     * @param argc the postion of the argument
     * @param argv the character of the argument
     */
    virtual void parse_arguments(int argc, char** argv) = 0;
    /*
     * This method is getting called when we start this app,
     * which is calling the protected method run().
     */
    void start() { _run(); };
};
