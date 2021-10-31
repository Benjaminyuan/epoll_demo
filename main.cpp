#include "HttpServer.h"
#include "EpollServerImpl.h"
using namespace std;
int main(int argsc, char *argv[]) {
  HttpServer* server = new EpollServer(8999,"0.0.0.0");
  server->Init();
  server->Listen();
}