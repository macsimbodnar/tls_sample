#pragma once
#include <iostream>                     // Used for cout

// GLOBAL DEFINES
#define PORT 69

// LOGS
#define LOG_I std::cout                 // Start log 
#define END_I std::endl                 // End log

#define LOG_S LOG_I << "\033[92m"       // Success green log
#define END_S "\033[37m" << END_I       // End success green log

#define LOG_W LOG_I << "\033[33m"       // Warning orange log
#define END_W "\033[37m" << END_I       // End warning orange log

#define LOG_E LOG_I << "\033[31m"       // Error red log
#define END_E "\033[37m" << END_I       // End Error red log

#define LOG_SSL_STACK() {\
    char buf[256];\
\
    while (ERR_peek_error()) {\
        ERR_error_string_n(ERR_get_error(), buf, sizeof(buf));\
        LOG_E << "openssl error: " << buf << END_E;\
    }\
}