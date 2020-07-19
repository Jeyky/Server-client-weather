## Server-client-weather

A simple bunch of programs that decreace number of requsets to weather API.

Server decreace a number of requests by saving requested data previously.
if necessary to update data server makes request, saves in directory "Cities" and sends to client.

***All server logs happens in logs.txt file which created by server.***

At first executing server than you can execute client (or/and) clien_gui.
server available to get requests from multiple clients.

On server and client programs default ip address is 127.0.0.1, default port 3622,
but you can change it while executing as command line argument.
**For example:**
  ./server 4000
  ./client_gui 4000
  ./client 4000
 
 ### there may be a need to install certain libraries like ljsonc.
$ sudo apt instal libjson-c-dev 
# Build program:

everything builds in directory .build <br />
$ make

