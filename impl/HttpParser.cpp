#include "HttpParser.h"
#include "EpollServerImpl.h"
#define BUFF_SIZE 2048

using namespace std;
std::unordered_map<std::string, std::string> CONTENT = {
    {"jpg", "image/jpeg"},
    {"jpeg", "image/jpeg"},
    {"jfif", "image/jpeg"},
    {"gif", "image/gif"},
    {"png", "image/png"},
    {"webp", "image/webp"},
    {"css", "text/css"},
    {"html", "text/html; charset=UTF-8"},
    {"htm", "text/html; charset=UTF-8"},
    {"txt", "text/plain"},
    {"ico", "image/jpeg"},
    {"json", "application/json"},
    {"js", "application/javascript"},
    {"pdf", "application/pdf"},
    {"zip", "application/zip"},
    {"gzip", "application/gzip"},
    {"doc", "application/msword"},
    {"exe", "application/x-msdownload"},
    {"ppt", "application/vnd.ms-powerpoint"},
};
HttpParser::HttpParser(const EpollServer *epollServer, int fd, string workdir) {
  base_dir = workdir;
  clientfd = fd;
  server = epollServer;
  buff.resize(BUFF_SIZE);
}

int HttpParser::recvData() {
  return recv(clientfd, (void *)buff.c_str(), BUFF_SIZE, 0);
}
string HttpParser::Get(string key) { return headers[key]; }

void HttpParser::parseRequestBasicContent(const string &s) {
  vector<string> res;
  int last = 0;
  for (int i = 0; i < s.length(); i++) {
    if (s[i] == ' ') {
      res.push_back(s.substr(last, i - last));
      last = i + 1;
    }
  }
  res.push_back(s.substr(last, s.length() - last));
  if (res.size() != 3) {
    cout << "recv invalid request, parse line:" << s << "failed " << endl;
    throw logic_error("invalid htpp request");
  }
  method = res[0];
  url = res[1];
  protocal = res[2];
}
void HttpParser::parseHeaderContent(vector<string> &headerList) {
  for (int i = 0; i < headerList.size(); i++) {
    string raw = headerList[i];
    if (raw.length() == 1) {
      break;
    }
    for (int j = 0; raw.length(); j++) {
      if (raw[j] == ':') {
        string key = raw.substr(0, j);
        if (j + 1 >= raw.length()) {
          cout << " parse header failed, parse line" << raw << endl;
          throw logic_error("parse header failed");
        }
        string value = raw.substr(j + 2, raw.length() - j - 2);
        headers[key] = value;
        break;
      }
    }
  }
  cout << "parse headers success !! total " << headers.size() << " line"
       << endl;
}
int HttpParser::Parse() {
  if (this == nullptr) {
    return -1;
  }
  this->parseHeader();
}
void HttpParser::parseHeader() {
  // 读取数据
  int m = recvData();
  if (m == 0) {
    server->EpollOpt(EPOLL_CTL_DEL, clientfd, 0);
    std::cout << "client disconnected,clientfd:" << clientfd << std::endl;
    close(clientfd);
    return;
  } else if (m < 0) {
    if (errno != EWOULDBLOCK && errno != EINTR) {
      server->EpollOpt(EPOLL_CTL_DEL, clientfd, 0);
      std::cout << "client disconnected,clientfd:" << clientfd << std::endl;
      close(clientfd);
      return;
    }
  }
  vector<string> header;
  int i = 0;
  int lineStart = 0;
  string basicInfo;
  while (buff[i] && i < 2048) {
    if (buff[i] == '\n') {
      int size = i - lineStart;
      if (size == 1) {
        break;
      }
      string s;
      s.resize(size);
      memcpy((void *)s.c_str(), buff.c_str() + lineStart, size - 1);
      s[size - 1] = '\0';
      if (lineStart == 0) {
        basicInfo = s;
      } else {
        header.push_back(s);
      }
      lineStart = i + 1;
    }
    i++;
  }
  cout << "header line-1:" << basicInfo << endl;
  parseRequestBasicContent(basicInfo);
  parseHeaderContent(header);
  // printHeaders();
  // 读取静态内容
  readData();
}
void HttpParser::GetContentTypeFromSuffix() {
  // 不是 / 开头的是非法url,通过前面的文件判断可以排除了
  if (headers.count("content-type") != 0) {
    return;
  }
  for (int i = url.length() - 1; i >= 1; i--) {
    if (url[i] == '.') {
      std::cout << "\nget content-type from url,index: \n" << i << std::endl;
      std::string s = url.substr(i + 1, url.length() - i - 1);
      std::for_each(s.begin(), s.end(), [](char &c) {
        c = ::tolower(static_cast<unsigned char>(c));
      });
      headers["content-type"] = s;
      std::cout << "\ncontent-type: " << headers["content-type"] << "\n"
                << std::endl;
      return;
    }
  }
}

void HttpParser::printHeaders() {
  std::cout << "-------- header content----------" << std::endl;
  auto iter = headers.begin();
  while (iter != headers.end()) {
    std::cout << "key: " << iter->first << "  value:" << iter->second
              << std::endl;
    iter++;
  }
  std::cout << "--------header content----------" << std::endl;
}
void HttpParser::readData() {
  string path;
  if (url == "/") {
    path = base_dir + "/index.html";
    headers["content-type"] = "html";
    std::cout << "--main---" << std::endl;
  } else {
    path = base_dir + url;
  }
  std::cout << "readData file_path:" << path << std::endl;
  ifstream in(path.c_str(), ios::in);
  struct stat s_buf;
  stat(path.c_str(), &s_buf);
  //文件不存在或者是文件夹
  if (!in || S_ISDIR(s_buf.st_mode)) {
    headers["content-type"] = "txt";
    std::cout << "file not exit " << std::endl;
  } else {
    GetContentTypeFromSuffix();
    in.seekg(0, in.end);
    int contentLength = in.tellg();
    in.seekg(0, in.beg);
    body.clear();
    body.resize(contentLength);
    in.read((char *)body.c_str(), contentLength);
    std::cout << "read data finish "
              << "content_length" << contentLength << std::endl;
    in.close();
  }
  // EpollOpt(EPOLL_CTL_MOD, fd, EPOLLOUT);
  //| EPOLLONESHOT;
  server->EpollOpt(EPOLL_CTL_MOD, clientfd, EPOLLOUT | EPOLLET);
  std::cout << "thread-" << std::this_thread::get_id() << ":"
            << "read finish" << std::endl;
}
void HttpParser::SendResp() {
  int sum = 0;
  char temp[100];
  // TODO: exec handle func
  std::string res = "";
  std::cout << "-----------------" << std::endl;
  std::cout << "url:" << url << " method: " << method
            << " protocal: " << protocal << std::endl;
  std::cout << "thread-" << std::this_thread::get_id() << ":" << std::endl;
  std::cout << "-------write return data-------" << std::endl;
  if (body.size() > 0) {
    //添加请求信息
    sprintf(temp, "%s %d %s\r\n", protocal.c_str(), 200, "OK");
    res.append(temp);
    memset(temp, 0, 100);
    // 添加请求头部
    cout << "raw content-type" << headers["content-type"] << endl;
    sprintf(temp, "%s: %s\r\n", "content-type",
            CONTENT[headers["content-type"]].c_str());
    res.append(temp);
    memset(temp, 0, 100);
    sprintf(temp, "%s: %d\r\n", "content-length", body.size());
    res.append(temp);
    memset(temp, 0, 100);

    //添加空行
    res.append("\r\n");
    //添加body
    std::cout << "header-length:" << res.length() << std::endl;
    std::cout << "header:" << res.substr(0, 100) << std::endl;
    send(clientfd, res.c_str(), res.size(), 0);
    send(clientfd, body.c_str(), body.size(), 0);
    std::cout << "---------write return data with body finish--------"
              << std::endl;
  } else {
    std::cout << "-------write return data-------" << std::endl;
    sprintf(temp, "%s %d %s\r\n", protocal.c_str(), 404, "Not Found");
    res.append(temp);
    res.append("\r\n");
    res.append("file you want not exist!!");
    std::cout << "---------write return data finish--------" << std::endl;
    sum = send(clientfd, res.c_str(), res.length(), 0);
  }
  server->EpollOpt(EPOLL_CTL_DEL, clientfd, 0);
  close(clientfd);
  std::cout << "send data to :" << clientfd << ", finished : " << sum
            << std::endl;
  clear();
}
void HttpParser::clear() {
  headers.erase(headers.begin(), headers.end());
  method = "";
  url = "";
  protocal = "";
  body.clear();
}