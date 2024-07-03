#include <sys/stat.h>
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

#include "librerie/configuringVpn.h"
#include "librerie/readFile.h"
#include "librerie/file_sender_new.h"
//#include "librerie/main_p2p_chat.h"
#include "librerie/miaLibVarie.h"
#include "librerie/new_version_p2p_chat.h"


#define MAX_IP_LENGTH 18
#define PORT 51810
#define MAX_BUF_SIZE 1024

#define PATH_TRACKER_DIRECTORY "http://13.53.40.109:8080/home/ubuntu/apple/"
#define TRACKER_SERVER_IP "13.53.40.109"

//conditional debug
// Define DEBUG to enable debug mode
// This can be defined in the code or via the compiler command line
//#define DEBUG

#ifdef DEBUG
    #define DEBUG_PRINT(fmt, args...)    fprintf(stderr, "DEBUG: " fmt, ## args)
#else
    #define DEBUG_PRINT(fmt, args...)    /* Don't do anything in release builds */
#endif



char* retrieve_assigend_private_ip(const char* host, int port);  // -> from the tracker server
void download_file(const char *base_url, const char *ip_folder, const char *filename); // -> from the tracker server

int main()
{
	char cwd[1024];
	getcwd(cwd, sizeof(cwd));
	DEBUG_PRINT("%s", cwd);

	char *pathVPNconfiguration = concatenateStrings(cwd, "/.vpn-secrets/wg0_vpn.conf");
	char *pathWireguardPrivateKey = concatenateStrings(cwd, "/.vpn-secrets/privatekey");



	//connecting to the tracker so it can assign this machine's private ip address (for the vpn connection)
	char static_assigned_private_ip[MAX_IP_LENGTH];
	char *assigned_private_ip = retrieve_assigend_private_ip(TRACKER_SERVER_IP,8080);

 	if (assigned_private_ip == NULL) {
        fprintf(stderr, "Failed to retrieve the assigned private IP address. An error occurred, the tracker server might be down. Quitting...\n");
		free(assigned_private_ip);
		//qui aassegni manualmente l'ip e vaffanculo
        return 1;
    	}

    strncpy(static_assigned_private_ip, assigned_private_ip, MAX_IP_LENGTH - 1);
    static_assigned_private_ip[MAX_IP_LENGTH - 1] = '\0'; // Ensure null-termination
	free(assigned_private_ip);
    DEBUG_PRINT("\n\nPRIVATE IP ASSIGNED BY THE TRACKER SERVER = %s\n\n", static_assigned_private_ip);



	//writing the vpn and the openssl configs
	int vpnConf_errorHandler = vpnConf(cwd);
	if(vpnConf_errorHandler != 0){
	fprintf(stderr, "Error while configuring the VPN services. \n");
	return 1;
	}

	crittografiaSetup(cwd);


	//this creates the config file for the wireguad configuration
	char *wireguardPrivateKey = readFile(pathWireguardPrivateKey);
	writeConfFile(pathVPNconfiguration, wireguardPrivateKey, PORT, static_assigned_private_ip);	//only this machine's interface, no peer




	char *pathHostPubKey=concatenateStrings(cwd,"/.secrets/public_key.pem");
	char *hostPubKey= readFile(pathHostPubKey);
	char *path_HostConfForPeer=concatenateStrings(cwd,"/.vpn-secrets/HostConfForPeer");



	//writing the wireguard configuration that other peer's will append in their wg config file to connect to this ip
	configurazione_Peer_per_questo_host(cwd, static_assigned_private_ip);

	sleep(2);



	//sending the files needed for the communication with the other host to the tracker. should implement a check
	sender_nuovo(path_HostConfForPeer);
	sender_nuovo(pathHostPubKey);


	/* THIS IS THE FIRST CHECKPOINT. SO FAR SO GOOD.
	THE TRACKER HAS OUR DATA, AND WE'VE BEEN ASSIGNED A PRIVATE IP
	*/



	printf("\nNow let's get the peer stuff! Insert the other dude's PUBLIC ipv4: ");
	char ip_query[20];
	scanf("%s", &ip_query);

	//writing the full name of the folder (on the server)
	char long_query[30];
	snprintf(long_query, sizeof(long_query), "%s-files", ip_query);


/*DOWNLOADING the HostConfForPeer, the public_key.pem for ssl encryption
and also THE PRIVATE IP TO PUT IN THE SECUREP2PCHAT FUNCTION to enable the connection
*/
    download_file(PATH_TRACKER_DIRECTORY, long_query, "HostConfForPeer");
    download_file(PATH_TRACKER_DIRECTORY, long_query, "public_key.pem");
    download_file(PATH_TRACKER_DIRECTORY, long_query, "private_address");


	//movving all in the folder for that ip
	char peer_folder_on_host[200];
	snprintf(peer_folder_on_host, sizeof(peer_folder_on_host), ".%s-info", long_query);
	mkdir(peer_folder_on_host, 0777);

	//moving files into folder
	char moving_files_comand[300];
	snprintf(moving_files_comand, sizeof(moving_files_comand), "mv private_address public_key.pem HostConfForPeer %s", peer_folder_on_host);
	system(moving_files_comand);



	//reading from the downloaded the file the peeer's private ip
	char path_private_ip_address_peer[200];
	snprintf(path_private_ip_address_peer, sizeof(path_private_ip_address_peer), "%s/%s/private_address",cwd,peer_folder_on_host);
	char *private_ip_of_peer= readFile(path_private_ip_address_peer);

	DEBUG_PRINT("\n\n\nPrivate ip of peer: %s\n\n\n",private_ip_of_peer);




	// appending the peer's info to our wg config file
	char sourceFile[200];
	snprintf(sourceFile, sizeof(sourceFile), "%s/%s/HostConfForPeer", cwd, peer_folder_on_host);
	char destFile[200];
	snprintf(destFile, sizeof(destFile), "%s/.vpn-secrets/wg0_vpn.conf", cwd);

	FILE *src = fopen(sourceFile, "r");
    if (src == NULL) {
        perror("Error opening source file");
        exit(EXIT_FAILURE);
    }

    FILE *dest = fopen(destFile, "a");
    if (dest == NULL) {
        perror("Error opening destination file");
        fclose(src);
        exit(EXIT_FAILURE);
    }

    char ch;
    while ((ch = fgetc(src)) != EOF) {
        fputc(ch, dest);
    }

    fclose(src);
    fclose(dest);





	//retriving data for initalzing our chat
	char peerPubKey_due[200];
	snprintf(peerPubKey_due, sizeof(peerPubKey_due), "%s/%s/public_key.pem", cwd, peer_folder_on_host); //quessto è wrong, deve essere il path della folder del peer

    char hostPrivateKey[250];
	snprintf(hostPrivateKey, sizeof(hostPrivateKey), "%s/.secrets/private_key.pem", cwd);


/*
CHECKPOINT 2 REACHED.
If all was correctly executed, now we should start the wg tunnel so that the encr chat can start.

Al 26 giugno, tutto funziona perfettamente fino a questo putno. Ci sono solo dei
problemi per quanto riguarda la memoria. Chiaramente, da questo punto in acanti non funzia
Inoltre, dovrai inserire un controllo che disattiva la conf wireguard quando si quitta
*/


	//qui ci deve esseee attivazione vpn, sennò no funzia
    printf("\nACTIVACTING VPN...");
    sleep(2);

    char attiva_vpn[150];
    snprintf(attiva_vpn, sizeof attiva_vpn,"sudo wg-quick up %s", destFile); //comando per attivare vpn
    system(attiva_vpn);


    //new version
    secureP2Pchat(private_ip_of_peer, peerPubKey_due, hostPrivateKey);



    //now wee should erease the config file and the folder
	free(pathVPNconfiguration);
	free(pathWireguardPrivateKey);

	return 0;

}





char* retrieve_assigend_private_ip(const char* host, int port) {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[MAX_BUF_SIZE];
    char* result = NULL;

    // Create socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        return NULL;
    }

    // Initialize server address structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(host);

    // Connect to server
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Connection failed");
        close(sockfd);
        return NULL;
    }

    // Receive data
    ssize_t bytes_received = recv(sockfd, buffer, MAX_BUF_SIZE - 1, 0);
    if (bytes_received == -1) {
        perror("Receive failed");
        close(sockfd);
        return NULL;
    }

    // Null-terminate received data
    buffer[bytes_received] = '\0';

    // Copy received data to a dynamically allocated string
    result = strdup(buffer);
    if (result == NULL) {
        perror("Memory allocation failed");
        close(sockfd);
        return NULL;
    }

    // Close socket
    close(sockfd);

    return result;
}





void download_file(const char *base_url, const char *ip_folder, const char *filename) {
    char command[300];
    snprintf(command, sizeof(command), "wget %s/%s/%s", base_url, ip_folder, filename);
    system(command);
}
