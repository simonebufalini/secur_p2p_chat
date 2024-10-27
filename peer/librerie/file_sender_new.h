#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>



void send_file(FILE *file, int socket);

int sender_nuovo(char fileDaInviare[]);
