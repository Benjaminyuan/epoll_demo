#ifndef EPOLL_SERVER
#define EPOLL_SERVER
#include "HttpParser.h"
#include "HttpServer.h"
#include "ThreadPool.hpp"
#include <fcntl.h>
#include <functional>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#define THREAD_POOL_SIZE 10
#define EPOLL_EVENT_SIZE 4096
#define MAX_FD 2048
#define MAX_BUFF_SIZE 2048
class EpollServer : public HttpServer {
public:
  EpollServer(int port, std::string addr);
  virtual ~EpollServer();
  virtual int Init();
  virtual int Listen();
  virtual int RegisterHandleFunc(
      std::string path,
      function<void(const HttpRequest &, const HttpResponse &)>);
  int EpollOpt(int opt, int listenfd, int event) const;
protected:
private:
  int epollfd;
  int listenfd;
  int thread_pool_num;
  std::string working_dir;
  std::map<int, HttpParser *> httpParsers;
  struct sockaddr_in bind_addr;
  epoll_event epoll_events[EPOLL_EVENT_SIZE];
};
#endif