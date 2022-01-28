#include <iostream>
#include <sys/socket.h>
#include <cstdlib>
#include <unistd.h>
#include <netinet/in.h>

#include <string>
#include <fstream> // read file
#include <filesystem> // get size of file
#include <time.h> // time_t, time, tm
#include <boost/filesystem/operations.hpp> // last_write_time
#include <boost/algorithm/string.hpp> // to_lower


#include <vector>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>


using namespace std;

#define MAX_CONNS 10
#define BUF_SIZE 4096
#define PORT_NUM 8080


string getContentType(const string fileName) {
  string extension = fileName.substr(fileName.find_last_of('.') + 1);
  boost::algorithm::to_lower(extension); // make extension lowercase

  string contentType;

  if (extension == "html" || extension == "htm")
    contentType = "text/html";
  else if (extension == "txt")
    contentType = "text/plain";
  else if (extension == "jpg" || extension == "jpeg")
    contentType = "image/jpeg";
  else if (extension == "png")
    contentType = "image/png";
  else if (extension == "gif")
    contentType = "image/gif";
  else contentType = "application/octet-stream";

  return contentType;
}


string getDatetime (time_t param = -1) {
  // if param is blank, get current time
  time_t rawtime = (rawtime == -1) ? time( &rawtime ) : param;
  struct tm * ptm;
  ptm = gmtime( &rawtime );

  char buffer[128];
  strftime(buffer, sizeof(buffer), "%a, %e %b %Y %T %Z", ptm);

  // Tue, 09 Aug 2011 15:44:04 GMT
  return buffer;
}

void responseToClient (const string fileName, const int connection) {

  // Open file in READ, preserve \r\n, set cursor to end
  ifstream myfile (fileName, ios::in|ios::binary|ios::ate);
  bool isFound = myfile.is_open();

  string statusLine = isFound ? "200 OK" : "404 Not Found";

  // Initial header
  string responseHeader = "HTTP/1.1 "+ statusLine + "\r\n"
                          "Connection: close\r\n"
                          "Date:" + getDatetime() + "\r\n"
                          "Server: Nico Server\r\n";

  // File does not exist
  if (!isFound) {
    send(connection, responseHeader.c_str(), responseHeader.size(), 0);
    return;
  }

  // Get Last-Modified time
  struct stat result;
  stat(fileName.c_str(), & result);
  time_t rawMtime = result.st_mtime;

  streampos size = myfile.tellg(); //cursor is at EOF

  responseHeader += "Last-Modified: " + getDatetime(rawMtime) + "\r\n"
                    "Content-Length: " + to_string(size) + "\r\n"
                    "Content-Type: " + getContentType(fileName) + "\r\n\r\n";

  char * memblock = new char[size];
  myfile.seekg(0, ios::beg); // reset cursor to beginning
  myfile.read(memblock, size); // read and store data in memblock

  // Combine header and response body
  string finalResponseString = responseHeader + memblock;
  const char * finalResponse = finalResponseString.c_str();

  // send response
  send(connection, finalResponse, strlen(finalResponse), 0);

  myfile.close();
  delete[] memblock;
}

string getFileName(char * buffer) {
  return "fileName";
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

  // Bind port 8080 to socket, use std::bind() 
  if (::bind(sockfd, (struct sockaddr*) &sockaddr, sizeof(sockaddr)) < 0) {
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
