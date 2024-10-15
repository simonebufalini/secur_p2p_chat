#include <gmp.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#define BIT_SIZE 1024
#define TEST_ITERATIONS 50
#define FILENAME "key"

typedef struct {

    mpz_t prime;
    gmp_randstate_t *state;
    size_t bit_size;
    int result;
} prime_thread_data_t;

int generate_prime(mpz_t prime, gmp_randstate_t state, size_t bit_size) {

    do {
        mpz_urandomb(prime, state, bit_size);
        mpz_setbit(prime, bit_size - 1);
    } while (mpz_probab_prime_p(prime, TEST_ITERATIONS) == 0);

    if (gmp_errno != 0){
        fprintf(stderr, "GMP error %d.\n", gmp_errno);
        return -1;
    }
    return 0;
}

void* thread_generate_prime(void* arg) {

    prime_thread_data_t* data = (prime_thread_data_t*) arg;
    data->result = generate_prime(data->prime, *(data->state), data->bit_size);
    pthread_exit(NULL);
}

int generate_rsa_keys(mpz_t *n, mpz_t *e, mpz_t *d, size_t bit_size) {
    
    mpz_t p, q, phi_n, diff, min_distance;
    gmp_randstate_t state;
    size_t min_distance_bits = bit_size / 4;
    int status = 0;

    mpz_inits(p, q, phi_n, diff, min_distance, NULL);
    gmp_randinit_mt(state);
    gmp_randseed_ui(state, time(NULL));

    mpz_set_ui(*e, 65537);
    mpz_ui_pow_ui(min_distance, 2, min_distance_bits);

    pthread_t thread1, thread2;
    prime_thread_data_t data1, data2;

    mpz_init(data1.prime);
    mpz_init(data2.prime);

    data1.state = &state;
    data2.state = &state;
    data1.bit_size = bit_size;
    data2.bit_size = bit_size;

    pthread_create(&thread1, NULL, thread_generate_prime, (void*)&data1);
    pthread_create(&thread2, NULL, thread_generate_prime, (void*)&data2);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    if (data1.result != 0 || data2.result != 0) {
        fprintf(stderr, "Error during RSA key generation (prime generation).\n");
        status = -1;
        goto cleanup;
    }

    mpz_set(p, data1.prime);
    mpz_set(q, data2.prime);

    mpz_sub(diff, p, q);
    mpz_abs(diff, diff);

    if (mpz_cmp(p, q) == 0 || mpz_cmp(diff, min_distance) < 0) {
        fprintf(stderr, "p and q are either equal or too close.\n");
        status = -1;
        goto cleanup;
    }

    mpz_sub_ui(p, p, 1);
    mpz_sub_ui(q, q, 1);
    mpz_mul(phi_n, p, q);

    while (mpz_gcd_ui(NULL, phi_n, 65537) != 1) {
        fprintf(stderr, "GCD of phi_n and e is not 1.\n");
        status = -1;
        goto cleanup;
    }

    if (mpz_invert(*d, *e, phi_n) == 0) {
        fprintf(stderr, "Error during RSA key generation (inversion failed).\n");
        status = -1;
        goto cleanup;
    }

    mpz_add_ui(p, p, 1);
    mpz_add_ui(q, q, 1);
    mpz_mul(*n, p, q);

cleanup:
    mpz_clears(p, q, phi_n, diff, min_distance, data1.prime, data2.prime, NULL);
    gmp_randclear(state);
    return status;
}

int safe_write(int file_descriptor, const char *buffer) {

    if (file_descriptor < 0) {
        fprintf(stderr, "Invalid file descriptor.\n");
        return -1;
    }

    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;

    if (fcntl(file_descriptor, F_SETLKW, &lock) == -1) {
        if (errno == EACCES || errno == EAGAIN) {
            fprintf(stdout, "File is not available.\n");
        } else {
            fprintf(stderr, "Unable to lock the file: %s\n", strerror(errno));
        }
        return -1;
    }

    ssize_t bytes = write(file_descriptor, buffer, strlen(buffer));
    if (bytes < 0) {
        fprintf(stderr, "Error writing on file: %s\n", strerror(errno));

        lock.l_type = F_UNLCK;
        fcntl(file_descriptor, F_SETLK, &lock);
        return -1;
    }

    lock.l_type = F_UNLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;

    if (fcntl(file_descriptor, F_SETLK, &lock) == -1) {
        fprintf(stderr, "Unexpected error occurred while unlocking: %s\n", strerror(errno));
        return -1;
    }

    return 0;
}

int main(int argc, char *argv[]){

    mpz_t n, e, d;
    mpz_inits(n, e, d, NULL);

    mpz_t *ptr_n = &n, *ptr_e = &e, *ptr_d = &d;
    int status = generate_rsa_keys(ptr_n, ptr_e, ptr_d, BIT_SIZE);
    if (status != 0){
        fprintf(stderr, "generate_rsa_keys error.");
        exit(EXIT_FAILURE);
    }

    int file_d = open(FILENAME, O_RDWR | O_CREAT | O_TRUNC, 0644);

    if (file_d == -1){
        fprintf(stderr, "Unexpected error occurred.\n");
        exit(EXIT_FAILURE);
    }

    size_t len = mpz_sizeinbase(n, 10) + mpz_sizeinbase(e, 10) + mpz_sizeinbase(d, 10);
    char *buffer = (char*)malloc(sizeof(char) * (len)+3);

    if (buffer == NULL){
        fprintf(stderr, "Malloc error.\n");
        close(file_d);
        exit(EXIT_FAILURE);
    }

    gmp_sprintf(buffer, "%Zd;%Zd;%Zd;", n, e, d);
    safe_write(file_d, buffer);
    mpz_clears(n, e, d, NULL);
    close(file_d);
    free(buffer);
    
    exit(EXIT_SUCCESS);
}
