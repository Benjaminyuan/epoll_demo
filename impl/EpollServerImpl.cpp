#include "EpollServerImpl.h"
// HttpServer::HttpServer() : HttpServer(DEFAULT_PORT)
// {
// }
int EpollServer::Init() {
  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  if (listenfd == -1) {
    std::cout << "create listen socket error" << std::endl;
  }
  int on = 1;
  //允许地址复用
  setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));
  setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));
  //设置socket 为非阻塞
  int oldSocketFlag = fcntl(listenfd, F_GETFL, 0);
  int newSocketFlag = oldSocketFlag | O_NONBLOCK;
  if (fcntl(listenfd, F_SETFL, newSocketFlag) == -1) {
    close(listenfd);
    std::cout << "fail to set listenfd unblock!!" << std::endl;
    return -1;
  }
  bind_addr.sin_family = AF_INET;
  bind_addr.sin_port = htons(port);
  // bind_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  bind_addr.sin_addr.s_addr = this->netAddr;

  if (bind(listenfd, (struct sockaddr *)&bind_addr, sizeof(bind_addr)) == -1) {
    close(listenfd);
    std::cout << "fail to bind listenfd!!" << std::endl;
    return -1;
  }
  std::cout << "bind port: " << port << " success" << std::endl;
  // 创建epoll 实例
  epollfd = epoll_create(1);
  if (epollfd == -1) {
    std::cout << "fail to create epollfd" << std::endl;
    return -1;
  }
  return 0;
}
EpollServer::EpollServer(int port, std::string addr) : HttpServer(port, addr) {
  thread_pool_num = THREAD_POOL_SIZE;
  working_dir = WORK_DIR;
}

int EpollServer::EpollOpt(int opt, int listenfd, int event) const {
  epoll_event *eventP = nullptr;
  if (event != 0) {
    epoll_event clientfd_event;
    clientfd_event.data.fd = listenfd;
    clientfd_event.events = event;
    eventP = &clientfd_event;
  }

  if (epoll_ctl(epollfd, opt, listenfd, eventP) == -1) {
    std::cout << "fail to modify epoll status" << std::endl;
    return -1;
  }
  return 0;
}
int EpollServer::Listen() {
  if (listen(listenfd, MAXCONN) == -1) {
    std::cout << "listen error" << std::endl;
    close(listenfd);
    return -1;
  }
  if (EpollOpt(EPOLL_CTL_ADD, listenfd, EPOLLIN) == -1) {
    std::cout << "fail to create epoll" << std::endl;
    close(listenfd);
    return -1;
  }
  std::cout << "init thread_pool, size: " << thread_pool_num << std::endl;
  ThreadPool thread_pool(thread_pool_num);
  thread_pool.start();
  while (true) {
    int n = epoll_wait(epollfd, epoll_events, 2048, 100);
    if (n < 0) {
      if (errno == EINTR) {
        continue;
      }
      break;
    } else if (n == 0) {
      continue;
    }

    for (int i = 0; i < n; i++) {
      // 可读
      if (epoll_events[i].events & EPOLLIN) {
        // 处理连接
        if (epoll_events[i].data.fd == listenfd) {
          cout << "listen fd happen event: " << std::hex << epoll_events[i].events << endl;
          struct sockaddr_in client_addr;
          socklen_t client_addr_len = sizeof(client_addr);
          int clientfd = accept(listenfd, (struct sockaddr *)&client_addr,
                                &client_addr_len);
          if (clientfd != -1) {
            int oldSocketFlag = fcntl(clientfd, F_SETFL, 0);
            int newSocketFlag = oldSocketFlag | O_NONBLOCK;
            if (fcntl(clientfd, F_SETFL, newSocketFlag) == -1) {
              close(clientfd);
              std::cout << "set clientfd to nonblocking failed." << std::endl;
              break;
            } else {
              if (EpollOpt(EPOLL_CTL_ADD, clientfd, EPOLLIN | EPOLLET) == 0) {
                std::cout << "new client accept,listenfd: " << clientfd
                          << std::endl;
              } else {
                std::cout << "add client failed " << std::endl;
                break;
              }
            }
          }
        } else {
          // 可读取数据
          int fd = epoll_events[i].data.fd;
          struct sockaddr_in sa;
          socklen_t len = sizeof(sa);
          getpeername(fd, (struct sockaddr *)&sa, &len);
          const char *ip = inet_ntoa(sa.sin_addr);
          int port = int(ntohs(sa.sin_port));
          std::cout << "------------------------" << std::endl;
          std::cout << "clientfd: " << fd << " EPOLLIN" << std::endl;
          std::cout << "client_ip:" << ip << "\nport:" << port << std::endl;
          std::cout << "------------------------\n" << std::endl;
          HttpParser *parser;
          if (httpParsers.count(fd) > 0) {
            const auto it = httpParsers.find(fd);
            parser = (it->second);
          } else {
            parser = new HttpParser(this, fd, working_dir);
          }
          // std::cout << "\n\n----------
          httpParsers.insert(std::make_pair(fd, parser));
          thread_pool.appendTask(std::bind(&HttpParser::Parse, parser));
          // threads.push_back(thread(&HttpParser::parseHeader,parser));
        }

      } else if ((epoll_events[i].events & EPOLLOUT) &&
                 epoll_events[i].data.fd != listenfd) {
        // 可以写
        int fd = epoll_events[i].data.fd;
        std::cout << "------------------------" << std::endl;
        std::cout << "clientfd: " << fd << " EPOLLOUT" << std::endl;
        std::map<int, HttpParser *>::iterator it = httpParsers.find(fd);
        if (it == httpParsers.end()) {
          std::cout << "fail to get parser" << std::endl;
        }
        HttpParser *parser = it->second;
        thread_pool.appendTask(std::bind(&HttpParser::SendResp, parser));
        // threads.push_back(std::thread(&HttpParser::sendRes,parser));
      }
    }
    // for(auto iter = threads.begin();iter != threads.end();iter++){
    //     iter->join();
    // }
  }
  thread_pool.stop();
  close(listenfd);
  return 0;
}
EpollServer::~EpollServer() {}

int EpollServer::RegisterHandleFunc(
    std::string path,
    function<void(const HttpRequest &, const HttpResponse &)>) {
      // TODO 
    }