#include <iostream>
#include <sys/socket.h>
#include <cstdlib>
#include <unistd.h>
#include <netinet/in.h>

#include <string>
#include <fstream> // read file
#include <filesystem> // get size of file
#include <time.h> // time_t, time, tm
#include <algorithm> // convert to lowercase
#include <cctype> 
#include <cstring> // strlen
#include <signal.h>
// #include <stdlib.h>
// #include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>


using namespace std;

#define MAX_CONNS 10
#define BUF_SIZE 2048
#define PORT_NUM 8080
int sockfd, connection;

void sig_callback_handler(int signum) {
  if (signum == SIGINT) {
    close(connection);
    close(sockfd);
    cout << "Caught signal " << signum << endl;
    exit(signum);
  }
}

string toLower(string word) {
  transform(word.begin(), word.end(), word.begin(),
    [](unsigned char c){ return tolower(c); });
  return word;
}

string getContentType(const string fileName) {
  string extension = fileName.substr(fileName.find_last_of('.') + 1);

  transform(extension.begin(), extension.end(), extension.begin(),
    [](unsigned char c){ return tolower(c); });

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

string checkOtherFiles(string fileName) {
  struct stat buffer; // check if file exists
  if (stat(fileName.c_str(), & buffer) == 0)
    return fileName;

  // If file doesn't exist, it could just be missing an extension
  fileName = toLower(fileName);

  string dir;
  int index = fileName.find_last_of('/');
  if (index > fileName.length()) // file located in '.' directory
    dir = ".";
  else dir = "./" + fileName.substr(0, index);
  cout << dir <<endl;
  // Iterate through directory to find file with missing extension
  for (const auto& entry : filesystem::directory_iterator(dir)) {
    string altFile = toLower(string(entry.path()));
    cout << altFile<<endl;
    // if both files start the same way...
    if (altFile.rfind("./"+fileName, 0) == 0) {
      // if the last '.' is after the filename
      if (altFile.find_last_of('.') == (fileName.length()+2)) {
        return altFile.substr(2);
      }
    }
  }
  return "";
}

void responseToClient (string fileName, const int connection) {
  struct stat buffer; // check if file exists
  bool isFound = (stat(fileName.c_str(), & buffer) == 0);


  // Open file in READ, preserve \r\n, set cursor to end 
  ifstream myfile (fileName, ios::in|ios::binary|ios::ate);
  streampos size = myfile.tellg(); //cursor position equals file length

  isFound = isFound && (size >= 0); // size cannot be negative

  string statusLine = isFound ? "200 OK" : "404 Not Found";

  // Initial header
  string responseHeader = "HTTP/1.1 "+ statusLine + "\r\n"
                          "Connection: close\r\n"
                          "Date:" + getDatetime() + "\r\n"
                          "Server: Nico Server\r\n";
  // File does not exist
  if (!isFound) {
    responseHeader += "\r\n";
    send(connection, responseHeader.c_str(), responseHeader.size(), 0);
    return;
  }

  responseHeader += "Last-Modified: " + getDatetime(buffer.st_mtime) + "\r\n"
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

string ReplaceAll(string str, const string& from, const string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
    return str;
}

string getFileName(string buffer) {
  // ignore everything but first line
  string line = buffer.substr(0,buffer.find_first_of("\r\n"));
  // file name surrounded by '/' and ' '
  int beg = line.find_first_of('/');
  int len = line.find_last_of(' ') - beg - 1;
  string fileName = line.substr(beg+1, len);
  // replace %20 with ' '
  fileName = ReplaceAll(fileName, "%20", " ");
  fileName = checkOtherFiles(fileName);
  return fileName;
}

int main()
{
  signal(SIGINT, sig_callback_handler);
  // Create socket
  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  const int opt = 1; // Allows socket to be reused for testing purposes
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
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
  while ((connection = accept(sockfd, (struct sockaddr*) &sockaddr, (socklen_t*) &addrlen)) > 0) {
    char buffer[BUF_SIZE];
    if (read(connection, buffer, BUF_SIZE) < 0) {
      cout << "Failed connection: " << errno << endl;
      close(connection);
      exit(EXIT_FAILURE);
    }
    string fileName = getFileName(string(buffer));
    responseToClient(fileName, connection);

    shutdown(connection, 0);
  }

  close(sockfd);

  return 0;
}
