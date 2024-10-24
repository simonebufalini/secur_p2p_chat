#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#include <openssl/rand.h>

#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "librerie/RSACommLib.h"

// gcc test_cripto.c librerie/RSACommLib.c -o rsa_test -lssl -lcrypto -pthread -w //-Wall



int main() {
    const char *private_key_file = "private_key.pem";
    const char *public_key_file = "public_key.pem";
    const int key_size = 2048;

    // Step 1: Generate RSA keys
    if (RSACommLib_Init(key_size, private_key_file, public_key_file) != 0) {
        fprintf(stderr, "Failed to initialize RSA keys\n");
        return 1;
    }

    const char *message = "Hello, RSA!";
    char encrypted[256];
    char decrypted[256];

    // Step 2: Encrypt the message
    int encrypted_length = RSACommLib_Encrypt(public_key_file, message, encrypted);
    if (encrypted_length == -1) {
        fprintf(stderr, "Encryption failed\n");
        return 1;
    }

    // Step 3: Decrypt the message
    int decrypted_length = RSACommLib_Decrypt(private_key_file, encrypted, decrypted);
    if (decrypted_length == -1) {
        fprintf(stderr, "Decryption failed\n");
        return 1;
    }

    printf("Original: %s\n", message);
    printf("Encrypted: ");
    for (int i = 0; i < encrypted_length; i++) {
        printf("%02x", (unsigned char)encrypted[i]);
    }
    printf("\n");
    printf("Decrypted: %s\n", decrypted);

    return 0;
}
