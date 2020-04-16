#pragma once

#define CLIENT_DEFAULT_IP_STR  "127.0.0.1"
#define SERVER_PORT            8080

/** Convience header that includes the code for both servers and clients. */
#include "network/server.h"
#include "network/client.h"
