/*
 * filename      : http_request.h
 * Copyright (c) 2003-2012 Xunlei Inc.
 * author        : fengyajie@xunlei.com
 * descriptor    :  
 * create time   : 2012-09-28 09:30
 * modify list   :
 */
#ifndef HTTP_REQUEST_H_
#define HTTP_REQUEST_H_
#if 0
#include "event_thread.h"
#include <curl/curl.h>
namespace hpf{

class HttpResponseCallBack;
class Request;
class HttpRequest : public EventThread
{
        public:
        HttpRequest();
        ~HttpRequest();
        // 线程初始化时调用
        virtual sint32 self_init(); 

        // 线程进入事件循环前调用
        virtual sint32 begin_routine();

        // 线程停止事件循环前调用
        virtual sint32 end_routine();
    


        bool send_request(const char *url, char *ret_buff, int bufflen, HttpResponseCallBack *tcb, void *param);
    private:
        

        bool init_curl();
        void new_request(Request *req);
        void finish_request(bool succ, Request *req);
        void check_multi_info();
        static void event_cb(int fd, short kind, void *userp);
        static size_t curl_write_cb(void *ptr, size_t size, size_t nmemb, void *data);
        static int curl_sock_cb(CURL *easy, curl_socket_t sock, int what, void *cbp, void *sockp);
        static void pipe_event(int fd, short event, void *arg);
    private:

        CURLM *m_multi;
        int m_running;

    friend class Request;
    friend class HttpJob;

    };
    

}

#endif
#endif
