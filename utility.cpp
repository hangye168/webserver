#include "utility.h"

map<string,int> hghp_config_keyword_map;

string hghp_time_gmt()
{
    time_t now;//时间戳
    struct tm *time_now;

    string str_time;

    time(&now);
    time_now = localtime(&now);

    switch(time_now->tm_wday)
    {
    case 0:
        str_time += "SUN, ";
        break;
    case 1:
        str_time += "MON, ";
        break;
    case 2:
        str_time += "TUE, ";
        break;
    case 3:
        str_time += "WEB, ";
        break;
    case 4:
        str_time += "THU, ";
        break;
    case 5:
        str_time += "FRI, ";
        break;
    case 6:
        str_time += "SAT, ";
        break;
    }
    char buf[16];
    snprintf(buf,sizeof(buf),"%d",time_now->tm_mday);//天
    str_time += string(buf);

    switch(time_now->tm_mon)
    {
    case 0:
        str_time += "Jan ";
        break;
    case 1:
        str_time += "Fed ";
        break;
    case 2:
        str_time += "Mar ";
        break;
    case 3:
        str_time += "Apr ";
        break;
    case 4:
        str_time += "May ";
        break;
    case 5:
        str_time += "Jun ";
        break;
    case 6:
        str_time += "Jul ";
        break;
    case 7:
        str_time += "Aug ";
        break;
    case 8:
        str_time += "Sep ";
        break;
    case 9:
        str_time += "Oct ";
        break;
    case 10:
        str_time += "Nov ";
        break;
    case 11:
        str_time += "Dec ";
        break;
    }
    snprintf(buf,sizeof(buf),"%d",time_now->tm_year + 1900);
    str_time += string(buf);
    snprintf(buf,sizeof(buf),"%d:%d:%d",time_now->tm_hour,time_now->tm_min,time_now->tm_sec);
    str_time += string(buf);

    str_time += "GMT";
    return str_time;
}

string hghp_make_real_url(const string &url)
{
    string real_url,url2;
    int n = 0;

    //从url中获取文件的真实地址，并与服务器根目录组合  获取文件的真实地址
    if((n = url.find(hghp_domain,0))!=string::npos)
        url2 = url.substr(hghp_domain.size(),url.size() - hghp_domain.size());//复制url中除域名以外的数据
    else
        url2 = url;

    if(hghp_docroot[hghp_docroot.size() - 1] == '/')
    {
        if(url2[0] == '/')
            real_url = hghp_docroot + url2.erase(0,1);
        else
            real_url = hghp_docroot + url2;
    }else
    {
        if(url2[0] == '/')
            real_url = hghp_docroot + url2;
        else
            real_url = hghp_docroot + '/' + url2;
    }
    return real_url;
}

int hghp_get_file_length(const char *path)
{
    struct stat buf;
    int ret = stat(path,&buf);//将文件的信息 存入结构体中
    if(ret == -1)
    {
        perror("get file length error");
        exit(-1);
    }
    return (int)buf.st_size;
}

string hghp_get_file_modified_time(const char *path)
{
    struct stat buf;//stat 这个结构体中存储着一个文件的所有信息
    int ret = stat(path,&buf);
    if(ret == -1)
    {
        perror("get file modified time error");
        exit(-1);
    }

    char array[32] = {0};
    snprintf(array,sizeof(array),"%s",ctime(&buf.st_mtime));//ctime函数是将时间和日期装换为字符串
    return string(array,array + strlen(array));
}

void hghp_init_config_keyword_map()
{
    hghp_config_keyword_map.insert(make_pair("docroot", hghp_DOCROOT));
    hghp_config_keyword_map.insert(make_pair("domain", hghp_DOMAIN));
}

int hghp_parse_config(const char *path)
{
    hghp_init_config_keyword_map();
    int ret = 0;
    fstream infile(path,fstream::in);
    string line,word;
    if(!infile)
    {
        printf("%s can`t open\n",path);
        infile.close();
        return -1;
    }

    while(getline(infile,line))
    {
        stringstream stream(line);
        stream >> word;
        map<string,int>::const_iterator cit = hghp_config_keyword_map.find(word);
        if(cit == hghp_config_keyword_map.end())
        {
            printf("can not find keyword\n");
            infile.close();
            return -1;
        }

        switch(cit->second)
        {
        case hghp_DOCROOT:
            stream >> hghp_docroot;
            break;
        case hghp_DOMAIN:
            stream >> hghp_domain;
            break;
        default:
            infile.close();
            return -1;
        }
    }
    infile.close();
    return 0;
}

void hghp_set_nonblocking(int fd)
{
    int flags = fcntl(fd,F_GETFL,0);//获取文件描述符的状态
    if(flags < 0)
    {
        perror("fcntl f_getfl error");
        exit(-1);
    }
    flags |= O_NONBLOCK;//设置为非阻塞IO
    int ret = fcntl(fd,F_SETFL,flags);
    if(ret < 0)
    {
        perror("fcntl setfl error");
        exit(-1);
    }
}


void hghp_set_reuse_addr(int sockfd)//开启端口复用
{
    int on = 1;
    int ret = setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
    if(ret == -1)
    {
        perror("setsockopt SO_REUSEADDR error");
        exit(-1);
    }
}

void hghp_set_off_tcp_nagle(int sockfd)
{
    int on = 1;
    int ret = setsockopt(sockfd,IPPROTO_TCP,TCP_NODELAY,&on,sizeof(on));
    if(ret == -1)
    {
        perror("setsockopt on IPPROTO_TCP TCP_NODELAY error");
        exit(-1);
    }
}


void hghp_set_on_tcp_nagle(int sockfd)
{
    int off = 1;
    int ret = setsockopt(sockfd,IPPROTO_TCP,TCP_NODELAY,&off,sizeof(off));
    if(ret == -1)
    {
        perror("setsockopt off IPPROTO_TCP TCP_NODELAY error");
        exit(-1);
    }
}


void hghp_set_on_tcp_cork(int sockfd)
{
    int on = 1;
    int ret = setsockopt(sockfd,SOL_TCP,TCP_CORK,&on,sizeof(on));
    if(ret == -1)
    {
        perror("setsockopt on SOL_TCP TCP_CORK error");
        exit(-1);
    }
}


void hghp_set_off_tcp_cork(int sockfd)
{
    int off = 0;
    int ret = setsockopt(sockfd,SOL_TCP,TCP_CORK,&off,sizeof(off));
    if(ret == -1)
    {
        perror("setsockopt off SOL_TCP TCP_CORK error");
        exit(-1);
    }
}

void hghp_set_recv_timeo(int sockfd,int sec,int usec)
{
    struct timeval time = {sec,usec};
    int ret = setsockopt(sockfd,SOL_SOCKET, SO_RCVTIMEO,&time,sizeof(time));//设置超时时间
    if(ret == -1)
    {
        perror("setsockopt set_recv_time error");
        exit(-1);
    }
}

void hghp_set_send_timeo(int sockfd,int sec,int usec)
{
    struct timeval time = {sec,usec};
    int ret = setsockopt(sockfd,SOL_SOCKET, SO_SNDTIMEO,&time,sizeof(time));//设置send超时时间
    if(ret == -1)
    {
        perror("setsockopt set_send_time error");
        exit(-1);
    }
}

//系统函数的重新封装
int hghp_socket(int domain,int type,int protocol)
{
    int listen_fd;
    if((listen_fd = socket(AF_INET,SOCK_STREAM,0)) == -1)
    {
        perror("socket error");
        exit(-1);
    }
    return listen_fd;
}

void hghp_listen(int sockfd,int backlog)
{
    if(listen(sockfd,backlog) == -1)
    {
        perror("listen error");
        exit(-1);
    }
}

void hghp_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    if(bind(sockfd,addr,addrlen) == -1)
    {
        perror("bind error");
        exit(-1);
    }
}

int hghp_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)//设置新链接上的套接字为非阻塞模式
{
    int ret_fd = 0;
    for(;;)
    {
        ret_fd = accept(sockfd,addr,addrlen);
        if(ret_fd > 0)
            break;
        else if(ret_fd == 01)
        {
            if(errno != EAGAIN && errno != EPROTO && errno != EINTR && errno != ECONNABORTED)
            {
                perror("accept error");
                exit(-1);
            }
        }
        else
            continue;
    }
    return ret_fd;
}

struct servent* hghp_getservbyname(const char *name, const char *proto)//通过协议设置端口
{
    struct servent *pservent;
    if((pservent = getservbyname(name,proto)) == NULL)
    {
        perror("getservbyname error");
        exit(-1);
    }
    return pservent;
}

int hghp_epoll_create(int size)
{
    int epollfd;
    epollfd = epoll_create(size);
    if(epollfd == -1)
    {
        perror("epoll_create error");
        exit(-1);
    }
    return epollfd;
}

void hghp_epoll_ctl(int epfd, int op, int fd, struct epoll_event *event)
{
    if(epoll_ctl(epfd,op,fd,event) == -1)
    {
        perror("epoll_ctl error");
        exit(-1);
    }
}

int hghp_epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout)
{
again:
    int nfds = epoll_wait(epfd,events,maxevents,timeout);//等待所监控文件描述符上有事件的产生
    if(nfds == -1)
    {
        if(errno != EINTR)
        {
            perror("epoll_wait error");
            exit(-1);
        }
        else
            goto again;
    }
    return nfds;
}

void *hghp_malloc(size_t size)
{
    void *ptr = malloc(size);
    if(NULL == ptr)
    {
        perror("malloc error");
        exit(-1);
    }
    return ptr;
}

void hghp_free(void *ptr)
{
    free(ptr);
}

