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
To compile and run the code, use

```bash
make compile
run
```

 If you want to compile and run with debug information printed to console, use

```bash
make compile_debug
run_debug
```

Once used, use 
```bash
make clean
```
to clean the diretory (also remember to delete the hidden folders!)

