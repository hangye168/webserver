#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <map>
#include <utility>
#include <sstream>
#include <ctype.h>
#include <iostream>

using namespace std;

typedef map<string,string> hghp_header;

#define make_hghp_header(key,value) \
    make_pair((key),(value))

typedef struct _hghp_http_header_t
{
    string method;
    string url;
    string version;
    hghp_header header;
    string body;
}hghp_http_header_t;//保存从http_request解析下来的数组

void hghp_print_http_header_header(const hghp_header& head);//打印结构体中的header  由于header是map

void hghp_print_http_header(hghp_http_header_t *phttphdr);//打印hghp_http_header_t

hghp_http_header_t *hghp_alloc_http_header();//初始化hghp_http_header_t

void hghp_free_http_header(hghp_http_header_t *phttphdr);//释放hghp_http_header_t的空间

bool hghp_parse_http_request(const string& http_request, hghp_http_header_t *phttphdr);//解析http_request
