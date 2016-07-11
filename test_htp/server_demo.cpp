#include "../reactor.h"
#include "../global_var.h"
#include "../message_processor.h"
#include "../bytebuffer/bytebuffer.h"
#include "../asyn_operation.h"
#include "../connection_manager.h"
#include "../message_parser.h"
#include "../SDSocketUtility.h"
#include <signal.h>
using namespace hpf;

//const char* g_ip = "10.1.7.176";
//const char* g_ip = "10.1.3.166";
const char* g_ip = "0.0.0.0";
int g_connectino_num = 0;
int g_cmd_recv = 0;
int g_cmd_send = 0;
int g_send_failed = 0;
int g_cmd_resp = 0;
pthread_mutex_t g_conn_lock;
pthread_mutex_t g_recv_lock;
pthread_mutex_t g_send_lock;
pthread_mutex_t g_resp_lock;

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

class TestParser : public MessageParser
{
public:
    ~TestParser () {}
    sint32 parse_message(ByteBuffer &bb, Fifo<ByteBuffer*> &packets)
    {
        while (bb.remaining() >= CMDLEN)
        {
            pthread_mutex_lock(&g_recv_lock);
            ++g_cmd_recv;
            pthread_mutex_unlock(&g_recv_lock);
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
    sint32 process_input(ByteBuffer *bb, sint32 handle)
    {
        LOGGER_DEBUG(g_framework_logger, "process cmd. pthread_self = << " << pthread_self() << ", bufferlen = " << bb->remaining());
        char *buf = new char[CMDLEN];
        ByteBuffer buff(buf, CMDLEN, CMDLEN);
        memcpy(buf, bb->address(), CMDLEN);
        RespAsynOperation *oper = new RespAsynOperation(buff);
        if (ConnectionManager::get_instance()->send_data(handle, oper) < 0)
        {
            delete oper;
        }
        delete [] bb->address();
        delete bb;
        pthread_mutex_lock(&g_resp_lock);
        ++g_cmd_resp;
        pthread_mutex_unlock(&g_resp_lock);
        return 0;
    }

    sint32 connect(sint32 handle, const string &ip, uint16 port)
    {
        LOGGER_DEBUG(g_framework_logger, "connect get a handle=" << handle);
        pthread_mutex_lock(&g_conn_lock);
        ++g_connectino_num;
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

TestParser parser;
TestMessageProcessor tp;

void start_listen()
{
    Reactor::get_instance()->add_htp_listener(g_ip, 8088, &parser, &tp, 0);
}

void print()
{
    printf("time:%d\n", time(NULL));
    pthread_mutex_lock(&g_recv_lock);
    pthread_mutex_lock(&g_send_lock);
    printf("connection num:%d\n", g_connectino_num);
    printf("cmd recv:%d\n", g_cmd_recv);
    printf("cmd send:%d\n", g_cmd_send);
    printf("cmd resp:%d\n", g_cmd_resp);
    printf("cmd send failed:%d\n\n", g_send_failed);
    pthread_mutex_unlock(&g_send_lock);
    pthread_mutex_unlock(&g_recv_lock);
}

void test_server()
{
    sockaddr_in addr;
    int fd = SDSocketUtility::listen_to(g_ip, 8088, &addr, true);
    int client = SDSocketUtility::accept_client(fd, &addr);
    char buff[2048];
    while (true)
    {
        SDSocketUtility::recv_data(client, buff, 2048);
    }
}

int main(int argc, char **argv)
{
    signal(SIGPIPE, SIG_IGN);

    LOG_INIT_EX("../conf/server_log4cplus.conf");
    pthread_mutex_init(&g_conn_lock, NULL);
    pthread_mutex_init(&g_recv_lock, NULL);
    pthread_mutex_init(&g_send_lock, NULL);

    Reactor *reactor = Reactor::get_instance();
    reactor->init(atoi(argv[1]), 60);

    start_listen();

    int total = 10;
    if (argc > 2)
    {
        total = atoi(argv[2]);
    }
    sint32 count = 0;
    while (count < total)
    {
        sleep(10);
        count++;
        print();
    }

    reactor->destroy();
    return 0;
}
