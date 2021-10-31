#ifndef HTTP_PARSER
#define HTTP_PARSER
#include <algorithm>
#include <arpa/inet.h>
#include <atomic>
#include <cctype>
#include <chrono>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <poll.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <thread>
#include <unistd.h>
#include <unordered_map> 
#include <utility>
#include <vector>
using namespace std;
class EpollServer;
class HttpParser {
public:
  std::string buff;
  std::string body;
  int clientfd;
  const EpollServer *server;
  std::string method;
  std::string url;
  std::string protocal;
  std::string base_dir;
  map<string, string> headers;
  string Get(string key);
  HttpParser(const EpollServer *epollServer, int fd, string workdir);
  int Parse();
  void SendResp();

private:
  void parseHeader();
  void clear();
  int recvData();

  void parseRequestBasicContent(const string &s);
  void parseHeaderContent(vector<string> &headers);
  void readData();
  void printHeaders();
  void GetContentTypeFromSuffix();
};
#endif