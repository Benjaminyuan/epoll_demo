#ifndef HTTP_SERVER
#define HTTP_SERVER
#include <HttpRequest.h>
#include <HttpResponse.h>
#include <arpa/inet.h>
#include <functional>
#include <string>
#define DEFAULT_PORT 3000
#define DEFAULT_THREAD_POOL_SIZE 12
#define MAXCONN 1024
#define BUFFSIZE 2048
#define WORK_DIR "./public"
class HttpServer {
protected:
  int port;
  uint32_t netAddr;

public:
  HttpServer(int port, std::string addr) : port(port) {
    this->netAddr = inet_addr(addr.c_str());
  }
  virtual ~HttpServer() = default;
  virtual int Listen() = 0;
  virtual int RegisterHandleFunc(
      std::string path,
      function<void(const HttpRequest &, const HttpResponse &)>) = 0;
  virtual int Init() = 0;
};

#endif