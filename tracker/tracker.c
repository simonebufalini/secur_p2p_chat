/*
 * gabricampaa ©
 *
 *
 * This is the code that should run on the tracker serveer
 *
 *
*/

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>

#define BUFFER_SIZE 1024
#define PORT 8080

#define MAX_CONNECTIONS 5
#define HASH_TABLE_SIZE 255
#define IP_ADDRESS_LENGTH 16
#define NUM_PORTS 5

const int available_ports[NUM_PORTS] = {6969, 51810, 51812, 51811, 8080};


void handle_client_dos(int client_socket, struct sockaddr_in client_address) {

    //finding client's public ipv4 as soon as he connects
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(client_address.sin_addr), client_ip, INET_ADDRSTRLEN);


    //creating a folder with the client's ip - example: 192.168.1.1-files
    char folderName[100];
    snprintf(folderName, sizeof(folderName), "%s-files", client_ip);
    int result = mkdir(folderName, 0777);
    //should add error handling


    char buffer[BUFFER_SIZE];
    char file_name[BUFFER_SIZE];
    int file_name_index = 0;
    ssize_t bytes_received;



    // Reeceiving th filee - reading the name of the file being sent - and saving the file with that name
    while ((bytes_received = recv(client_socket, buffer, 1, 0)) > 0) {
        if (buffer[0] == '\n') {
            file_name[file_name_index] = '\0';
            break;
        }
        if (file_name_index < BUFFER_SIZE - 1) {
            file_name[file_name_index++] = buffer[0];
        } else {
            fprintf(stderr, "Nome del file troppo lungo\n");
            close(client_socket);
            return;
        }
    }

    if (bytes_received <= 0) {
        perror("Errore nella ricezione del nome del file");
        close(client_socket);
        return;
    }

    FILE *file = fopen(file_name, "wb");
    if (file == NULL) {
        perror("Errore nell'apertura del file");
        close(client_socket);
        return;
    }

    // Recieve and write the file's content
    while ((bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0)) > 0) {
        fwrite(buffer, 1, bytes_received, file);
    }

    if (bytes_received < 0) {
        perror("Errore nella ricezione dei dati");
    }

    fclose(file);

    char fullMvComand[300];
    snprintf(fullMvComand, sizeof(fullMvComand),"mv %s %s",file_name, folderName);
    system(fullMvComand);
    close(client_socket);


/*
UNIFY THE FILES IN A SINGLE TAR ARCHIVE. EASIER TO SEND AND RETRIEVE
//create tar not two files
char tarComand[300];
    snprintf(tarComand, sizeof(tarComand),"tar -czvf myfolder.tar.gz %s", folderName);
    system(tarComand);

    //system("mv myfolder.tar.gz %s", folderName);
    close(client_socket);
    //here u tar?
    //tar -czvf myfolder.tar.gz myfolder
    //tar -xzvf myfolder.tar.gz
*/
}




//handler for the sending and reciving of files - a parallel operation
int new_reciver() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    // Creating socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Errore nella creazione del socket");
        exit(EXIT_FAILURE);
    }

    // Configuring the address and the port
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(51812);

    // Binding the socket
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Errore nel binding del socket");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Start listening
    if (listen(server_socket, 5) == -1) {
        perror("Errore nell'ascolto del socket");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server in ascolto sulla porta %d...\n", PORT);

    // Infinite cyclle to keep listener always alive
    while (1) {
        // Accettazione delle connessioni in arrivo
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_socket == -1) {
            perror("Errore nell'accettazione della connessione");
            continue;
        }

        printf("Connessione accettata...\n");

        // Gestione del client in una nuova funzione
        if (fork() == 0) { // Processo figlio
            close(server_socket); // Il processo figlio non ha bisogno del socket del server
            handle_client_dos(client_socket, client_addr);
            printf("File ricevuto e connessione chiusa.\n");
            exit(0);
        }

        // Il processo padre chiude il socket del client e continua ad accettare nuove connessioni
        close(client_socket);
    }

    // Closing the socket
    close(server_socket);

    return 0;
}



//creates a folder named 'ip_client - files' - returns path of that folder
char *storeClientFiles(char *ipOfTheMachine)
{

    char folderPath[1024];
    getcwd(folderPath, sizeof(folderPath));

    char comando[500];
    snprintf(comando, sizeof(comando),"mkdir '%s/%s-files'",folderPath, ipOfTheMachine);
    system(comando);

    char *path = (char *)malloc(strlen(folderPath) + strlen(ipOfTheMachine) + 8); // 8 is for "/-files\0"
    if (path == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    sprintf(path, "%s/%s-files", folderPath, ipOfTheMachine);

    // Free the memory allocated by getcwd
    //free(folderPath);

    /*
    *
    *
    * qui succede che noi riceviamo la chiave epubblica per la chrittografia E il sample del peer (che sono stati generati con le info generate dal peer)
    *
    *
    *
    *
    */

    return path;

}


void handle_sigpipe(int sig) {
    // Ignora il segnale SIGPIPE
}




//creating an hash table to assign private ips to the client (and checking it it had alreay been assigned)
typedef struct Node {
    char key[IP_ADDRESS_LENGTH]; // IP pubblico
    char ip_address[IP_ADDRESS_LENGTH]; // IP privato
    struct Node* next;
} Node;

typedef struct HashTable {
    Node* head;
    int num_connections;
} HashTable;

Node* create_node(const char* key, const char* ip_address) {
    Node* new_node = (Node*)malloc(sizeof(Node));
    if (new_node == NULL) {
        fprintf(stderr, "Memoria esaurita\n");
        exit(EXIT_FAILURE);
    }
    strcpy(new_node->key, key);
    strcpy(new_node->ip_address, ip_address);
    new_node->next = NULL;
    return new_node;
}

void insert(HashTable* hash_table, const char* key, const char* ip_address) {
    Node* new_node = create_node(key, ip_address);
    new_node->next = hash_table->head;
    hash_table->head = new_node;
    hash_table->num_connections++;
}

char* search(HashTable* hash_table, const char* key) {
    Node* current = hash_table->head;
    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            return current->ip_address;
        }
        current = current->next;
    }
    return NULL;
}

void hash_display(HashTable* hash_table) {
    printf("Numero di connessioni attive: %d\n", hash_table->num_connections);
    printf("Active Nodes:\n");
    Node* current = hash_table->head;
    while (current != NULL) {
        printf("Key: %s, IP Address: %s\n", current->key, current->ip_address);
        current = current->next;
    }
}



//handling main function - check if the peer is asking for files to download or if it only has to be assigned a private ip
void handle_client(int client_socket, struct sockaddr_in client_address, HashTable* hash_table, int* last_suffix) {


    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(client_address.sin_addr), client_ip, INET_ADDRSTRLEN);
    printf("Client IPv4 address: %s\n", client_ip);

  // Imposta il timeout per la ricezione dei dati dal client a x secondi
    struct timeval timeout;
    timeout.tv_sec = 1;  // 1 secondo
    timeout.tv_usec = 0;

    if (setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
        perror("Error setting receive timeout");
        close(client_socket);
        pthread_exit(NULL);
    }



    char request[1024];
    recv(client_socket, request, sizeof(request), 0);
    signal(SIGPIPE, handle_sigpipe);

 if (strncmp(request, "GET", 3) == 0) {
        // Estrai il percorso richiesto (dalla seconda parola della richiesta)
        char *path_start = strchr(request, ' ');
        if (path_start != NULL) {
            path_start++; // Sposta il puntatore oltre lo spazio
            char *path_end = strchr(path_start, ' ');
            if (path_end != NULL) {
                *path_end = '\0'; // Termina la stringa al termine del percorso
                // Ora "path_start" contiene il percorso richiesto
                printf("Requested path: %s\n", path_start);

                // Apri il file richiesto in modalità di lettura binaria
                FILE *file = fopen(path_start, "rb");
                if (file == NULL) {
                    perror("Error opening file");
                    close(client_socket);
                    return;
                }

                // Leggi e trasmetti il contenuto del file al client
                char buffer[1024];
                size_t bytes_read;
                while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
                    if (send(client_socket, buffer, bytes_read, 0) != bytes_read) {
                        perror("Error sending file");
                        fclose(file);
                        close(client_socket);
                        return;
                    }
                }

                // Chiudi il file dopo aver completato la trasmissione
                fclose(file);

            }
                    close(client_socket);

        }

        return;

    }else{

        printf("\nNon è richiesta get!\n");
       // close(client_socket);
    }




            //creats folder for that peer
            char *peer_folder_path = storeClientFiles(client_ip);


    ssize_t bytes_sent; //THIS IS WHAT SENDS BACK THE ASSIGNED PRIVED IP TO THE MACHINE

    char* ip_privato = search(hash_table, client_ip);
    if (ip_privato != NULL) {
        printf("L'indirizzo IP privato per %s è: %s\n", client_ip, ip_privato);
        bytes_sent =  send(client_socket,ip_privato,strlen(ip_privato),0);

    } else {
        // Costruisci l'indirizzo IP privato nell'intervallo richiesto
        char ip_address[IP_ADDRESS_LENGTH];
        snprintf(ip_address, IP_ADDRESS_LENGTH, "10.0.0.%d", *last_suffix);

        // Inserisce l'indirizzo IP nella hash table
        insert(hash_table, client_ip, ip_address);
        printf("Indirizzo IP privato assegnato per %s: %s\n", client_ip, ip_address);
        (*last_suffix)++;
        bytes_sent =  send(client_socket,ip_address,strlen(ip_address),0);


        char cwd[100];
        getcwd(cwd, sizeof(cwd));

        char path_to_ip_file[200];
        snprintf(path_to_ip_file, sizeof(path_to_ip_file),"%s/private_address",peer_folder_path);

        FILE *file = fopen(path_to_ip_file, "w");
        if (file == NULL) {
            perror("Errore nell'apertura del file");
            close(client_socket);
            return;
        }
        //writing the private ip file to the file
        fprintf(file, ip_address);
        fclose(file);



    }

    printf("\nCurrent state of the table. \n");
    hash_display(hash_table);

    close(client_socket);

}




// Function that will be run by the thread
void *thread_function(void *arg) {
    new_reciver();
}







int main() {

    pthread_t thread; // Thread identifier
    int result;

    // Create the thread
    result = pthread_create(&thread, NULL, thread_function, NULL);
    if (result != 0) {
        fprintf(stderr, "Error creating thread\n");
        return 1;
    }

    HashTable hash_table = {NULL, 0};

    int server_socket, client_socket;
    socklen_t client_length;
    struct sockaddr_in server_address, client_address;


    // Create a socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Error opening socket");
        exit(EXIT_FAILURE);
    }

    // Initialize server address structure
    memset((char *)&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    // Bind the socket to the address and port
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Error on binding");
        exit(EXIT_FAILURE);
    }

    // Start listening for incoming connections
    listen(server_socket, MAX_CONNECTIONS);
    printf("Server listening on port %d\n", PORT);

    int last_suffix = 1;

    while (1) {
        // Accept a client connection
        client_length = sizeof(client_address);
        client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_length);
        if (client_socket < 0) {
            perror("Error on accept");
            exit(EXIT_FAILURE);
        }

        printf("\n\n\nConnection from: %s:%d\n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));

        // Handle the client connection
        handle_client(client_socket, client_address, &hash_table, &last_suffix);
    }

    // Free memory allocated for the hash table
    Node* current = hash_table.head;
    while (current != NULL) {
        Node* temp = current;
        current = current->next;
        free(temp);
    }

    return 0;
}
