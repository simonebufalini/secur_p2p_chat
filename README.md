# secure_p2p_chat

In the future we are adding a dialog base configuration to facilitatee the process.

--------
Dependencies:
- libssl (sudo apt-get install libssl-dev)
- wget
- wireguard (sudo apt install wireguard)
- curl  (sudo apt install curl)

Run 'make dep' to install them
____

## Funzionamento

Cose

-----

## Documentation

### 1. Tracker

The Tracker file is a unique source file, with all the necessary parts built in. Just compile it with 
```bash
gcc tracker.c -o tracker
```

THE TRACKER ASSUMES THAT:

```C
    const int available_ports[NUM_PORTS] = {6969, 51810, 51812, 51811, 8080};
    http://<pub ip>:<port>/home/ubuntu/apple/ existis
```
-------------
### 2. Peer
Let's now talk about the big part. 

#### 2.1 Makefile
- To compile and run the code, use

```bash
make compile
run
```

- If you want to compile and run with debug information printed to console, use

```bash
make compile_debug
run_debug
```

Once used, use 
```bash
make clean
```
- to clean the diretory (also remember to delete the hidden folders!)

#### 2.2 main.c
##### - 2.2.1 Headers
- First we include all the libraries, both the 'classics' and the custom written ones.
```C
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
#include "librerie/miaLibVarie.h"
#include "librerie/new_version_p2p_chat.h"
```
- Then we define costants
```C
#define MAX_IP_LENGTH 18
#define PORT 51810
#define MAX_BUF_SIZE 1024
```
- Lasst, but not least, we define the IP and the DIR of the tracker (this should be customizeed accordingly - in the future will write a setupper to aumatically do this when building from source)
```C
#define PATH_TRACKER_DIRECTORY "http://13.53.40.109:8080/home/ubuntu/apple/"
#define TRACKER_SERVER_IP "13.53.40.109"
```
- We later define the conditional debug stuff

```C
#ifdef DEBUG
    #define DEBUG_PRINT(fmt, args...)    fprintf(stderr, "DEBUG: " fmt, ## args)
#else
    #define DEBUG_PRINT(fmt, args...)    /* Don't do anything in release builds */
#endif

```

##### 2.2.2. - Functions
- The "retrieve_assigend_private_ip" function retrives the assgned private ip from the tracker server
- The "download_file" functions downloads a file using wget.
```C
char* retrieve_assigend_private_ip(const char* host, int port);  // -> from the tracker server
void download_file(const char *base_url, const char *ip_folder, const char *filename); // -> from the tracker server
```
