#include "HttpResponse.h"

class ResponseImpl : public HttpResponse {
private:
  /* data */
public:
  ResponseImpl(/* args */);
  ~ResponseImpl();
  void SetHeaders(unordered_map<string, string> &header);
  int SetContent(vector<uint8_t> &data);
};

ResponseImpl::ResponseImpl(/* args */) {}

ResponseImpl::~ResponseImpl() {}
void ResponseImpl::SetHeaders(unordered_map<string, string> &header){

}
int ResponseImpl::SetContent(vector<uint8_t> &data){}