#include <iostream>         // Used for cout
#include <errno.h>          // Uset for errno
#include <string.h>         // Used for strerror
#include <netinet/in.h>     // Used for sockaddr_in
#include <sys/socket.h>     // Used for ... You gess, socket
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "defines.hpp"      // Contains the log macros and other defines


int main(int argc, char **argv) {

    /**
     * Create a socket file descriptor
     *
     * socket       returns the file descrip#include <netinet/in.h>tor for the new socket and -a in case of error
     * PF_INET      set the protocol to IP protocol family
     * SOCK_STREAM  specify that we want to use TCP. Use SOCK_DGRAM for UDP
     * 0            this parameter set the protocol. If 0 than the protocol is chosen automatically
     */
    if (socket(PF_INET, SOCK_STREAM, 0) == -1) {
        const int err = errno;
        LOG_E << "Failed to init the socket: " << strerror(err) << END_E;
    }

    // struct sockaddr_in addr;



    // Load error strings to get human readable results in case of error
    SSL_load_error_strings();
    // Clear the error queue before start
    ERR_clear_error();

    return 0;
}