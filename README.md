# TLS server/client sample with openssl

This is simple project that shows how to setup a TLS communication between server and client using openssl

## Generate test Root CA

* Generate the private key of the root CA: `openssl genrsa -out rootCAKey.pem 2048`
* Generate random password for the certificate: `openssl rand -hex 32 > password`

Note that this snippet will generate a certificate for `localhost`. If you want to change domain replace `localhost` with the desired domain.

* Generate the certificate key encrypted with the password in password: `openssl genrsa -aes256 -passout pass:$(cat password) -out certificate.key 2048`
* Generate the public certificate for localhost: `openssl req -new -days 3650 -x509 -key certificate.key -passin pass:$(cat password) -out certificate.pem -subj "/C=IT/ST=Italy/L=Milan/O=MazerFaker/OU=Engineering/CN=localhost"`

## Guides

* [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)