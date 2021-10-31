#ifndef HTTP_REQUEST
#define HTTP_REQUEST
#include <unordered_map>
#include <vector>
using namespace std;

class HttpRequest {
private:
  /* data */
public:
  HttpRequest(/* args */) = default;
  ~HttpRequest() = default;
  virtual unordered_map<string, string> GetHeaders() = 0;
  virtual int GetContent(vector<uint8_t> &data) = 0;
};
#endif