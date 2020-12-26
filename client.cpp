#include <errno.h>          // Uset for errno
#include <string.h>         // Used for strerror
#include <netinet/in.h>     // Used for sockaddr_in
#include <sys/socket.h>     // Used for socket
#include <netdb.h>          // Used for gethostbyname
#include <arpa/inet.h>      // Used for inet_ntoa
#include <openssl/ssl.h>
#include <openssl/err.h>
#include "defines.hpp"      // Contains the log macros and other defines


#define ADDRESS "localhost"
#define CA_PATH NULL        // TODO(max): Set this
#define CA_FILE "certificate.pem"


int main(int argc, char **argv) {

    int descriptor;
    struct sockaddr_in sockaddress = {0};
    const std::string address = ADDRESS;
    const unsigned short port = PORT;
    const char *ca_path = CA_PATH;
    const char *ca_file = CA_FILE;
    struct hostent *host;
    char *bin_ip;
    SSL *ssl;

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

    ///////////////////////////////////////////////////////////////// TLS
    LOG_I << "Setup TLS connection " << END_I;

    SSL_load_error_strings();   // Load error strings to get human readable results in case of error
    ERR_clear_error();          // Clear the error queue before start

    // Create new context
    SSL_CTX *ctx = SSL_CTX_new(TLS_client_method());
    if (ctx == NULL) {
        LOG_E << "Failed to get openssl context" << END_E;
        return 1;
    }

    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);
    SSL_CTX_set_verify_depth(ctx, 5);

    // Load the certificate from file and dir
    if (SSL_CTX_load_verify_locations(ctx, ca_file, ca_path) == 0) {
        LOG_E << "Failed to load the certificate/s" << END_E;
        // NOTE(max): In case we return but not close the program we want to free the context
        SSL_CTX_free(ctx);
        return 1;
    }

    // Create a new TLS structure
    if ((ssl = SSL_new(ctx)) == NULL) {
        LOG_E << "Failed to create a new SSL" << END_E;
        SSL_CTX_free(ctx);
        return 1;
    }

    // Set the hostname to send with the "Client Hello"
    if (SSL_set_tlsext_host_name(ssl, address.c_str()) == 0) {
        LOG_E << "Failed to set the host name to send with the Client Hello" << END_E;
        SSL_CTX_free(ctx);
        SSL_free(ssl);
        return 1;
    }

    LOG_S << "Closing client..." << END_S;
    return 0;
}