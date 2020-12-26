#include <errno.h>          // Uset for errno
#include <string.h>         // Used for strerror
#include <netinet/in.h>     // Used for sockaddr_in
#include <sys/socket.h>     // Used for ... You gess, socket
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "defines.hpp"      // Contains the log macros and other defines


#define PORT 69

int main(int argc, char **argv) {

    int listen_descriptor;
    int work_descriptor;
    int optval = 1;
    const unsigned short port = PORT;

    LOG_S << "Starting server..." << END_S;

    /**
     * Create a socket file listen_descriptor
     *
     * socket       returns the file descrip#include <netinet/in.h>tor for the new socket and -a in case of error
     * PF_INET      set the protocol to IP protocol family
     * SOCK_STREAM  specify that we want to use TCP. Use SOCK_DGRAM for UDP
     * 0            this parameter set the protocol. If 0 than the protocol is chosen automatically
     */
    if ((listen_descriptor = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        const int err = errno;
        LOG_E << "Failed to init the socket: " << strerror(err) << END_E;
        return 1;
    }

    // Set the SO_REUSEADDR option to avoid the error: "Address already in use"
    if (setsockopt(listen_descriptor, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
        const int err = errno;
        LOG_E << "Failed to setsockopt: " << strerror(err) << END_E;
        return 1;
    }

    struct sockaddr_in sockaddress = {0};          // Declare and zeros the general address structure

    sockaddress.sin_family = AF_INET;              // Always AF_INET

    sockaddress.sin_port = htons(port);            // Port

    sockaddress.sin_addr.s_addr = INADDR_ANY;      // Binds the socket to all available interfaces

    // Assigns the address specified by sockaddr to the socket referred to by listen_descriptor
    if (bind(listen_descriptor, (struct sockaddr *)&sockaddress, sizeof(sockaddress)) == -1) {
        const int err = errno;
        LOG_E << "Failed to bind: " << strerror(err) << END_E;
        return 1;
    }

    // Marks the socket as a passive socket that will be used to accept incoming connection. 100 is the backlog
    if (listen(listen_descriptor, 100) == -1) {
        const int err = errno;
        LOG_E << "Failed to listen: " << strerror(err) << END_E;
        return 1;
    }

    // Infinite loop where we wait for new connectiona
    while (true) {
        LOG_S << "Waiting for client..." << END_S;
        /**
         *  Accept the incoming connection
         *
         *  The addrlen argument is a value-result argument: the caller must
         *  initialize it to contain the size (in bytes) of the structure
         *  pointed to by addr; on return it will contain the actual size of
         *  the peer address.
         */
        socklen_t addrlen = sizeof(sockaddress);

        if ((work_descriptor = accept(listen_descriptor, (sockaddr *)&sockaddress, &addrlen)) == -1) {
            const int err = errno;
            LOG_E << "Failed to accept: " << strerror(err) << END_E;
            return 1;
        }
    }

    // Load error strings to get human readable results in case of error
    // SSL_load_error_strings();

    // // Clear the error queue before start
    // ERR_clear_error();

    LOG_S << "Closing server..." << END_S;

    return 0;
}