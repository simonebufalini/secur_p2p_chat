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
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdbool.h>
#include <libgen.h> // For basename()

#define MAX_FILE_NAME 150
#define BUFFER_SIZE 1024



char* get_cwd(){

    char cwd[1024];

    // Get the current working directory
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("Current working directory: %s\n", cwd);

        // Concatenate the current working directory with "./secret"
        char pathBuffer[1024];
        //snprintf(pathBuffer, sizeof(pathBuffer), "%s%s", cwd, "/.secret");
        snprintf(pathBuffer, sizeof(pathBuffer), "%s", cwd);

        // Assign a pointer to the concatenated path
        char *confFilePath = pathBuffer;

        printf("Concatenated path: %s\n", confFilePath);
        return confFilePath;
    } else {
        perror("getcwd() error");
        return "ERROR";
    }
}


char* concatenateStrings(const char* str1, const char* str2) {
    // Alloca memoria per la nuova stringa, considerando la lunghezza di entrambe le stringhe più il terminatore NULL
    char* result = malloc(strlen(str1) + strlen(str2) + 1);
    if (result == NULL) {
        fprintf(stderr, "Errore durante l'allocazione di memoria\n");
        exit(1);
    }

    // Copia la prima stringa nella nuova stringa
    strcpy(result, str1);

    // Concatena la seconda stringa alla fine della prima
    strcat(result, str2);

    return result;
}





size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
    // Concatenate received data into userdata
    strcat(userdata, ptr);
    return size * nmemb;
}



// Function to retrieve public IPv4 address
char* getHostIp() {
    CURL *curl;
    CURLcode res;

    // Initialize curl
    curl = curl_easy_init();
    if (curl) {
        // Set URL to get public IP
        curl_easy_setopt(curl, CURLOPT_URL, "http://api.ipify.org/");

        // Response buffer
        char* response = (char*)malloc(100);
        if (!response) {
            fprintf(stderr, "Memory allocation failed.\n");
            return NULL;
        }
        response[0] = '\0';

        // Set callback function to write response to buffer
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);

        // Perform HTTP request
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            free(response);
            return NULL;
        }

        // Cleanup
        curl_easy_cleanup(curl);

        // Return public IP address
        return response;
    } else {
        fprintf(stderr, "Failed to initialize curl.\n");
        return NULL;
    }
}




char* askQuestion(const char *question) {
    printf("%s", question);

    char *response = (char*)malloc(100); // Allocate memory for the response
    if (response == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }

    // Read the input line by line
    if (fgets(response, 100, stdin) == NULL) {
        fprintf(stderr, "Error reading input.\n");
        free(response); // Free allocated memory
        exit(EXIT_FAILURE);
    }

    // Remove trailing newline character, if present
    response[strcspn(response, "\n")] = '\0';

    return response;
}







void error(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

typedef struct {
    char serverIp[16];
    int port;
    char filePath[256];
} ThreadArgs;



void sender(const char *server_ip,int server_port,const char *file_path) {

    // Open the file
    FILE *file = fopen(file_path, "rb");
    if (file == NULL) {
        error("Error opening file");
    }



    // Create socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        error("Error creating socket");
    }

    // Set up server address
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(server_ip);
    server_addr.sin_port = htons(server_port);

    // Connect to server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        error("Error connecting to server");
    }

    //send(sockfd, file_path, strlen(file_path), 0);


    // Read and send file contents
    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
        if (send(sockfd, buffer, bytes_read, 0) == -1) {
            error("Error sending file");
        }
    }

    // Close file and socket
    fclose(file);
    close(sockfd);

    printf("File sent successfully.\n");

   // pthread_exit(NULL);
}

void *receiver(void *arg) {
    ThreadArgs *args = (ThreadArgs *)arg;
    int port = args->port;
    const char *output_file = args->filePath;

    // Create socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        error("Error creating socket");
    }

    // Set up server address
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    // Bind socket
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        error("Error binding socket");
    }

    // Listen for connections
    if (listen(sockfd, 5) == -1) {
        error("Error listening");
    }

    // Accept connection
    int client_sockfd = accept(sockfd, (struct sockaddr *)&client_addr, &client_addr_len);
    if (client_sockfd == -1) {
        error("Error accepting connection");
    }

    // Open file for writing
    FILE *file = fopen(output_file, "wb");
    if (file == NULL) {
        error("Error opening file for writing");
    }

    // Receive file contents
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;
    while ((bytes_received = recv(client_sockfd, buffer, BUFFER_SIZE, 0)) > 0) {
        fwrite(buffer, 1, bytes_received, file);
    }

    if (bytes_received == -1) {
        error("Error receiving file");
    }

    // Close file and sockets
    fclose(file);
    close(client_sockfd);
    close(sockfd);

    printf("File received successfully.\n");

    pthread_exit(NULL);
}








int Search_in_File(char *fname, char *str) {
	FILE *fp;
	int line_num = 1;
	int find_result = 0;
	char temp[512];

	if((fp = fopen(fname, "r")) == NULL) {
		return(-1);
	}



	while(fgets(temp, 512, fp) != NULL) {
		if((strstr(temp, str)) != NULL) {
			printf("A match found on line: %d\n", line_num);
			printf("\n%s\n", temp);
			find_result++;
		}
		line_num++;
	}

	if(find_result == 0) {
		printf("\nSorry, couldn't find a match.\n");
	}

	//Close the file if still open.
	if(fp) {
		fclose(fp);
	}
   	return(0);
}



int reciver(int choosenPort, char *outputFile) {


    int port = choosenPort;
    const char *output_file = outputFile;

    // Create socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        error("Error creating socket");
    }

    // Set up server address
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    // Bind socket
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        error("Error binding socket");
    }

    // Listen for connections
    if (listen(sockfd, 5) == -1) {
        error("Error listening");
    }

    // Accept connection
    int client_sockfd = accept(sockfd, (struct sockaddr *)&client_addr, &client_addr_len);
    if (client_sockfd == -1) {
        error("Error accepting connection");
    }

    // Open file for writing
    FILE *file = fopen(output_file, "wb");
    if (file == NULL) {
        error("Error opening file for writing");
    }

    // Receive file contents
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;
    while ((bytes_received = recv(client_sockfd, buffer, BUFFER_SIZE, 0)) > 0) {
        fwrite(buffer, 1, bytes_received, file);
    }

    if (bytes_received == -1) {
        error("Error receiving file");
    }

    // Close file and sockets
    fclose(file);
    close(client_sockfd);
    close(sockfd);

    printf("File received successfully.\n");

    return 0;
}



int writeToFilePath(const char *filePath, const char * textToWrite) {
    FILE *file = fopen(filePath, "a"); // Open file in write mode
    if (file == NULL) {
        perror("Error opening file");
        return -1; // Return -1 to indicate failure
    }

    // Write the variable's content into the file
    fprintf(file, "%s", textToWrite);

    fclose(file); // Close the file
    return 0; // Return 0 to indicate success
}
