#include "../reactor.h"
#include "../base_header.h"
//#include "../global_var.h"
#include "../message_processor.h"
#include "../asyn_operation.h"
#include "../bytebuffer/bytebuffer.h"
#include "../asyn_operation.h"
#include "../connection_manager.h"
#include <signal.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <log4cplus/configurator.h>
using namespace hpf;

int g_total_conn = 0;
int g_connectino_num = 0;
int g_cmd_recv = 0;
int g_cmd_send = 0;
int g_send_failed = 0;
pthread_mutex_t g_conn_lock;
pthread_mutex_t g_recv_lock;
pthread_mutex_t g_send_lock;

const int CMDLEN = 2048;

class RespAsynOperation : public AsynOperation
{
public:
    RespAsynOperation (ByteBuffer &bf) :
        AsynOperation(bf)
    {
    }
    virtual ~RespAsynOperation() 
    {
        delete [] m_byte_buffer.address();
    }
    virtual void on_operation_complete(sint32 result, sint32 reason)
    {
        if (result != 0)
        {
            ++g_send_failed;
        }
        else
        {
            pthread_mutex_lock(&g_send_lock);
            ++g_cmd_send;
            pthread_mutex_unlock(&g_send_lock);
        }
        delete this;
    }
};

class TestMessageProcessor : public MessageProcessor
{
    public:
        sint32 process_input(ByteBuffer *bb, sint32 handle)
        {
            while (bb->remaining() >= CMDLEN)
            {
                LOGGER_DEBUG(g_framework_logger, "process cmd. pthread_self = << " << pthread_self() << ", bufferlen = " << bb->remaining());
                // you need move the buffer cursor your self
                char *buf = new char[CMDLEN];
                ByteBuffer buff(buf, CMDLEN, CMDLEN);
                memcpy(buf, bb->address(), CMDLEN);
                bb->position(bb->position() + CMDLEN);
                RespAsynOperation *oper = new RespAsynOperation(buff);
                pthread_mutex_lock(&g_recv_lock);
                ++g_cmd_recv;
                pthread_mutex_unlock(&g_recv_lock);
                if (ConnectionManager::get_instance()->send_data(handle, oper) < 0)
                {
                    return -1;
                }
            }
            return 0;
        }

        sint32 connect(sint32 handle, const string &ip, sint16 port)
        {
            LOGGER_DEBUG(g_framework_logger, "connect get a handle=" << handle);
            pthread_mutex_lock(&g_conn_lock);
            ++g_connectino_num;
            ++g_total_conn;
            pthread_mutex_unlock(&g_conn_lock);
            return 0;
        }

        sint32 disconnect(sint32 handle)
        {
            LOGGER_DEBUG(g_framework_logger, "disconnect handle=" << handle);
            pthread_mutex_lock(&g_conn_lock);
            --g_connectino_num;
            pthread_mutex_unlock(&g_conn_lock);
            return 0;
        }
};
TestMessageProcessor tp;

void start_listen()
{
    Reactor::get_instance()->add_hcc_listener("0.0.0.0", 8088, &tp, 20);
}

void print()
{
    pthread_mutex_lock(&g_recv_lock);
    pthread_mutex_lock(&g_send_lock);
    printf("connection total:%d\n", g_total_conn);
    printf("connection num:%d\n", g_connectino_num);
    printf("cmd recv:%d\n", g_cmd_recv);
    printf("cmd send:%d\n", g_cmd_send);
    printf("cmd send failed:%d\n\n", g_send_failed);
    pthread_mutex_unlock(&g_send_lock);
    pthread_mutex_unlock(&g_recv_lock);
}

int main(int argc, char **argv)
{

    if (argc != 2)
    {
        fprintf(stderr, "usage: %s thread-number\n", argv[0]);
        return -1;
    }

    signal(SIGPIPE, SIG_IGN);

    struct rlimit limit;
    limit.rlim_cur = limit.rlim_max = 100;
    if(setrlimit(RLIMIT_NOFILE, &limit) < 0)
    {
        fprintf(stderr,"set rlimit RLIMIT_NOFILE failed!");
        return -1;
    }

    //LOG_INIT_EX("../conf/server_log4cplus.conf");
    //log4cplus::PropertyConfigurator::doConfigure("/home/yajie/svn/download_server/utility/cc_utility/hpf/conf/server_log4cplus.conf");
    log4cplus::PropertyConfigurator::doConfigure("../conf/server_log4cplus.conf");
    pthread_mutex_init(&g_conn_lock, NULL);
    pthread_mutex_init(&g_recv_lock, NULL);
    pthread_mutex_init(&g_send_lock, NULL);

    Reactor *reactor = Reactor::get_instance();
    reactor->init(atoi(argv[1]), 60);

    printf("test\n");
    LOGGER_DEBUG(g_framework_logger, "before start listen");
    start_listen();

    sint32 count = 0;
    while (count < 10000)
    {
        sleep(10);
        count++;
        print();
    }

    reactor->destroy();
    return 0;
}
