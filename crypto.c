#include <gmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include "crypto.h"

#define BUFFER_SIZE 2048
#define FILENAME "key"

int read_file(int file_descriptor, char *buffer) {

    if (file_descriptor < 0 || buffer == NULL) {
        fprintf(stderr, "Invalid input parameters.\n");
        return -1;
    }

    struct flock lock;
    lock.l_type = F_RDLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;

    if (fcntl(file_descriptor, F_SETLKW, &lock) == -1) {
        fprintf(stderr, "Error locking the file: %s\n", strerror(errno));
        return -1;
    }

    off_t file_size = lseek(file_descriptor, 0, SEEK_END);
    if (file_size == -1) {
        fprintf(stderr, "Error seeking to end of file: %s\n", strerror(errno));
        lock.l_type = F_UNLCK;
        fcntl(file_descriptor, F_SETLK, &lock);
        return -1;
    }

    if (lseek(file_descriptor, 0, SEEK_SET) == -1) {
        fprintf(stderr, "Error seeking to start of file: %s\n", strerror(errno));
        lock.l_type = F_UNLCK;
        fcntl(file_descriptor, F_SETLK, &lock);
        return -1;
    }

    ssize_t bytes_read = read(file_descriptor, buffer, file_size);
    if (bytes_read < 0) {
        fprintf(stderr, "Error reading file: %s\n", strerror(errno));
        lock.l_type = F_UNLCK;
        fcntl(file_descriptor, F_SETLK, &lock);
        return -1;
    }

    buffer[bytes_read] = '\0';

    lock.l_type = F_UNLCK;
    if (fcntl(file_descriptor, F_SETLK, &lock) == -1) {
        fprintf(stderr, "Unexpected error occurred while unlocking: %s\n", strerror(errno));
        return -1;
    }

    return bytes_read;
}

int extract_key(const char *key_filename, mpz_t n, mpz_t e, mpz_t d) {
    int file = open(key_filename, O_RDONLY);
    if (file == -1) {
        perror("Error opening file");
        return -1;
    }

    char key_buffer[BUFFER_SIZE];
    if (read_file(file, key_buffer) == -1) {
        close(file);
        return -1;
    }
    close(file);

    char *module = NULL;
    char *ptr = key_buffer;
    size_t len = 0;
    int i = 0;

    while (*ptr != '\0') {
        len = 0;

        while (ptr[len] != ';' && ptr[len] != '\0') {
            len++;
        }

        module = (char*) realloc(module, len + 1);
        if (module == NULL) {
            fprintf(stderr, "Memory allocation error\n");
            return -1;
        }

        memcpy(module, ptr, len);
        module[len] = '\0';

        switch (i) {
            case 0:
                mpz_set_str(n, module, 10);
                break;
            case 1:
                mpz_set_str(e, module, 10);
                break;
            case 2:
                mpz_set_str(d, module, 10);
                break;
            default:
                break;
        }

        ptr += len + 1;
        i++;

        if (*ptr == '\0') {
            break;
        }
    }

    free(module);
    return 0;
}

int encrypt(const char *input_buffer, char *output_buffer) {
    const char *key_file = FILENAME;

    mpz_t n, e, d, m, crypted, base, char_code;
    mpz_inits(n, e, d, m, crypted, base, char_code, NULL);

    int code = extract_key(key_file, n, e, d);
    if (code != 0) {
        fprintf(stderr, "Error extracting key\n");
        return -1;
    }

    mpz_set_ui(m, 0);
    mpz_set_ui(base, 256);

    while (*input_buffer != '\0') {
        mpz_set_ui(char_code, (unsigned char)(*input_buffer));
        mpz_mul(m, m, base);
        mpz_add(m, m, char_code);
        input_buffer++;
    }

    mpz_powm(crypted, m, e, n);

    char *crypted_string = mpz_get_str(NULL, 10, crypted);
    strcpy(output_buffer, crypted_string);

    mpz_clears(n, e, d, m, crypted, base, char_code, NULL);
    free(crypted_string);

    return 0;
}

int decrypt(const char *input_buffer, char *output_buffer) {
    const char *key_file = FILENAME;

    mpz_t n, e, d, m, crypted, base, char_code;
    mpz_inits(n, e, d, m, crypted, base, char_code, NULL);

    int code = extract_key(key_file, n, e, d);
    if (code != 0) {
        fprintf(stderr, "Error extracting key\n");
        return -1;
    }

    mpz_set_str(crypted, input_buffer, 10);

    mpz_powm(m, crypted, d, n);

    mpz_set_ui(base, 256);

    size_t output_idx = 0;
    while (mpz_cmp_ui(m, 0) > 0) {
        mpz_mod(char_code, m, base);
        output_buffer[output_idx++] = (char)mpz_get_ui(char_code);
        mpz_div(m, m, base);
    }

    output_buffer[output_idx] = '\0';

    for (size_t i = 0; i < output_idx / 2; i++) {
        char temp = output_buffer[i];
        output_buffer[i] = output_buffer[output_idx - 1 - i];
        output_buffer[output_idx - 1 - i] = temp;
    }

    mpz_clears(n, e, d, m, crypted, base, char_code, NULL);
    return 0;
}
