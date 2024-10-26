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
#include <openssl/err.h>
#include <curl/curl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdbool.h>


#define BUFFER_SIZE 1024

char* get_cwd();

char* concatenateStrings(const char* str1, const char* str2);

char* getHostIp() ;

char* askQuestion(const char *question);

void error(const char *msg);


/*
typedef struct {
    char serverIp[16];
    int port;
    char filePath[256];
} ThreadArgs;

 */
int reciver(int choosenPort, char *outputFile);
void sender(const char *server_ip,int server_port,const char *file_path);
void *receiver(void *arg) ;

size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata);

int Search_in_File(char *fname, char *str);

int writeToFilePath(const char *filePath, const char * textToWrite);
