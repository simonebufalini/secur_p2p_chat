#ifndef CRYPTO_H
#define CRYPTO_H

//  return 0 for success, -1 for fail.
int encrypt(const char *input_buffer, char* output_buffer);

//  return 0 for success, -1 for fail.
int decrypt(const char *input_buffer, char* output_buffer);

#endif
