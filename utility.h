#ifndef _UTILITY_H_
#define _UTILITY_H

#include<sys/socket.h>
#include<sys/types.h>
#include<stdlib.h>
#include<stdio.h>
#include<netdb.h>//主要定义了与网络有关的结构、变量类型、宏、函数
#include<sys/epoll.h>
#include<strings.h>
#include<string>
#include<errno.h>
#include<unistd.h>
#include<fcntl.h>
#include<utility>
#include<fstream>
#include<sstream>
#include<map>
#include<iostream>
#include<string.h>
#include<pthread.h>
#include<netinet/tcp.h>
#include<time.h>
#include<sys/stat.h>

using namespace std;

extern string hghp_docroot;
#define hghp_DOCROOT 1
extern string hghp_domain;
#define hghp_DOMAIN 2


//获取系统时间
string hghp_time_gmt();
//根据http包中url和配置文件中的dcroot配置选项构造真正的url
string hghp_make_real_url(const string& url);
//测试文件是否存在
inline int hghp_is_file_existed(const char *path)
{
    int ret = open(path,O_RDONLY|O_EXCL);
    close(ret);
    return ret;
}
//获取文件的长度
int hghp_get_file_length(const char *path);
//获取文件的最后修改时间
 string hghp_get_file_modified_time(const char *path);
//初始化全局变量hghp_config_keyword_map，必须在使用hghp_config_keyword_map前调用此函数
 void hghp_init_config_keyword_map();
//解析配置文件
 int hghp_parse_config(const char *path);
//设置文件描述符为非阻塞模式
 void hghp_set_nonblocking(int fd);
//设置套接字SO_REUSEADDR选项
 void hghp_set_reuse_addr(int sockfd);
//开启套接字TCP_NODELAY选项，关闭nagle算法
 void hghp_set_off_tcp_nagle(int sockfd);
//关闭套接字TCP_NODELAY选项，开启nagle算法
 void hghp_set_on_tcp_nagle(int sockfd);
//开启套接字TCP_CORK选项
 void hghp_set_on_tcp_cork(int sockfd);
//关闭套接字TCP_CORK选项
 void hghp_set_off_tcp_cork(int sockfd);
//设置套接字SO_RCVTIMEO选项，接收超时
 void hghp_set_recv_timeo(int sockfd, int sec, int usec);
//设置套接字SO_SNDTIMEO选项，发送超时
 void hghp_set_snd_timeo(int sockfd, int sec, int usec);


//系统函数的重新封装
int hghp_socket(int domain, int type, int protocol);
void hghp_listen(int sockfd, int backlog);
void hghp_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int hghp_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
struct servent* hghp_getservbyname(const char *name, const char *proto);
int hghp_epoll_create(int size);
void hghp_epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
int hghp_epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);

void *hghp_malloc(size_t size);
void hghp_free(void *ptr);

#endif
