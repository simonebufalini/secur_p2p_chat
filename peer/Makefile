CC = gcc
CFLAGS  = -lssl -pthread -lcrypto -lcurl -w #-Wall
TARGET = main
executable_name = main_beta
other_files = librerie/RSACommLib.c librerie/readFile.c librerie/configuringVpn.c librerie/file_sender_new.c librerie/miaLibVarie.c librerie/p2p_chat.c

dependencies =libcurl, wget, openssl, gcc, wireguard, sudo apt install libssl-dev

compile:
	$(CC) $(TARGET).c -o $(executable_name) $(other_files) $(CFLAGS)

compile_debug:
	$(CC) $(TARGET).c -o $(executable_name)_debug $(other_files) $(CFLAGS) -DDEBUG





clean:
	rm -f $(executable_name)
	rm -f $(executable_name)_debug
	sudo rm -rf .vpn-secrets
	sudo rm -rf .secrets
	sudo rm -f public_key.pem
	sudo rm -f HostConfForPeer
	sudo rm -rf main_beta.dSYM

run:
	./$(executable_name)


run_debug:
	./$(executable_name)_debug
