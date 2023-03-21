## Serve Stuff - MVP server-side web-framework for static sites

Simple web server written from scratch according to the MVP principle.

### Requirements

- Must be able to be assigned an IP-address (currently loopback to 127.0.0.1:<port> is used)
- Must be able to store files uploaded to the server
- Must be able to display files to the client
- Must be able to handle HTTP protocol 1.1

### Blocks

- TCP Socket
- HTTP Parser
- Template engine

### Use it

1. Download the _src_ and place it in a directory of your choice
2. Change current directory to _src_ in the command line
3. Run the command: _make_
4. Run the command: ./app <port> (try port=8080 if in doubt)
5. Search for 127.0.0.1:<port> in your preferred browser
6. Do stuff until it breaks!

### Develop it

- After _make_ is called, remember to _make clean_, or fix the makefile if you want!
- Break it and find out why!
- Good suggestions to the POST handling?
- Enable LAN access?
- Run on Raspberry Pi?
- Database integration?
