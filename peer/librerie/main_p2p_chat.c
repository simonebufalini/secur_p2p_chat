#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "crittografia.h"
#include "miaLibVarie.h"


//#define LISTENING_PORT_RECIVING_FILES 51811
//#define PORT_FOR_SENDING_FILES 51812

//RICORDATI DI FARE UFW ALLOW PORTE

int PORT = 51810;

void sending(char * TargetIp, char *pathToPeerPubKey);
void receiving(int server_fd);
void *receive_thread(void *server_fd);


char* pathToHostPrivateKey = NULL;

int secureP2Pchat(char* TargetIp, char* pubKeyPath, char* pathToHostPrivKey)
{
    	char *workingDirectory = get_cwd();
        const char *fileChiave = concatenateStrings(workingDirectory, ".secrets/public_key.pem");


    if (pathToHostPrivateKey == NULL) {
        pathToHostPrivateKey = pathToHostPrivKey;
    }
    //NOTA: lui usa la chiave pubblica del peer per crirptatre. solo così il peer riceve e decritta con la sua chiave privata.ERGO VANNO SCAMBIATE
    char name[20];

    char* publicIP = getHostIp();
    printf("Public IPv4 address of this machine: %s\n\n", publicIP);
    free(publicIP); // Free memory allocated for publicIP

    printf("Enter your name (max 20 char):");
    fgets(name, sizeof(name), stdin);

    printf("\nEnter your port number:(FOR DEMO POURPOSE ITS FIXED AT %d)", PORT);
   // scanf("%d", &PORT);

    printf("\nEnter other peer's ip:");
    fgets(TargetIp, sizeof(TargetIp), stdin);


//Devi implementare thread separato per reciver e togliere l'inserimento manuale della pubblica

   // Forking for sender

    /*
    pid_t pid_sender = fork();

    if (pid_sender == -1) {
        // Fork failed
        perror("fork sender");
        return 1;
    } else if (pid_sender == 0) {
        // Child process for sender
        sender(TargetIp, PORT_FOR_SENDING_FILES, fileChiave);
        exit(0);
    } else {
        // Parent process
        printf("Sender process forked with PID: %d\n", pid_sender);
    }

    // Forking for receiver
    pid_t pid_receiver = fork();

    if (pid_receiver == -1) {
        // Fork failed
        perror("fork receiver");
        return 1;
    } else if (pid_receiver == 0) {
        // Child process for receiver
        while(1){
        //int i = reciver(LISTENING_PORT_RECIVING_FILES, "peerPublicKeyFile_recived.p");
        reciver(LISTENING_PORT_RECIVING_FILES, "peerPublicKeyFile_recived.p");
        }
        exit(0);
    } else {
        // Parent process
        printf("Receiver process forked with PID: %d\n", pid_receiver);
    }
*/

/*sudo ufw allow this porta
#ifdef __linux__
char comandoAllowPorta[100];
snprintf(comandoAllowPorta, sizeof(comandoAllowPorta), "sudo ufw allow %s", PORT);
system(comandoAllowPorta);
#endif
*/

    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int k = 0;

    // Creazione del socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    // Attach del socket alla porta

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Stampa dell'indirizzo IP e della porta del server
    printf("IP address is: %s\n", inet_ntoa(address.sin_addr));
    printf("Port is: %d\n", (int)ntohs(address.sin_port));

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 5) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    int ch;
    pthread_t tid;
    pthread_create(&tid, NULL, &receive_thread, &server_fd); // Creazione del thread per continuare a ricevere i messaggi in tempo reale
    printf("\n*****At any point in time press the following:*****\n1. Send message\n0. Quit\n");
    printf("\nEnter choice:");
    do
    {
        scanf("%d", &ch);
        switch (ch)
        {
        case 1:
            sending(TargetIp,pubKeyPath);
            break;
        case 0:
            printf("\nLeaving\n");
            break;
        default:
            printf("\nWrong choice\n");
        }
    } while (ch);

    close(server_fd);

    return 0;
}


//Sending messages to port
void sending(char* TargetIp, char *pathToPeerPubKey) {
    char buffer[2000] = {0};
    int PORT_server = PORT;

   /* printf("Enter the port to send message: ");
    scanf("%d", &PORT_server);*/

    int sock = 0;
    struct sockaddr_in serv_addr;
    char hello[1024] = {0};

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(TargetIp);;
    serv_addr.sin_port = htons(PORT_server);

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return;
    }
/// QUI CI DEVE ESSERE SCAMBIO CHIAVI PUBBLICHE
    char dummy;
    printf("Enter your message: ");
    scanf("%c", &dummy); // Clear the newline character from the buffer
    scanf("%[^\n]s", hello);

    int ciphertext_len;
    unsigned char *ciphertext = crittografa(pathToPeerPubKey, hello, &ciphertext_len);

    if (ciphertext == NULL) {
        fprintf(stderr, "Error in encrypting plaintext.\n");
        close(sock);
        return;
    }

    // Print the hexadecimal representation of the ciphertext
    for (int i = 0; i < ciphertext_len; i++) {
        printf("%02X", ciphertext[i]);
    }

    int hex_string_len = ciphertext_len * 2 + 1;
    char *hex_string = (char *)malloc(hex_string_len);

    if (hex_string == NULL) {
        fprintf(stderr, "Error allocating memory for hexadecimal string.\n");
        free(ciphertext);
        close(sock);
        return;
    }

    hex_string[0] = '\0';

    for (int i = 0; i < ciphertext_len; i++) {
        sprintf(hex_string + strlen(hex_string), "%02X", ciphertext[i]);
    }

    hex_string[hex_string_len - 1] = '\0';
    printf("\n");

    // Send the hexadecimal string
    send(sock, hex_string, strlen(hex_string), 0);
    printf("\nMessage sent.\n");

    close(sock);
    free(ciphertext);
    free(hex_string);
}


//Calling receiving every 2 seconds
void *receive_thread(void *server_fd)
{
    int s_fd = *((int *)server_fd);
    while (1)
    {
        sleep(2);
        receiving(s_fd);
    }
}

void receiving(int server_fd) {
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    int client_socket;

    if ((client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    char buffer[2048] = {0};
    int valread = recv(client_socket, buffer, sizeof(buffer), 0);
    if (valread == -1) {
        perror("Error receiving message");
        close(client_socket);
        return;
    }

    printf("\n\n\n-----START RECIVING MESSASGEE-----\n\n%s\n\n----END----", buffer);
    int buf_len= strlen(buffer)/2;
    //decrittografa(buffer,buf_len,pathToHostPrivKey);
    char* pathPrivKeyHost=pathToHostPrivateKey;
    char *decrDef= decrittografa(buffer,buf_len,pathPrivKeyHost);
    printf("\n\n%s",decrDef);
    close(client_socket);
}
