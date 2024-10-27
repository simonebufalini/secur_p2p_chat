#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "miaLibVarie.h"
#include  "RSACommLib.h"


int secureP2Pchat_simone(char* TargetIp, char* peer_s_pubKeyPath, char* pathToHostPrivKey);
