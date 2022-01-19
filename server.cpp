#include <iostream>
#include <string>
#include <vector>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <time.h>
#include <cstdlib>

using namespace std;

#define NUM_CON 10
#define BUF_SIZE 4096
#define PORT_NUM 8080

int main()
{
  // Create socket
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd == -1) {
    cout << "Failed to create socket: " << errno << endl;
    exit(EXIT_FAILURE);
  }

  // Bind port 8080
  sockaddr_in sockaddr;
  sockaddr.sin_family = AF_INET;
  sockaddr.sin_addr.s_addr = INADDR_ANY;
  sockaddr.sin_port = htons(PORT_NUM);

  if (bind(sockfd, (struct sockaddr*) &sockaddr, sizeof(sockaddr)) < 0) {
    cout << "Failed to bind to port 8080: " << errno << endl;
    exit(EXIT_FAILURE);
  }

  // Listen on socket. Hold at most 10 connections in the queue
  if (listen(sockfd, NUM_CON) < 0) {
    cout << "Failed to listen on socket: " << errno << endl;
    exit(EXIT_FAILURE);
  }

  auto addrlen = sizeof(sockaddr);

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
