#pragma once

class Application {
protected:
    virtual void _run() = 0;

    virtual void _help(const char* bin_name) const = 0;

public:
    Application() {};

    virtual ~Application() {};

    virtual void parse_arguments(int argc, char** argv) = 0;

    void start() { _run(); };
};
