#ifndef HTTP_RESPONSE
#define HTTP_RESPONSE
#include <unordered_map>
#include <vector>
using namespace std;
class HttpResponse {
private:
  /* data */
public:
  HttpResponse() = default;
  ~HttpResponse() = default;
  virtual void SetHeaders(unordered_map<string, string> &header) = 0;
  virtual int SetContent(vector<uint8_t> &data) = 0;
};
#endif
