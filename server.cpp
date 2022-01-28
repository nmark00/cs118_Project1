#include <iostream>
#include <sys/socket.h>
#include <cstdlib>
#include <unistd.h>
#include <netinet/in.h>

#include <string>
#include <fstream>
#include <filesystem>
#include <time.h> // time_t, time, tm
#include <iomanip> //put_time


#include <vector>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>


using namespace std;
using namespace std::chrono;

#define MAX_CONNS 10
#define BUF_SIZE 4096
#define PORT_NUM 8080

string getStatusline () {
  return "200 OK";
}

string getDatetime (time_t param = -1) {
  // if param is blank, get current time
  time_t rawtime = (rawtime == -1) ? time( &rawtime ) : param;
  struct tm * ptm;
  ptm = gmtime( &rawtime );

  // Tue, 09 Aug 2011 15:44:04 GMT
  return put_time(ptm, "%a, %e %b %Y %T %Z").str();
}

void respondToClient (string fileName,
                      int connection,
                      string contentType)
{
  // Initial header
  string responseHeader = "HTTP/1.1 "+ getStatusline() + "\r\n"
                          "Connection: close\r\n"
                          "Date:" + getDatetime() + "\r\n"
                          "Server: Nico Server\r\n";

  // Open file in READ, preserve \r\n, set cursor to end
  ifstream myfile (fileName, ios::in|ios::binary|ios::ate);

  // File does not exist
  if (!myfile.is_open()) {
    send(connection, responseHeader, responseHeader.length());
    close(fd);
    return;
  }


  auto mtime = filesystem::last_write_time(fileName);
  time_t rawMtime = system_clock::to_time_t(file_clock::to_sys(mtime));

  streampos size = myfile.tellg() //cursor is at EOF

  responseHeader += "Last-Modified: " + getDatetime(rawMtime) + "\r\n"
                    "Content-Length: " + to_string(size) + "\r\n"
                    "Content-Type: " + contentType + "\r\n\r\n";

  // char * memblock = new char[size];
  // file.seekg(0, ios::beg); // reset cursor to beginning
  // file.read(memblock, size); // read and store data in memblock

  send(connection, responseHeader, responseHeader.length());

  // delete[] memblock;

  
  

}

int main()
{
  // Create socket
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd == -1) {
    cout << "Failed to create socket: " << errno << endl;
    exit(EXIT_FAILURE);
  }

  // Use port 8080
  sockaddr_in sockaddr;
  sockaddr.sin_family = AF_INET;
  sockaddr.sin_addr.s_addr = INADDR_ANY;
  sockaddr.sin_port = htons(PORT_NUM);

  // Bind port 8080 to socket
  if (bind(sockfd, (struct sockaddr*) &sockaddr, sizeof(sockaddr)) < 0) {
    cout << "Failed to bind to port 8080: " << errno << endl;
    exit(EXIT_FAILURE);
  }

  // Listen on socket. Hold at most MAX_CONNS connections in the queue
  if (listen(sockfd, MAX_CONNS) < 0) {
    cout << "Failed to listen on socket: " << errno << endl;
    exit(EXIT_FAILURE);
  }

  socklen_t addrlen;

  // Accept connections 
  int connection;
  while ((connection = accept(sockfd, (struct sockaddr*) &sockaddr, (socklen_t*) &addrlen)) > 0) {
    char buffer[BUF_SIZE];
    auto bytesRead = read(connection, buffer, BUF_SIZE);

    if (bytesRead < 0) {
      cout << "Failed connection: " << errno << endl;
      close(connection);
      exit(EXIT_FAILURE);
    }

    cout << "The message was: " << buffer;
    


    string response = "Good talking to you\n";
    send(connection, response.c_str(), response.size(), 0);
    shutdown(connection, 0);
  }

  close(sockfd);

  return 0;
}
