# secure_p2p_chat

In the future we are adding a dialog base configuration to facilitatee the process.

--------
Dependencies:
- libssl (sudo apt-get install libssl-dev)
- wget
- wireguard (sudo apt install wireguard)
- curl  (sudo apt install curl)

<b>Run 'make dep' to install them<b>

## Documentation
### 1. Tracker
The Tracker file is a unique source file, with all the necessary parts built in. Just compile it with 
'''sh
gcc tracker.c -o tracker
'''
<b>THE TRACKER ASSUMES THAT<b>
const int available_ports[NUM_PORTS] = {6969, 51810, 51812, 51811, 8080};
http://<pub ip>:<port>/home/ubuntu/apple/ existis
