#include "../reactor.h"
#include "../global_var.h"
#include "../message_processor.h"
#include "../asyn_operation.h"
#include "../connection_manager.h"
#include "../hcc_sender.h"
#include "../message_parser.h"
#include "../htp_connection_info.h"
#include "../SDSocketUtility.h"
#include <signal.h>
using namespace hpf;

//const char* g_ip = "127.0.0.1";
const char* g_ip = "10.1.1.120";
//const char* g_ip = "10.1.3.166";
int g_conn_per_thread = 0;
int g_cmd_per_conn = 0;
volatile int g_cmd_send = 0;
volatile int g_cmd_recv = 0;
int g_send_failed = 0;
int g_total_send = 0;
int g_cmd_delt = 0;
pthread_mutex_t g_send_lock;
pthread_mutex_t g_recv_lock;
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
                pthread_mutex_lock(&g_send_lock);
                ++g_send_failed;
                pthread_mutex_unlock(&g_send_lock);
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

class TestParser : public MessageParser
{
public:
    ~TestParser () {}
    sint32 parse_message(ByteBuffer &bb, Fifo<ByteBuffer*> &packets)
    {
        while (bb.remaining() >= CMDLEN)
        {
            char *buf = new char[CMDLEN];
            ByteBuffer *buff = new ByteBuffer(buf, CMDLEN, CMDLEN);
            memcpy(buf, bb.address(), CMDLEN);
            bb.position(bb.position() + CMDLEN);
            packets.push_back(buff);
        }
        return 0;
    }
};

class TestMessageProcessor : public MessageProcessor
{
public:
    TestMessageProcessor()
    {
        m_cmd_send = 0;
        m_handle = -1;
    }
    sint32 process_input(ByteBuffer *bb, sint32 handle)
    {
        LOGGER_DEBUG(g_framework_logger, "process input.");
        pthread_mutex_lock(&g_recv_lock);
        ++g_cmd_recv;
        pthread_mutex_unlock(&g_recv_lock);
        delete [] bb->address();
        delete bb;
        return 0;
    }

    sint32 connect(sint32 handle, const string &ip, uint16 port)
    {
        LOGGER_DEBUG(g_framework_logger, "connect success, get a handle=" << handle);
        m_handle = handle;
        return 0;
    }

    bool send_cmd()
    {
        if (m_cmd_send >= g_cmd_per_conn)
        {
            return false;
        }
        if (m_handle < 0)
        {
            return true;
        }
        ++g_total_send;
        ++m_cmd_send;
        char *buffer = new char[CMDLEN];
        memset(buffer, 0, CMDLEN);
        ByteBuffer bb(buffer, CMDLEN, CMDLEN);
        TestOperation *to = new TestOperation(bb);
        if (0 != ConnectionManager::get_instance()->send_data(m_handle, to))
//        if (0 != m_conn->write_data(0, to))
        {
            printf("send data failed conn handle:%d\n", m_handle);
            delete to;
        }
        return true;
    }
    sint32 disconnect(sint32 handle)
    {
        m_handle = -1;
        LOGGER_DEBUG(g_framework_logger, "disconnect handle=" << handle);
        return 0;
    }

    int m_cmd_send;
    sint32 m_handle;
};

TestParser parser;

void *start_routine(void *arg)
{
    TestMessageProcessor *tm = new TestMessageProcessor[g_conn_per_thread];
    for (int i = 0; i < g_conn_per_thread; i++)
    {
        Reactor::get_instance()->add_htp_connector(g_ip, 8088, &parser, tm+i, true);
    }
    int count = 0;
    while (true)
    {
//        if (g_total_send - g_cmd_recv > g_cmd_delt)
//        {
//            continue;
//        }
        ++count;
        bool ret = false;
        for (int i = 0; i < g_conn_per_thread; i++)
        {
            ret |= tm[i].send_cmd();
        }
        if (!ret)
        {
            break;
        }
    }
    printf("cmd send finished!!!count:%d\n", count);
    for (int i = 0; i < g_conn_per_thread; i++)
    {
        printf("conn %d send cmd:%d\n", i + 1, tm[i].m_cmd_send);
    }
    return 0;
} 

void print()
{
    printf("time:%d\n", time(NULL));
    pthread_mutex_lock(&g_send_lock);
    pthread_mutex_lock(&g_recv_lock);
    printf("cmd send:%d\n", g_cmd_send);
    printf("cmd recv:%d\n", g_cmd_recv);
    printf("cmd send failed:%d\n\n", g_send_failed);
    pthread_mutex_unlock(&g_recv_lock);
    pthread_mutex_unlock(&g_send_lock);
}

void test_client()
{
    sockaddr_in addr;
    int fd = SDSocketUtility::connect_to(g_ip, 8088, &addr);
    char buff[2048];
    while (true)
    {
        SDSocketUtility::send_data(fd, buff, 2048);
    }
}

int main(int argc, const char* argv[])
{
    signal(SIGPIPE, SIG_IGN);

    LOG_INIT_EX("../conf/client_log4cplus.conf");

    Reactor *reactor = Reactor::get_instance();
    reactor->init(atoi(argv[5]), 60);

    int threads = atoi(argv[1]);
    g_conn_per_thread = atoi(argv[2]);
    g_cmd_per_conn = atoi(argv[3]);
    g_cmd_delt = atoi(argv[4]);
    for (int i = 0; i < threads; i++)
    {
        pthread_t pt;
        pthread_create(&pt, NULL, start_routine, (void*)NULL);
    }

    int total = 10;
    if (argc > 6)
    {
        total = atoi(argv[6]);
    }
    sint32 count = 0;
    while (count < total)
    {
        sleep(10);
        count += 1;
        print();
    }

    reactor->destroy();
    return 0;
}
