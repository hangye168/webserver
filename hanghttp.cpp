#include "hanghttp.h"

#define ONEKILO     1024
#define ONEMEGA     1024*ONEKILO
#define ONEGIGA     1024*ONEMEGA
#define TIMEOUT 1000*60*4

void* hghp_thread_func(void *param);//记录当前处理线程的数量
int32_t hghp_thread_num = 0;
pthread_mutex_t hghp_thread_num_mutex = PTHREAD_MUTEX_INITIALIZER;//静态初始化互斥锁

void hghp_thread_num_add1();
void hghp_thread_num_minus1();

int hghp_do_http_header(hghp_http_header_t *phttphdr, string& out);

char *hghp_get_state_by_codes(int http_codes);



int main(int argc, char const *argv[])
{
    int 				listen_fd;
	int 				conn_sock; 
	int 				nfds;
	int 				epollfd;
	uint16_t 			listen_port;
	struct servent 		*pservent;
	struct epoll_event 	ev;
	struct epoll_event	events[MAX_EVENTS];
	struct sockaddr_in 	server_addr;
	struct sockaddr_in	client_addr;
	socklen_t 			addrlen;
	pthread_attr_t		pthread_attr_detach;
	_epollfd_connfd 	epollfd_connfd;
	pthread_t 			tid;

    if(argc != 2)
    {
        printf("usage:%s <config_path>\n",argv[0]);
        exit(-1);
    }

    if(hghp_is_file_existed(argv[1]) == -1)
    {
        perror("hghp_is_file_existed error");
        exit(-1);
    }

    if(hghp_parse_config(argv[1]) == -1)
    {
        perror("hghp_parse_config error");
        exit(-1);
    }

    listen_fd = hghp_socket(AF_INET,SOCK_STREAM,0);//创建监听套接字
    hghp_set_nonblocking(listen_fd);//设置监听套接字为非阻塞模式
    hghp_set_reuse_addr(listen_fd);//设置端口复用
    pservent = hghp_getservbyname("http","tcp");//根据服务器名  设置端口
    listen_port = pservent->s_port;

    bzero(&server_addr,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = listen_port;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    hghp_bind(listen_fd,(struct sockaddr*)&server_addr,sizeof(server_addr));//绑定
    hghp_listen(listen_fd,MAX_BACKLOG);

    epollfd = hghp_epoll_create(MAX_EVENTS);//创建epoll文件描述符

    ev.events = EPOLLIN;//可读事件
    ev.data.fd = listen_fd;
    hghp_epoll_ctl(epollfd,EPOLL_CTL_ADD,listen_fd,&ev);//注册事件

    for(;;)
    {
        nfds = hghp_epoll_wait(epollfd,events,MAX_EVENTS,-1);
        if(nfds == -1 && errno == EINTR)
            continue;

        for(int n = 0;n!=nfds;n++)
        {
            if(events[n].data.fd == listen_fd)
            {
                conn_sock = hghp_accept(listen_fd,(struct sockaddr*)&client_addr,&addrlen);
                hghp_set_nonblocking(conn_sock);

                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = conn_sock;

                hghp_epoll_ctl(epollfd,EPOLL_CTL_ADD,conn_sock,&ev);
            }
            else
            {
                epollfd_connfd.epollfd = epollfd;
                epollfd_connfd.connfd = events[n].data.fd;
                ev.data.fd = conn_sock;
                
                hghp_epoll_ctl(epollfd,EPOLL_CTL_DEL,conn_sock,&ev);

                pthread_create(&tid,&pthread_attr_detach,&hghp_thread_func,(void*)&epollfd_connfd);
            }
        }
    }
    pthread_attr_destroy(&pthread_attr_detach);
    close(listen_fd);
    return 0;
}

//处理客户端的线程
void* hghp_thread_func(void *param)
{
    hghp_http_header_t *phttphdr = hghp_alloc_http_header();

    _epollfd_connfd *ptr_epollfd_connfd = (_epollfd_connfd*)param;

    int conn_sock = ptr_epollfd_connfd->connfd;//取出socket
    
    struct epoll_event ev,events[2];

    ev.events = EPOLLIN | EPOLLET;//设置epoll事件 和触发方式
    ev.data.fd = conn_sock;
    int epollfd = hghp_epoll_create(2);//创建监听的数目
    hghp_epoll_ctl(epollfd,EPOLL_CTL_ADD,ev.data.fd,&ev);

    int nfds = 0;
    pthread_t tid = pthread_self();//获得当前线程的ID
    printf("NO.%u thread runs now\n",(unsigned int)tid);

    char *buff = (char*)hghp_malloc(ONEMEGA);//缓冲区
    bzero(buff,ONEMEGA);//初始化

    hghp_set_off_tcp_nagle(conn_sock);//关闭nagle
    hghp_set_recv_timeo(conn_sock,30,0);//设置接收超时时间30秒
begin:
    int32_t nread = 0,n = 0;
    for(;;)
    {
        if((n = read(conn_sock,buff+nread,ONEMEGA - 1)) > 0)//将接收到数据传递到缓冲区中
            nread += n;
        else if(n == 0)//没有接收到数据
            break;
        else if(n == -1 && errno == EINTR)//信号中断
            continue;
        else if(n == -1 && errno == EAGAIN)//文件的某部分被其他进程锁定
            break;
        else if(n == -1 && errno == EWOULDBLOCK)//超时
        {
            perror("socket read timeout");
            goto out;
        }
        else
        {
            perror("read http request error");
            hghp_free(buff);//释放缓冲区内存
            break;
        }
    }

    if(nread != 0)
    {
        string str_http_request(buff,buff + nread);//把buff中nread之后的复制到str_http_request中
        if(hghp_parse_http_request(str_http_request,phttphdr));
        {
            perror("http request failed");
            goto out;
        }
        cout<<"解析出来的http请求包："<<endl;
        hghp_print_http_header(phttphdr);

        string out;
        int http_codes = hghp_do_http_header(phttphdr,out);

        cout<<"http响应包："<<endl<<out<<endl;

        char *out_buf = (char *)hghp_malloc(out.size());
        if(out_buf == NULL)
            goto out;
        int i = 0;
        for(i = 0;i != out.size();i++)
            out_buf[i] = out[i];
        out_buf[i] = '\0';
        int nwrite = 0,n = 0;
        if( http_codes == hghp_BADREQUEST || http_codes == hghp_NOIMPLEMENTED || http_codes == hghp_NOTFOUND
            || (http_codes == hghp_OK && phttphdr->method == "HEAD"))
        {
            while((n == write(conn_sock,out_buf + nwrite,i)) != 0)//把out_buf中的所有数据写回到socket中
            {
                if(n == -1)
                {
                    if(errno == EINTR)
                        continue;
                    else
                        goto out;
                }
                nwrite += n;//循环写入
            }
        }

        if(http_codes == hghp_OK)
        {
            if(phttphdr->method == "GET")//get方式
            {
                while(n = write(conn_sock,out_buf + nwrite,i) != 0)
                {
                    cout<<n<<endl;
                    if(n == -1)
                    {
                        if(n == -1)
                        {
                            if(errno == EINTR)
                                continue;
                            else
                                goto out;
                        }
                        nwrite += n;
                    }
                }    
                string real_url = hghp_make_real_url(phttphdr->url);

                int fd = open(real_url.c_str(),O_RDONLY);//c_str() 将string中的数据装换为C语言字符串
                int file_size = hghp_get_file_length(real_url.c_str());
                cout<<"file size"<<file_size<<endl;
                int nwrite = 0;
                cout<<"sendfile:"<<real_url.c_str()<<endl;
            again:
                if((sendfile(conn_sock,fd,(off_t*)&nwrite,file_size)) < 0)//另拷贝
                    perror("sendfile error");

                if(nwrite < file_size)
                    goto again;
                cout<<"sendfile ok:"<<nwrite<<endl;
            }
        }
        free(out_buf);

        nfds = hghp_epoll_wait(epollfd,events,2,TIMEOUT);
        if(nfds == 0)
            goto out;
        for(int i = 0;i < nfds;i++)
        {
            if(events[i].data.fd == conn_sock)
                goto begin;
            else
                goto out;
        }
    }
out:
    hghp_free_http_header(phttphdr);
    close(conn_sock);
    printf("NO.%u thread ends now\n",(unsigned int)tid);
}


void hghp_thread_num_add1()//解锁
{
    pthread_mutex_lock(&hghp_thread_num_mutex);
    ++hghp_thread_num;
    pthread_mutex_unlock(&hghp_thread_num_mutex);
}

void hghp_thread_num_minus1()//加锁
{
    pthread_mutex_lock(&hghp_thread_num_mutex);
    --hghp_thread_num;
    pthread_mutex_unlock(&hghp_thread_num_mutex);
}

int hghp_do_http_header(hghp_http_header_t *phttphdr,string &out)
{
    char status_line[256] = {0};
    string crlf("\r\n");
    string server("Server:hanghttp\r\n");
    string Public("Public:GET,HEAD\r\n");
    string content_base = "Content-Base:" + hghp_domain + crlf;
    string date = "Date:" + hghp_time_gmt() + crlf;

    string content_length("Content-Length:");
    string content_location("Content-Location:");
    string last_modified("Last-Modified:");

    if(phttphdr == NULL)
    {
        snprintf(status_line,sizeof(status_line),"HTTP/1.1 %d %s\r\n",
                 hghp_BADREQUEST,hghp_get_state_by_codes(hghp_BADREQUEST));
        out = status_line + crlf;
        return hghp_BADREQUEST;
    }

    string method = phttphdr->method;
    string real_url = hghp_make_real_url(phttphdr->url);
    string version = phttphdr->version;
    if(method == "GET" || method == "HEAD")
    {
        if(hghp_is_file_existed(real_url.c_str()) == -1)
        {
            snprintf(status_line, sizeof(status_line), "HTTP/1.1 %d %s\r\n", 
                        hghp_NOTFOUND, hghp_get_state_by_codes(hghp_NOTFOUND));
            out += (status_line + server + date + crlf); 
            return hghp_NOTFOUND;//404
        }
        else
        {
            int len = hghp_get_file_length(real_url.c_str());
            snprintf(status_line, sizeof(status_line), "HTTP/1.1 %d %s\r\n", 
                        hghp_OK, hghp_get_state_by_codes(hghp_OK));
            out += status_line;
            snprintf(status_line, sizeof(status_line), "%d\r\n", len);
            out += content_length + status_line;
            out += server + content_base + date;
            out += last_modified + hghp_get_file_modified_time(real_url.c_str()) +crlf + crlf;
        }
    }
    else if(method == "PUT" || method == "DELETE" || method == "POST")
    {
        snprintf(status_line, sizeof(status_line), "HTTP/1.1 %d %s\r\n", 
                    hghp_NOIMPLEMENTED, hghp_get_state_by_codes(hghp_NOIMPLEMENTED));
        out += status_line + server + Public + date + crlf;
        return hghp_NOIMPLEMENTED;
    }
    else
    {

        snprintf(status_line, sizeof(status_line), "HTTP/1.1 %d %s\r\n", 
                    hghp_BADREQUEST, hghp_get_state_by_codes(hghp_BADREQUEST));
        out += status_line + crlf;
        return hghp_BADREQUEST;
    }
    return hghp_OK;
}

char *hghp_get_state_by_codes(int http_codes)
{
    switch(http_codes)
    {
    case hghp_OK:
        return hghp_ok;
    case hghp_BADREQUEST:
        return hghp_badrequest;
    case hghp_FORBIDDEN:
        return hghp_forbidden;
    case hghp_NOTFOUND:
        return hghp_notfound;
    case hghp_NOIMPLEMENTED:
        return hghp_noimplemented;
    default:
        break;
    }
    return NULL;
}






