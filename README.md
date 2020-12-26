# TLS server/client sample with openssl

This is simple project that shows how to setup a TLS communication between server and client using openssl

## Generate test Root CA

* Generate the private key of the root CA: `openssl genrsa -out rootCAKey.pem 2048`
* Generate random password for the certificate: `openssl rand -hex 32 > password`
* Generate the certificate key encrypted with the password in password: `openssl genrsa -aes256 -passout pass:$(cat password) -out certificate.key 2048`
* Generate the public certificate: `openssl req -new -days 3650 -x509 -key certificate.key -passin pass:$(cat password) -out certificate.pem -subj "/C=US/ST=Milan/L=Milan/O=MazerFaker/OU=Engineering/CN=TheMazerFaker.io"`

## Guides

* [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)