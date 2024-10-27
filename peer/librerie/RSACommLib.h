#ifndef RSACOMMLIB_H
#define RSACOMMLIB_H

int RSACommLib_Init(int key_size, const char *path_to_save_priv_key, const char *path_to_save_pub_key);

int RSACommLib_Encrypt( const char *public_key_file, const char *input_buffer, char* output_buffer);

int RSACommLib_Decrypt(const char *private_key_file, const char *input_buffer, char* output_buffer);
                                                 
#endif

/*
RSACommLib FUNCTIONS DESCRIPTION:

//int RSACommLib_Init(int key_size, const char *path_to_priv_key, const char *path_to_pub_key);
input: key size in bit, path to private key .pem file to create, path to public key .pem file to create
output: return 0 for success, raise errors and abort() for fatal errors.

int RSACommLib_Encrypt(const char *public_key_file, const char *input_buffer, char* output_buffer);
input: path to public key .pem file (previously generated), ptr to text buffer you have to encrypt, ptr to buffer where you want to write entcrypted text
output: return -1 if fail, otherwise it returns the size in byte of the encrypted text

int RSACommLib_Decrypt(const char *private_key_file, const char *input_buffer, char* output_buffer);
input: path to private key .pem file (previously generated), ptr to text buffer you have to decrypt, ptr to buffer where you want to write decrypted text
output: return -1 if fail, otherwise it returns the size in byte of the decrypted text

*/