#if 0
#include "http_request.h"
#include "job.h"
#include "http_response_callback.h"
namespace hpf
{

class Request : public MessageHandler
{
    public:
        char *url;
        char *result;
        int len;
        int maxlen;
        HttpResponseCallBack *cb;
        CURL *easy;
        Job *job;
        HttpRequest *hr;
        void *param;

        virtual void process_message(sint32 no, void *arg1, void *arg2)
        {
            HttpRequest *pthis = this->hr;
            assert(pthis);
            pthis->new_request(this);
        }

};

class HttpJob : public Job
{
    public:
        HttpJob(sint32 fd)
            : Job(fd)
        {}
        ~HttpJob()
        {}


        void *userp;

        sint32 run(sint32 fd, sint16 kind)
        {
            HttpRequest *pthis = reinterpret_cast<HttpRequest*>(userp);

            int action = (kind & EV_READ ? CURL_CSELECT_IN : 0) |
                (kind & EV_WRITE ? CURL_CSELECT_OUT : 0);
            CURLMcode rc = curl_multi_socket_action(pthis->m_multi, fd, action, &pthis->m_running);
            if (rc != CURLM_OK)
            {
                LOGGER_ERROR(g_framework_logger, "curl_multi_socket_action error");
                return -1;
            }

            pthis->check_multi_info();
            return 0;
        }

        // call this after delete from event_base
        virtual void delete_event_success() 
        {
//            delete this;
//            m_fd = 0;
        }
};

HttpRequest::HttpRequest()
{
    m_multi = NULL;
    m_running = 0;
}

HttpRequest::~HttpRequest()
{
    curl_multi_cleanup(m_multi);
}

// 线程初始化时调用
sint32 HttpRequest::self_init()
{

    m_multi = curl_multi_init();
    curl_multi_setopt(m_multi, CURLMOPT_SOCKETFUNCTION, curl_sock_cb);
    curl_multi_setopt(m_multi, CURLMOPT_SOCKETDATA, this);

    return true;
}

// 线程进入事件循环前调用
sint32 HttpRequest::begin_routine()
{
    LOGGER_INFO(g_framework_logger, "http request begin routine");
    return 0;
}

// 线程停止事件循环前调用
sint32 HttpRequest::end_routine()
{
    LOGGER_INFO(g_framework_logger, "http request end routine");
    return 0;
}



void HttpRequest::check_multi_info()
{
    CURLMsg *msg;
    int msgs_left;
    Request *req;
    CURL *easy;
    CURLcode res;

    LOGGER_DEBUG(g_framework_logger ,"running:" << m_running);
    while ((msg = curl_multi_info_read(m_multi, &msgs_left)) != NULL) 
    {
        if (msg->msg == CURLMSG_DONE) 
        {
            easy = msg->easy_handle;
            res = msg->data.result;
            curl_easy_getinfo(easy, CURLINFO_PRIVATE, &req);
            LOGGER_DEBUG(g_framework_logger, "DONE:" << req->url << ",code:" << res); 
            finish_request(res == CURLE_OK, req);
        }
    }
}

void HttpRequest::event_cb(int fd, short kind, void *userp)
{
    HttpRequest *pthis = reinterpret_cast<HttpRequest*>(userp);

    int action = (kind & EV_READ ? CURL_CSELECT_IN : 0) |
        (kind & EV_WRITE ? CURL_CSELECT_OUT : 0);
    CURLMcode rc = curl_multi_socket_action(pthis->m_multi, fd, action, &pthis->m_running);
    if (rc != CURLM_OK)
    {
        LOGGER_ERROR(g_framework_logger,"curl_multi_socket_action error");
        return;
    }

    pthis->check_multi_info();
}

int HttpRequest::curl_sock_cb(CURL *easy, curl_socket_t sock, int act, void *cbp, void *sockp)
{
    HttpRequest *pthis = reinterpret_cast<HttpRequest*>(cbp);
    const char *whatstr[] = {"none", "IN", "OUT", "INOUT", "REMOVE"};

    LOGGER_DEBUG(g_framework_logger, "socket callback: s=" << sock << ",e=" << easy << ",act:" << whatstr[act]);
    if (act == CURL_POLL_REMOVE) 
    {
        ;//do nothing
    }
    else 
    {
        Request *req = NULL;
        curl_easy_getinfo(easy, CURLINFO_PRIVATE, &req);
        if (req != NULL)
        {
            int kind = (act & CURL_POLL_IN? EV_READ : 0) | 
                (act & CURL_POLL_OUT? EV_WRITE : 0) | 
                EV_PERSIST;
            if (sockp != NULL)
            {
                pthis->delete_job(req->job);
            }
            else
            {
                curl_multi_assign(pthis->m_multi, sock, (void*)kind);
            }
            HttpJob *job = new HttpJob(0);
            job->userp = cbp;
            req->job = job;
            job->set_fd(sock);
            job->set_event_flags(kind);
            LOGGER_DEBUG(g_framework_logger, "test");
            pthis->add_new_job(job);
        }
        else 
        {
            LOGGER_ERROR(g_framework_logger, "easy private data null:" << easy);
        }
    }
    return 0;
}


void HttpRequest::finish_request(bool succ, Request *req)
{
    if (req->cb != NULL)
    {
        req->cb->response_callback(succ, req->result, req->len, req->param);
    }
    delete_job(req->job);
    free(req->url);
    if (req->easy != NULL)
    {
        curl_multi_remove_handle(m_multi, req->easy);
        curl_easy_cleanup(req->easy);
        req->easy = NULL;
    }
    delete req;
}

size_t HttpRequest::curl_write_cb(void *ptr, size_t size, size_t nmemb, void *data)
{
    Request *req = reinterpret_cast<Request*>(data);
    size_t realsize = size * nmemb;
    if (req->result != NULL && realsize <= (size_t)req->maxlen - req->len)
    {
        memcpy(req->result + req->len, ptr, realsize);
        req->len += realsize;
        return realsize;
    }
    return 0;
}

void HttpRequest::new_request(Request *req)
{
    req->easy = curl_easy_init();
    if (req->easy == NULL) 
    {
        LOGGER_ERROR(g_framework_logger, "curl_easy_init error");
        finish_request(false, req);
        return;
    }
    curl_easy_setopt(req->easy, CURLOPT_URL, req->url);
    curl_easy_setopt(req->easy, CURLOPT_WRITEFUNCTION, curl_write_cb);
    curl_easy_setopt(req->easy, CURLOPT_WRITEDATA, req);
    curl_easy_setopt(req->easy, CURLOPT_PRIVATE, req);
    LOGGER_DEBUG(g_framework_logger, "Adding easy " << req->easy << " to multi " << m_multi << " url:" << req->url);
    CURLMcode rc = curl_multi_add_handle(m_multi, req->easy);
    if (rc != CURLM_OK)
    {
        LOGGER_ERROR(g_framework_logger, "curl_multi_add_handle error");
        finish_request(false, req);
        return;
    }

    rc = curl_multi_socket_action(m_multi, 0, 0, &m_running);
    if (rc != CURLM_OK)
    {
        LOGGER_ERROR(g_framework_logger, "curl_multi_socket_action error");
        finish_request(false, req);
        return;
    }
}
    
bool HttpRequest::send_request(const char *url, char *ret_buff, int bufflen, HttpResponseCallBack *tcb, void *param)
{
    Request *req = new Request();
    req->url = strdup(url);
    req->result = ret_buff;
    req->len = 0;
    req->maxlen = bufflen;
    req->cb = tcb;
    req->hr = this;
    req->param = param;

    LOGGER_DEBUG(g_framework_logger, "send_request");
    this->post_message(req, NULL, NULL);
    return true;
}
    
}
#endif
