#include "HttpRequest.h"

class RequestImpl : public HttpRequest {
private:
  /* data */
public:
  RequestImpl(/* args */);
  ~RequestImpl();
  unordered_map<string, string> GetHeaders();
  int GetContent(vector<uint8_t> &data);
};

RequestImpl::RequestImpl(/* args */) {}

RequestImpl::~RequestImpl() {}
unordered_map<string, string> RequestImpl::GetHeaders() {}
int RequestImpl::GetContent(vector<uint8_t> &data) {}
