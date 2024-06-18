#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/err.h> // Include OpenSSL error handling

///// LA PRIRAM COSA DA FARE è MANDARE LE CHIAVI PUBBLICHE
#ifdef __APPLE__
char pubKeyFolderPath[]= "/Users/gabri/Desktop/secure_p2p_chat/def/.secrets/public_key.pem"; 
char privKeyFolderPath[]= "/Users/gabri/Desktop/secure_p2p_chat/def/.secrets/private_key.pem"; 

#elif __linux__
char pubKeyFolderPath[] = "~/secure_p2p_chat/src/secrets/peer_pub_key.pem";
char privKeyFolderPath[] = "~/secure_p2p_chat/src/secrets/private_key.pem";
#endif

#define SUCCESS 0
#define ERROR_OPENING_FILE 1
#define ERROR_READING_KEY 2
#define ERROR_ALLOCATING_MEMORY_CIPHERTEXT 3
#define ERROR_ALLOCATING_MEMORY_DECRYPTEDTEXT 4
#define ERROR_DECRYPTING_CIPHERTEXT 5

#define KEY_LENGTH  2048
#define PUB_EXP     65537


unsigned char* crittografa(const char* pathToPeerPubKey, const char *plaintext, int *ciphertext_len) {
    RSA *rsa = NULL;
    unsigned char *ciphertext = NULL;
    int ret;

    // Apertura del file della chiave pubblica
    FILE *pub_key_file = fopen(pathToPeerPubKey, "rb");
    if (pub_key_file == NULL) {
        printf("\n\n\n%s\n\n\n",pathToPeerPubKey);
        fprintf(stderr, "Errore nell'apertura del file della chiave pubblica.\n");
        exit(1);
    }

    // Lettura della chiave pubblica
    rsa = PEM_read_RSA_PUBKEY(pub_key_file, NULL, NULL, NULL);
    if (rsa == NULL) {
        fprintf(stderr, "Errore nella lettura della chiave pubblica.\n");
        exit(1);
    }

    // Chiusura del file della chiave pubblica
    fclose(pub_key_file);

    // Calcolo della lunghezza massima per il ciphertext
    int max_ciphertext_len = RSA_size(rsa);
    ciphertext = (unsigned char *)malloc(max_ciphertext_len);
    if (ciphertext == NULL) {
        fprintf(stderr, "Errore nell'allocazione della memoria per il ciphertext.\n");
        exit(1);
    }

    // Criptaggio del plaintext
    ret = RSA_public_encrypt(strlen(plaintext), (unsigned char*)plaintext, ciphertext, rsa, RSA_PKCS1_PADDING);
    if (ret == -1) {
        fprintf(stderr, "Errore nella crittografia del plaintext.\n");
        exit(1);
    }

    *ciphertext_len = ret;

    // Liberazione della memoria
    RSA_free(rsa);

    return ciphertext;
}

unsigned char* decrittografa(const char* pathToHostPrivKey,const char* testoCrittato, int ciphertext_len) {
    // Initialize variables
    RSA *rsa = NULL;
    unsigned char *ciphertext = NULL;
    unsigned char *decryptedtext = NULL;
    int ret, decryptedtext_len;

    // Open the private key file
    FILE *priv_key_file = fopen(pathToHostPrivKey, "rb");
    if (priv_key_file == NULL) {
        fprintf(stderr, "Error opening private key file.\n");
        exit(1);
    }

    // Read the private key
    rsa = PEM_read_RSAPrivateKey(priv_key_file, NULL, NULL, NULL);
    if (rsa == NULL) {
        fprintf(stderr, "Error reading private key.\n");
        exit(1);
    }

    // Close the private key file
    fclose(priv_key_file);

    // Allocate memory for the ciphertext
    ciphertext = (unsigned char *)malloc(ciphertext_len);
    if (ciphertext == NULL) {
        fprintf(stderr, "Error allocating memory for ciphertext.\n");
        exit(1);
    }

    // Convert the hexadecimal ciphertext to binary
    for (int i = 0; i < ciphertext_len; i++) {
        sscanf(testoCrittato + 2*i, "%02hhX", &ciphertext[i]);
    }

    // Allocate memory for the decrypted text
    decryptedtext = (unsigned char *)malloc(RSA_size(rsa));
    if (decryptedtext == NULL) {
        fprintf(stderr, "Error allocating memory for decrypted text.\n");
        exit(1);
    }

    // Decrypt the ciphertext
    ret = RSA_private_decrypt(ciphertext_len, ciphertext, decryptedtext, rsa, RSA_PKCS1_PADDING);
    if (ret == -1) {
        fprintf(stderr, "Error decrypting the ciphertext.\n");
        exit(1);
    }

    decryptedtext_len = ret;

    // Allocate memory for the staticResult and copy decrypted text
    unsigned char* staticResult = (unsigned char*)malloc(decryptedtext_len + 1);
    if (staticResult == NULL) {
        fprintf(stderr, "Error allocating memory for staticResult.\n");
        exit(1);
    }

    memcpy(staticResult, decryptedtext, decryptedtext_len);
    staticResult[decryptedtext_len] = '\0'; // Null-terminate the string

    // Print the decrypted text
    printf("Decrypted text: %s\n", staticResult);

    // Free memory
    RSA_free(rsa);
    free(ciphertext);
    free(decryptedtext);

    return staticResult;
}




