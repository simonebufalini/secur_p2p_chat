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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "readFile.h"
#include "miaLibVarie.h"

#include "RSACommLib.h"



int vpnConf(char *workingDirectory)
{
    //creating the folder
    int ret;
    char *tmp = concatenateStrings("mkdir -p ", workingDirectory);
    if (!tmp)
        return -1;

    char *comandoCreazioneDir = concatenateStrings(tmp, "/.vpn-secrets/");
    free(tmp); // Free tmp after use
    if (!comandoCreazioneDir)
        return -1;

    ret = system(comandoCreazioneDir);
    free(comandoCreazioneDir); // Free comandoCreazioneDir after use
    if (ret != 0)
    {
        fprintf(stderr, "Failed to create directory\n");
        return ret;
    }

        //generating keys for the wireguard config
    ret = system("wg genkey | tee privatekey | wg pubkey > publickey");
    if (ret != 0)
    {
        fprintf(stderr, "Failed to generate keys\n");
        return ret;
    }

    char *dir = concatenateStrings(workingDirectory, "/.vpn-secrets/");
    if (!dir)
        return -1;

    char comando[200];
    snprintf(comando, sizeof(comando), "mv privatekey %s", dir);
    ret = system(comando);
    if (ret != 0)
    {
        fprintf(stderr, "Failed to move privatekey\n");
        free(dir); // Free dir before returning
        return ret;
    }

    char comando_due[200];
    snprintf(comando_due, sizeof(comando_due), "mv publickey %s", dir);
    ret = system(comando_due);
    free(dir); // Free dir after use
    if (ret != 0)
    {
        fprintf(stderr, "Failed to move publickey\n");
        return ret;
    }

    return 0;
}


void crittografiaSetup(char *workingDirectory)
{
	char dirSecret[200];
	snprintf(dirSecret, sizeof(dirSecret), "%s/.secrets/", workingDirectory);

	//system("openssl genrsa -out private_key.pem 2048");
	//system("openssl rsa -pubout -in private_key.pem -out public_key.pem");

	//generating keys with simons stuff
    const char *private_key_file = "private_key.pem";
    const char *public_key_file = "public_key.pem";
    const int key_size = 2048;

    // Step 1: Generate RSA keys
    if (RSACommLib_Init(key_size, private_key_file, public_key_file) != 0) {
        fprintf(stderr, "Failed to initialize RSA keys\n");
        //return 1;
    }
	mkdir(dirSecret, 0777);

	char comandoSpostaChiavi[300];
	snprintf(comandoSpostaChiavi, sizeof(comandoSpostaChiavi), "mv private_key.pem public_key.pem %s", dirSecret);

	int execute = system(comandoSpostaChiavi);
	if(execute != 0){
		printf("\nAn error occurred with the configuring vpn filez");
	}

}



int writeConfFile(const char *configFile, const char * privateKey, int listenPort,
  const char * address) {

  FILE * file;

  // Open the file for writing
  file = fopen(configFile, "w");

  // Check if the file was opened successfully
  if (file == NULL) {
    printf("Unable to create file.\n");
    return 1;
  }

  // Write the content to the file
  fprintf(file, "[Interface]\nPrivateKey = %s\nListenPort = %d\nAddress = %s\n\n",
    privateKey, listenPort, address);

  // Close the file
  fclose(file);
  return 0;
}







void configurazione_Peer_per_questo_host(char* workingDirectory, char *assigned_private_ip)
{

	char shareFileName[] = "HostConfForPeer";

	char *secrets_dir = concatenateStrings(workingDirectory, "/.vpn-secrets/");

	char *pathWireguardPublicKey = concatenateStrings(workingDirectory, "/.vpn-secrets/publickey");

	char *hostIp = getHostIp();
	char *wg_pub_key = readFile(pathWireguardPublicKey);

	FILE *file_pointer = fopen(shareFileName, "w");
	// Write to the file
	fprintf(file_pointer, "[PEER]\n");
	fprintf(file_pointer, "PublicKey =  %s\n", wg_pub_key);
	fprintf(file_pointer, "AllowedIPs = %s/24\n", assigned_private_ip);
	fprintf(file_pointer, "Endpoint = %s:51810\n", hostIp);
	fprintf(file_pointer, "PersistentKeepalive = 25\n");

	// Close the file
	fclose(file_pointer);

	char comando[200];
	sprintf(comando, "mv %s %s", shareFileName, secrets_dir);

	system(comando);


}
