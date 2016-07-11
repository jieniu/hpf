#include "../reactor.h"
#include "../global_var.h"
#include "../message_processor.h"
#include "../asyn_operation.h"
#include "../connection_manager.h"
#include "../hcc_sender.h"
#include "../SDSocketUtility.h" 
#include <signal.h>
#include <sys/time.h>
#include <sys/resource.h>
using namespace hpf;

//const char* g_ip = "10.1.3.166";
const char* g_ip = "10.1.1.120";
int g_conn_per_thread = 0;
int g_cmd_per_conn = 0;
int g_cmd_send = 0;
int g_cmd_recv = 0;
int g_send_failed = 0;
int g_conn_fail = 0;
int g_conn_num = 0;
int g_conn_send = 0;
bool g_reconnect = false;
pthread_mutex_t g_send_lock;
pthread_mutex_t g_recv_lock;
pthread_mutex_t g_conn_lock;
const int CMDLEN = 2048;

class TestOperation : public AsynOperation
{
    public:   
        TestOperation(ByteBuffer &bb)
            : AsynOperation(bb)
        {}
        ~TestOperation()
        {
            delete [] m_byte_buffer.address();
        }

        void on_operation_complete(sint32 result, sint32 reason) 
        {
            LOGGER_DEBUG(g_framework_logger, "write operation result=" << result
                    << ", reason=" << reason << ", send_length=" << m_byte_buffer.capacity());
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
    TestMessageProcessor()
    {
        m_cmd_send = 0;
    }
    sint32 process_input(ByteBuffer *bb, sint32 handle)
    {
        LOGGER_DEBUG(g_framework_logger, "process input.");
        while (bb->remaining() >= CMDLEN)
        {
            pthread_mutex_lock(&g_recv_lock);
            ++g_cmd_recv;
            pthread_mutex_unlock(&g_recv_lock);
            bb->position(bb->position() + CMDLEN);
        }
        return send_cmd(handle);
    }

    sint32 connect(sint32 handle, const string &ip, uint16 port)
    {
        LOGGER_DEBUG(g_framework_logger, "connect success, get a handle=" << handle);
        m_cmd_send = 0;
        pthread_mutex_lock(&g_conn_lock);
        ++g_conn_num;
        pthread_mutex_unlock(&g_conn_lock);
        return send_cmd(handle); 
    }

    sint32 send_cmd(sint32 handle)
    {
        if (m_cmd_send >= g_cmd_per_conn)
        {
            return -1;
        }
        ++m_cmd_send;
        char *buffer = new char[CMDLEN];
        memset(buffer, 0, CMDLEN);
        ByteBuffer bb(buffer, CMDLEN, CMDLEN);
        TestOperation *to = new TestOperation(bb);
        return ConnectionManager::get_instance()->send_data(handle, to);
    }
    sint32 disconnect(sint32 handle)
    {
        LOGGER_DEBUG(g_framework_logger, "disconnect handle=" << handle);
        pthread_mutex_lock(&g_conn_lock);
        --g_conn_num;
        --g_conn_send;
        pthread_mutex_unlock(&g_conn_lock);
        delete this;
        return 0;
    }

    int m_cmd_send;
};

void *start_routine(void *arg)
{
    int i = 0;
    for (; i < g_conn_per_thread; i++)
    {
        if (g_conn_num > 8000)
        {
//            --i;
//            continue;
        }
        TestMessageProcessor *tm = new TestMessageProcessor;
        int ret = Reactor::get_instance()->add_connector(g_ip, 8088, tm, g_reconnect);
        if (ret < 0)
        {
            pthread_mutex_lock(&g_conn_lock);
            ++g_conn_fail;
            pthread_mutex_unlock(&g_conn_lock);
            delete tm;
        }
        else
        {
            pthread_mutex_lock(&g_conn_lock);
            ++g_conn_send;
            pthread_mutex_unlock(&g_conn_lock);
        }
        if ((i % 100) == 0)
        {
//            sleep(1);
        }
    }
    printf("thread end:%d\n", i);
    return 0;
} 

void print()
{
    pthread_mutex_lock(&g_send_lock);
    pthread_mutex_lock(&g_recv_lock);
    printf("cmd send:%d\n", g_cmd_send);
    printf("cmd recv:%d\n", g_cmd_recv);
    printf("cmd send failed:%d\n", g_send_failed);
    printf("conn num:%d\n", g_conn_num);
    printf("conn send:%d\n", g_conn_send);
    printf("conn failed:%d\n\n", g_conn_fail);
    pthread_mutex_unlock(&g_recv_lock);
    pthread_mutex_unlock(&g_send_lock);
}

int main(int argc, const char* argv[])
{
    struct rlimit limit;
    limit.rlim_cur = limit.rlim_max = 1000000;
    if(setrlimit(RLIMIT_NOFILE, &limit) < 0)
    {   
        fprintf(stderr,"set rlimit RLIMIT_NOFILE failed!");
        return -1; 
    }
    signal(SIGPIPE, SIG_IGN);
    LOG_INIT_EX("../conf/client_log4cplus.conf");

    pthread_mutex_init(&g_recv_lock, NULL);
    pthread_mutex_init(&g_send_lock, NULL);
    pthread_mutex_init(&g_conn_lock, NULL);
    Reactor *reactor = Reactor::get_instance();
    reactor->init(atoi(argv[4]), 60);

    int threads = atoi(argv[1]);
    g_conn_per_thread = atoi(argv[2]);
    g_cmd_per_conn = atoi(argv[3]);
    g_reconnect = (atoi(argv[5]) == 1);
    for (int i = 0; i < threads; i++)
    {
        pthread_t pt;
        pthread_create(&pt, NULL, start_routine, (void*)NULL);
    }

    sint32 count = 0;
    while (count < 100000)
    {
        sleep(10);
        count += 1;
        print();
    }

    reactor->destroy();
    return 0;
}
