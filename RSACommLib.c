#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#include <openssl/rand.h>

#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define RSA_PADDING_OVERHEAD 42
#define PUBLIC_EXP 65537
#define MAX_BYTES 64

void printError(){

    ERR_print_errors_fp(stderr);
}

void handleErrors() {

    ERR_print_errors_fp(stderr);
    abort();
}

void print_public_key(RSA *rsa) {

    BIO *bio = BIO_new_fp(stdout, BIO_NOCLOSE);
    if (bio == NULL) {
        fprintf(stderr, "Failed to create BIO\n");
        return;
    }

    if (PEM_write_bio_RSA_PUBKEY(bio, rsa) != 1) {
        fprintf(stderr, "Failed to write public key\n");
    }

    BIO_free(bio);
}

int init_prng(){

    if (RAND_load_file("/dev/urandom", MAX_BYTES) <= 0) {
        return 1;
    }

    const unsigned char custom_data[] = "additional entropy";
    RAND_add(custom_data, sizeof(custom_data), 0.0);
    return 0;
}

void write_private_key(RSA *rsa, const char *path_to_priv_key) {

    BIO *bio = BIO_new_file(path_to_priv_key, "w");
    if (bio == NULL) {
        handleErrors();
    }

    if (PEM_write_bio_RSAPrivateKey(bio, rsa, NULL, NULL, 0, NULL, NULL) != 1) {
        BIO_free_all(bio);
        handleErrors();
    }

    BIO_free_all(bio);
}

void write_public_key(RSA *rsa, const char *path_to_pub_key) {

    BIO *bio = BIO_new_file(path_to_pub_key, "w");
    if (bio == NULL) {
        handleErrors();
    }

    if (PEM_write_bio_RSA_PUBKEY(bio, rsa) != 1) {
        BIO_free_all(bio);
        handleErrors();
    }

    BIO_free_all(bio);
}

int RSACommLib_Init(int key_size, const char *path_to_priv_key, const char *path_to_pub_key){

    OpenSSL_add_all_algorithms();
    ERR_load_crypto_strings();
    
    RSA *rsa = RSA_new();
    if (rsa == NULL){
        RSA_free(rsa);
        handleErrors();
    }

    BIGNUM *bn = BN_new();
    if (bn == NULL) {
        RSA_free(rsa);
        BN_free(bn);
        handleErrors();
    }

    BN_set_word(bn, PUBLIC_EXP);

    if (init_prng() != 0) {
        fprintf(stderr, "Custom PRNG initialization failed. Using default\n");
    } else {
        fprintf(stdout, "Custom PRNG initialized\n");
    }

    if (RSA_generate_key_ex(rsa, key_size, bn, NULL) != 1){
        fprintf(stderr, "Error generating RSA key\n");
        RSA_free(rsa);
        BN_free(bn);
        handleErrors();
    }

    fprintf(stdout, "RSA key successfully generated\n");
    print_public_key(rsa);
    
    write_private_key(rsa, path_to_priv_key);
    write_public_key(rsa, path_to_pub_key);

    mode_t mode = 0644;
    chmod(path_to_priv_key, mode);
    chmod(path_to_pub_key, mode);

    BN_free(bn);
    RSA_free(rsa);
    return 0;
}

int RSACommLib_Encrypt(const char *public_key_file, const char *input_buffer, char *output_buffer) {

    FILE *pubkey_file = fopen(public_key_file, "r");
    if (pubkey_file == NULL) {
        fprintf(stderr, "Could not open public key file %s\n", public_key_file);
        return -1;
    }

    RSA *rsa = PEM_read_RSA_PUBKEY(pubkey_file, NULL, NULL, NULL);
    fclose(pubkey_file);
    if (rsa == NULL) {
        fprintf(stderr, "Unable to read public key\n");
        printError();
        return -1;
    }

    int input_length = strlen(input_buffer);
    int rsa_size = RSA_size(rsa);
    
    if (input_length > rsa_size - RSA_PADDING_OVERHEAD) {
        fprintf(stderr, "Input buffer is too large for the RSA key size\n");
        printError();
        RSA_free(rsa);
        return -1;
    }

    int encrypted_length = RSA_public_encrypt(input_length, (unsigned char *)input_buffer, 
                                              (unsigned char *)output_buffer, 
                                              rsa, RSA_PKCS1_OAEP_PADDING);
    
    RSA_free(rsa);
    if (encrypted_length == -1) {
        fprintf(stderr, "RSA_public_encrypt failed to encrypt data\n");
        return -1;
    }

    return encrypted_length;
}

int RSACommLib_Decrypt(const char *private_key_file, const char *input_buffer, char *output_buffer) {

    FILE *privkey_file = fopen(private_key_file, "r");
    if (privkey_file == NULL) {
        fprintf(stderr, "Could not open private key file %s\n", private_key_file);
        return -1;
    }

    RSA *rsa = PEM_read_RSAPrivateKey(privkey_file, NULL, NULL, NULL);
    fclose(privkey_file);
    if (rsa == NULL) {
        fprintf(stderr, "Unable to read private key\n");
        return -1;
    }

    int rsa_size = RSA_size(rsa);
    if (rsa_size <= 0) {
        fprintf(stderr, "Invalid RSA key size\n");
        RSA_free(rsa);
        return -1;
    }

    int decrypted_length = RSA_private_decrypt(rsa_size, (unsigned char *)input_buffer,
                                                (unsigned char *)output_buffer,
                                                rsa, RSA_PKCS1_OAEP_PADDING);
    
    RSA_free(rsa);
    if (decrypted_length == -1) {
        fprintf(stderr, "RSA_private_decrypt failed to decrypt data\n");
        return -1;
    }

    output_buffer[decrypted_length] = '\0';
    return decrypted_length;
}
