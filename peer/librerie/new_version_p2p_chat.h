


void sending(char * TargetIp, char *pathToPeerPubKey);
void receiving(int server_fd);
void *receive_thread(void *server_fd);

int secureP2Pchat(char *private_ip_of_peer, char* pubKeyPath, char* hostPrivateKey);