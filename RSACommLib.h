#ifndef RSACOMMLIB_H
#define RSACOMMLIB_H

int RSACommLib_Init(int key_size, const char *path_to_save_priv_key, const char *path_to_save_pub_key);

int RSACommLib_Encrypt( const char *public_key_file, const char *input_buffer, char* output_buffer);

int RSACommLib_Decrypt(const char *private_key_file, const char *input_buffer, char* output_buffer);
                                                 
#endif