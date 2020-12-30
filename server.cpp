#include <errno.h>          // Uset for errno
#include <string.h>         // Used for strerror
#include <netinet/in.h>     // Used for sockaddr_in
#include <sys/socket.h>     // Used for ... You gess, socket
#include <arpa/inet.h>      // Used for inet_ntop
#include <unistd.h>         // Used for close
#include <openssl/ssl.h>
#include <openssl/err.h>
#include "defines.hpp"      // Contains the log macros and other defines


#define CERT_PWD "98a288db5efe910a39df76cce4ddde005dae208cdfcfdde3f4d98b861a72f4c8" // Update this with the content of the password file
#define CERTIFICATE "certificate.pem"
#define KEY "certificate.key"


int main(int argc, char **argv) {

    int listen_descriptor;
    int client_descriptor;
    int optval = 1;
    const unsigned short port = PORT;
    std::string cert_pwd = CERT_PWD;
    std::string certificate = CERTIFICATE;
    std::string key = KEY;
    SSL *ssl;

    LOG_S << "Starting server..." << END_S;

    SSL_load_error_strings();   // Load error strings to get human readable results in case of error
    ERR_load_crypto_strings();
    ERR_clear_error();          // Clear the error queue before start

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

    ///////////////////////////////////////////// Create ssl context and load certificates
    // Create new SSL context
    SSL_CTX *ctx = SSL_CTX_new(TLS_server_method());

    // Load the certificate
    // Set the certificate password into the openssl userdata
    SSL_CTX_set_default_passwd_cb_userdata(ctx, (void *)&cert_pwd);
    // Set the certificate password callback. Maybe the most ugly code that you will see
    SSL_CTX_set_default_passwd_cb(ctx, [](char *buf, int size, int rwflag, void *userdata) -> int {
        std::string *pwd = (std::string *) userdata;

        if (pwd == 0 || pwd->empty() || size < (pwd->length() + 1)) {
            return 0;
        }

        strncpy(buf, pwd->c_str(), pwd->length());
        buf[pwd->length()] = 0;

        return pwd->length();
    });

    // Load the certificate file
    if (SSL_CTX_use_certificate_file(ctx, certificate.c_str(), SSL_FILETYPE_PEM) != 1) {
        LOG_E << "Failed to load the certificate " << certificate << END_E;
        LOG_SSL_STACK();
        SSL_CTX_free(ctx);
        return 1;
    }

    // Load the private key
    if (SSL_CTX_use_PrivateKey_file(ctx, key.c_str(), SSL_FILETYPE_PEM) <= 0) {
        LOG_E << "Failed to load the private key " << key << END_E;
        LOG_SSL_STACK();

        /**
         * NOTE(max):   The free is useless if we shut down the client like in this case.
         *              But if we plane to reuse the client application we need to clean up before leave
         */
        SSL_CTX_free(ctx);
        return 1;
    }

    // Verify the private key
    if (SSL_CTX_check_private_key(ctx) != 1) {
        LOG_E << "Failed to verify the private key" << END_E;
        LOG_SSL_STACK();
        SSL_CTX_free(ctx);
        return 1;
    }

    // Infinite loop where we wait for new connectiona
    while (true) {
        LOG_I << "Waiting for client..." << END_I;
        /**
         *  Accept the incoming connection
         *
         *  The addrlen argument is a value-result argument: the caller must
         *  initialize it to contain the size (in bytes) of the structure
         *  pointed to by addr; on return it will contain the actual size of
         *  the peer address.
         */
        socklen_t addrlen = sizeof(sockaddress);

        if ((client_descriptor = accept(listen_descriptor, (sockaddr *)&sockaddress, &addrlen)) == -1) {
            const int err = errno;
            LOG_E << "Failed to accept: " << strerror(err) << END_E;
            return 1;
        }

        // Here we can close the listen socket if we fork or want to serve only one client
        // if (close(listen_descriptor) == -1) {
        //     const int err = errno;
        //     LOG_W << "Failed to close the listen socket: " << strerror(err) << END_W;
        //     // Here we don't return but just procede to serve the connected client
        // }

        // Get the client ip. Extend the buffer to hold ipv6
        char client_ip_buff[INET6_ADDRSTRLEN];
        inet_ntop(AF_INET, &(sockaddress.sin_addr), client_ip_buff, INET6_ADDRSTRLEN); // Ignore potential errors
        LOG_I << "Client connected " << client_ip_buff << ":" << port << END_I;

        ///////////////////////////////////////////////////////////////// TLS CONNECTION

        // Create a new TLS structure
        if ((ssl = SSL_new(ctx)) == NULL) {
            LOG_E << "Failed to create a new SSL" << END_E;
            SSL_CTX_free(ctx);
            return 1;
        }

        // Connect the socket file descriptor with the ssl object
        if (SSL_set_fd(ssl, client_descriptor) == 0) {
            LOG_E << "Failed to bind the ssl object and the socket file descriptor" << END_E;
            SSL_CTX_free(ctx);
            SSL_free(ssl);
            return 1;
        }

        // Accept the incoming TLS connection
        const int accept_e = SSL_accept(ssl);

        if (accept_e <= 0) {
            LOG_E << "Failed to establish TLS connection" << END_E;
            LOG_SSL_STACK();

            SSL_CTX_free(ctx);
            SSL_free(ssl);
            return 1;
        }

        LOG_S << "TLS connection established with the client" << END_S;

        // Read the client data
        char msg[sizeof(MSG_1)];
        const int received_data = SSL_read(ssl, msg, sizeof(MSG_1));

        if (received_data <= 0) {
            LOG_E << "Failed to receive message from the client" << END_E;
            LOG_SSL_STACK();

            SSL_CTX_free(ctx);
            SSL_free(ssl);
            return 1;
        }

        LOG_I << "Received: " << msg << END_I;

        // Send data to client
        const int sent_data = SSL_write(ssl, MSG_2, sizeof(MSG_2));

        if (sent_data <= 0) {
            LOG_E << "Failed to send message to the client" << END_E;
            LOG_SSL_STACK();

            SSL_CTX_free(ctx);
            SSL_free(ssl);
            return 1;
        }

        LOG_I << "Send: " << MSG_2 << END_I;

        LOG_S << "Disconnecting from the client..." << END_S;
        SSL_shutdown(ssl);
        SSL_free(ssl);
        close(client_descriptor);
    }

    LOG_S << "Closing server..." << END_S;
    SSL_CTX_free(ctx);
    close(listen_descriptor);

    return 0;
}