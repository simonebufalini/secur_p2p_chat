#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>


#define PORT 51812
#define BUFFER_SIZE 1024



void send_file(FILE *file, int socket) {
    char buffer[BUFFER_SIZE];
    size_t bytes_read;

    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
        if (send(socket, buffer, bytes_read, 0) == -1) {
            perror("Errore nell'invio del file");
            exit(EXIT_FAILURE);
        }
    }
}

//this will have to take the tracker's ip
int sender_nuovo(char *fileDaInviare[]){

    const char *file_path = fileDaInviare;
    const char *file_name = strrchr(file_path, '/');
    file_name = (file_name == NULL) ? file_path : file_name + 1;

    FILE *file = fopen(file_path, "rb");
    if (file == NULL) {
        perror("Errore nell'apertura del file");
        exit(EXIT_FAILURE);
    }

    int client_socket;
    struct sockaddr_in server_addr;

    // Creazione del socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Errore nella creazione del socket");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    // Configurazione dell'indirizzo del server
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT); //tracker port
    server_addr.sin_addr.s_addr = inet_addr("13.53.40.109"); //tracker ip

    // Connessione al server
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Errore nella connessione al server");
        close(client_socket);
        fclose(file);
        exit(EXIT_FAILURE);
    }

    printf("Connesso al server. Inizio invio del file...\n");

    // Invio del nome del file
    if (send(client_socket, file_name, strlen(file_name), 0) == -1) {
        perror("Errore nell'invio del nome del file");
        close(client_socket);
        fclose(file);
        exit(EXIT_FAILURE);
    }

    // Invio di un terminatore per indicare la fine del nome del file
    if (send(client_socket, "\n", 1, 0) == -1) {
        perror("Errore nell'invio del terminatore del nome del file");
        close(client_socket);
        fclose(file);
        exit(EXIT_FAILURE);
    }

    // Invio del file
    send_file(file, client_socket);

    printf("File inviato con successo.\n");

    // Chiusura del file e del socket
    fclose(file);
    close(client_socket);

    return 0;
}

/*
int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <file da inviare>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *file_path = argv[1];
    const char *file_name = strrchr(file_path, '/');
    file_name = (file_name == NULL) ? file_path : file_name + 1;

    FILE *file = fopen(file_path, "rb");
    if (file == NULL) {
        perror("Errore nell'apertura del file");
        exit(EXIT_FAILURE);
    }

    int client_socket;
    struct sockaddr_in server_addr;

    // Creazione del socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Errore nella creazione del socket");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    // Configurazione dell'indirizzo del server
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("13.53.40.109");

    // Connessione al server
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Errore nella connessione al server");
        close(client_socket);
        fclose(file);
        exit(EXIT_FAILURE);
    }

    printf("Connesso al server. Inizio invio del file...\n");

    // Invio del nome del file
    if (send(client_socket, file_name, strlen(file_name), 0) == -1) {
        perror("Errore nell'invio del nome del file");
        close(client_socket);
        fclose(file);
        exit(EXIT_FAILURE);
    }

    // Invio di un terminatore per indicare la fine del nome del file
    if (send(client_socket, "\n", 1, 0) == -1) {
        perror("Errore nell'invio del terminatore del nome del file");
        close(client_socket);
        fclose(file);
        exit(EXIT_FAILURE);
    }

    // Invio del file
    send_file(file, client_socket);

    printf("File inviato con successo.\n");

    // Chiusura del file e del socket
    fclose(file);
    close(client_socket);

    return 0;
}
*/