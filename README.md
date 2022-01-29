Nicholas Mark, 305101337
# CS118 Project 1

This is the repo for winter22 cs118 project 1.

## server.cpp
1. The server creates a socket and binds it to port 8080
1. Then it listens to the socket and accepts requests from the socket.
1. Next the server reads from the socket and parses the HTTP request to extract the filename in the URL
1. It then checks if that file exists in the filesystem
    1. If the file name does, it reads the file and responds to the client with an HTTP header and the file data as the body.
    1. If the file does not exist, the server will check if a file with the same name but an extra extension exists.
        1. If so, it does the previous step
        1. If not, the server responds with 404 Not Found

## Troubleshooting
1. I had issues getting the file without the extention. I ended up brute-forcing it and querying the directory for files that start the same, but contains some '.*' after.
1. It also took me a while to figure out why my parsing function wasn't working. Turned out there were hidden whitespace characters in the HTTP header that I didn't account for.
1. I also spent time figuring out why some files returned with the expected data, and other files didn't. It was because I was concatting the binary data with the header string, so once I separated the send() responses, everything seemed to work.

## Libraries
1. <iostream>
1. <sys/socket.h> // create socket
1. <cstdlib>
1. <unistd.h>
1. <netinet/in.h> // create socket
1. <signal.h> //handling Ctrl-C
1. <string>
1. <fstream> // read file
1. <filesystem> // get size of file
1. <time.h> // time_t, time, tm
1. <algorithm> // convert to lowercase
1. <cctype> 
1. <cstring> // strlen
1. <sys/types.h>
1. <sys/stat.h> // get information on files such as existence and modification time


## Ackowledgments
1. I read the GeeksforGeeks page on socket programming to learn about each step in setting up a socket. I learned how to use each of the socket API's and libraries to include.
1. I used Stefan Mai's toLower() function from Stackoverflow to convert strings to lowercase.


