#include <errno.h>          // Uset for errno
#include <string.h>         // Used for strerror
#include <netinet/in.h>     // Used for sockaddr_in
#include <sys/socket.h>     // Used for socket
#include <netdb.h>          // Used for gethostbyname
#include <arpa/inet.h>      // Used for inet_ntoa
#include "defines.hpp"      // Contains the log macros and other defines


#define ADDRESS "localhost"


int main(int argc, char **argv) {

    int descriptor;
    struct sockaddr_in sockaddress = {0};
    const std::string address = ADDRESS;
    const unsigned short port = PORT;
    struct hostent *host;
    char *bin_ip;

    LOG_S << "Starting client..." << END_S;

    /**
     * Create a socket file listen_descriptor
     *
     * socket       returns the file descrip#include <netinet/in.h>tor for the new socket and -a in case of error
     * PF_INET      set the protocol to IP protocol family
     * SOCK_STREAM  specify that we want to use TCP. Use SOCK_DGRAM for UDP
     * 0            this parameter set the protocol. If 0 than the protocol is chosen automatically
     */
    if ((descriptor = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        const int err = errno;
        LOG_E << "Failed to init the socket: " << strerror(err) << END_E;
        return 1;
    }

    sockaddress.sin_family = AF_INET;       // Always AF_INET
    sockaddress.sin_port = htons(port);     // Port

    /**
     * Resolve the address passed like a string
     * Note that `host` is a static buffer and in case of concurency this part needs to be locked
     *
     * Acquire mutex here
     */
    if ((host = gethostbyname(address.c_str())) == NULL) {

        // Handle the error
        const int err = h_errno;

        switch (err) {

        // The specified host is unknown.
        case HOST_NOT_FOUND:
            LOG_E << "Host not found" << END_E;
            break;

        /**
         * The requested name is valid but does not have an IP
         * address.  Another type of request to the name server for
         * this domain may return an answer.  The constant NO_ADDRESS
         * is a synonym for NO_DATA.
         */
        case NO_DATA:

        // A nonrecoverable name server error occurred.
        case NO_RECOVERY:

        // A temporary error occurred on an authoritative name server. Try again later.
        case TRY_AGAIN:

        default:
            LOG_E << "Failed to get the host name. try again later" << END_E;
            break;
        }

        return 1;
    }

    /**
     * Converts the Internet host address cp from the IPv4 numbers-and-dots notation into binary form.
     * Note also that the `bin_ip` is a static buffer and in case of concurency needs to be locked
     */
    bin_ip = inet_ntoa(*((struct in_addr *)host->h_addr_list[0]));

    // Copy the address in order to unlock the static buffer
    const int inet_pton_res = inet_pton(AF_INET, bin_ip, &sockaddress.sin_addr);
    // Unlock the mutex here

    // Check for error
    if (inet_pton_res == 0) {
        // `bin_ip` does not contain a character string representing a valid network address in the specified address family
        LOG_E << "Invalid ip address" << END_E;
        return 1;
    } else if (inet_pton_res == -1) {
        // Does not contain a valid address family
        const int err = errno;
        LOG_E << "Not a valid address family: " << strerror(err) << END_E;
        return 1;
    }

    // Connect to the server
    if (connect(descriptor, (struct sockaddr *)&sockaddress, sizeof(sockaddress)) == -1) {
        const int err = errno;
        LOG_E << "Failed to connect to the server: " << strerror(err) << END_E;
        return 1;
    }

    LOG_I << "Connected to the server " << address << ":" << port << END_I;

    LOG_S << "Closing client..." << END_S;
    return 0;
}