#include "parse.h"

void hghp_print_http_header_header(const hghp_header& head)//打印结构体中的header  由于header是map
{
    if(!head.empty())
    {
        hghp_header::const_iterator cit = head.begin();
        while(cit != head.end())
        {
            cout<<cit->first<<":"<<cit->second<<endl;
            ++cit;
        }
    }
}
void hghp_print_http_header(hghp_http_header_t *phttphdr)//打印hghp_http_header_t
{
    if(phttphdr == NULL)
    {
        perror("phttphdr is null");
        exit(-1);
    }
    cout<<phttphdr->method<<" "<<phttphdr->url<<" "<<phttphdr->version<<endl;
    hghp_print_http_header_header(phttphdr->header);
    cout<<endl<<phttphdr->body<<endl;
}
hghp_http_header_t *hghp_alloc_http_header()//初始化hghp_http_header_t
{
    hghp_http_header_t *phttphdr = (hghp_http_header_t *)new hghp_http_header_t;
    
    if(phttphdr == NULL)
    {
        perror("hghp_alloc_http_header error");
        exit(-1);
    }
    return phttphdr;
}
void hghp_free_http_header(hghp_http_header_t *phttphdr)//释放hghp_http_header_t的空间
{
    if(phttphdr == NULL)
        return;
    delete phttphdr;
    phttphdr = NULL;
}

bool hghp_parse_http_request(const string& http_request, hghp_http_header_t *phttphdr)//解析http_request
{
    if(http_request.empty())
    {
        perror("hghp_parse_http_request is empty");
        return false;
    }
    if(phttphdr == NULL)
    {
        perror("hghp_parse_http_request phttphdr is null");
        return false;
    }

    string crlf("\r\n"),crlfcrlf("\r\n\r\n");
    size_t prev = 0,next = 0;
    
    if((next = http_request.find(crlf,prev))!=string::npos)
    {
        string first_line(http_request.substr(prev,next - prev));
        prev = next;
        stringstream sstream(first_line);
        sstream >> (phttphdr->method);
        sstream >> (phttphdr->url);
        sstream >> (phttphdr->version);
    }
    else
    {
        perror("hghp_parse_http_request http_request has not a \\r\\n");
        return false;
    }

    size_t pos_crlfcrlf = http_request.find(crlfcrlf, prev);//查找"\r\n\r\n\""
    if(pos_crlfcrlf == string::npos)
    {
        perror("hghp_parse_http_request http_request has not a \"\r\n\r\n\"");
        return false;
    }

    string buff,key,value;//解析首部行
    while(1)
    {
        next = http_request.find(crlf,prev + 2);
        if(next <= pos_crlfcrlf)
        {
            buff = http_request.substr(prev + 2,next - prev - 2);
            size_t end = 0;
            for(;isblank(buff[end]);++end);//消除开头的空格

            int beg = end;
            for(;buff[end]!=':' && !isblank(buff[end]);++end);

            key = buff.substr(beg,end - beg);

            for(;(!isalpha(buff[end]) && !isdigit(buff[end]));++end);

            beg = end;
            for(;next!=end;++end);
            value = buff.substr(beg,end - beg);
            phttphdr->header.insert(make_hghp_header(key,value));

            prev = next;
        }
        else
        {
            break;
        }
    }
    //获取其他http请求的值
    phttphdr->body = http_request.substr(pos_crlfcrlf +4,http_request.size() - pos_crlfcrlf - 4);
    cout<<http_request<<endl;
    return true;
}
