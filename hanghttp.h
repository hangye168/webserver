#ifndef _HANG_HTTP_H_
#define _HANG_HTPP_H_

#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/epoll.h>
#include <strings.h>
#include <string>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <utility>
#include <fstream>
#include <sstream>
#include <map>
#include <iostream>
#include <string.h>
#include <pthread.h>
#include <netinet/tcp.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/sendfile.h>

using namespace std;
#include "utility.h"
#include "parse.h"

typedef struct _epollfd_connfd
{
    int epollfd;
    int connfd;
}_epollfd_connfd;

//定义常数
#define MAX_EVENTS  1024    //epoll最大监听事件数
#define MAX_BACKLOG 100     //监听队列最大数

//保存配置信息
string hghp_domain("");
string hghp_docroot(""); 

//http状态码
#define hghp_CONTINUE       100 //收到了请求的起始部分，客户端应该继续请求

#define hghp_OK             200 //服务器已经成功处理请求
#define hghp_ACCEPTED       202 //请求已接受，服务器尚未处理

#define hghp_MOVED          301 //请求的URL已移走，响应应该包含Location URL
#define hghp_FOUND          302 //请求的URL临时移走，响应应该包含Location URL
#define hghp_SEEOTHER       303 //告诉客户端应该用另一个URL获取资源，响应应该包含Location URL
#define hghp_NOTMODIFIED    304 //资源未发生变化

#define hghp_BADREQUEST     400 //客户端发送了一条异常请求
#define hghp_FORBIDDEN      403 //服务器拒绝请求
#define hghp_NOTFOUND       404 //URL未找到

#define hghp_ERROR          500 //服务器出错
#define hghp_NOIMPLEMENTED  501 //服务器不支持当前请求所需要的某个功能
#define hghp_BADGATEWAY     502 //作为代理或网关使用的服务器遇到了来自响应链中上游的无效响应
#define hghp_SRVUNAVAILABLE 503 //服务器目前无法提供请求服务，过一段时间后可以恢复

char hghp_ok[] =            "OK";
char hghp_badrequest[] =    "Bad Request";
char hghp_forbidden[] =     "Forbidden";
char hghp_notfound[] =      "Not Found";
char hghp_noimplemented[] =     "No Implemented";

char *hghp_get_state_by_codes(int http_codes);//返回http状态码

//http响应的首部
#define hghp_ACCEPTRANGE_HEAD           "Accpet-Range"
#define hghp_AGE_HEAD                   "Age"
#define hghp_ALLOW_HEAD                 "Allow"
#define hghp_CONTENTBASE_HEAD           "Content-Base"
#define hghp_CONTENTLENGTH_HEAD         "Content-Length"
#define hghp_CONTENTLOCATION_HEAD       "Content-Location"
#define hghp_CONTENTRANGE_HEAD          "Content-Range"
#define hghp_CONTENTTYPE_HEAD           "Content-Type"
#define hghp_DATE_HEAD                  "Date"
#define hghp_EXPIRES_HEAD               "Expires"
#define hghp_LAST_MODIFIED_HEAD         "Last-Modified"
#define hghp_LOCATION_HEAD              "Location"
#define hghp_PUBLIC_HEAD                "Public"
#define hghp_RANGE_HEAD                 "Range"
#define hghp_SERVER_HEAD                "Server"

#endif
