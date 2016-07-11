#include "../reactor.h"
#include "../global_var.h"
#include "../message_processor.h"
#include "../udp_asyn_operation.h"
#include "../bytebuffer/bytebuffer.h"
#include "../udp_server.h"
#include <signal.h>
#include <sys/time.h>
#include <sys/resource.h>
using namespace hpf;

int g_cmd_recv = 0;
int g_cmd_send = 0;
int g_send_failed = 0;
pthread_mutex_t g_recv_lock;
pthread_mutex_t g_send_lock;

const int CMDLEN = 2048;

class TestMessageProcessor : public MessageProcessor
{
    public:
        sint32 process_input(ByteBuffer *bb, sint32 handle)
        {
            return 0;
        }
        sint32 process_udp_message(ByteBuffer *bb, UdpServer *server, 
                        sockaddr* peer, socklen_t peerlen) 
        { 
            printf("recv:%s\n", string(bb->address() + bb->position(), bb->remaining()).c_str());
            int len = bb->remaining();
            char *buff = new char[len];
            memcpy(buff, bb->address() + bb->position(), len);
            ByteBuffer resp(buff, len, len);
            UdpAsynOperation *oper = new UdpAsynOperation(resp, peer, peerlen);
            server->send_data(oper);
            return 0; 
        }
};
TestMessageProcessor tp;

void start_listen()
{
    Reactor::get_instance()->create_udp_server("0.0.0.0", 8088, &tp);
}

void print()
{
    pthread_mutex_lock(&g_recv_lock);
    pthread_mutex_lock(&g_send_lock);
    printf("cmd recv:%d\n", g_cmd_recv);
    printf("cmd send:%d\n", g_cmd_send);
    printf("cmd send failed:%d\n\n", g_send_failed);
    pthread_mutex_unlock(&g_send_lock);
    pthread_mutex_unlock(&g_recv_lock);
}

int main(int argc, char **argv)
{
    signal(SIGPIPE, SIG_IGN);

    struct rlimit limit;
    limit.rlim_cur = limit.rlim_max = 1000000;
    if(setrlimit(RLIMIT_NOFILE, &limit) < 0)
    {
        fprintf(stderr,"set rlimit RLIMIT_NOFILE failed!");
    }

    LOG_INIT_EX("../conf/server_log4cplus.conf");
    pthread_mutex_init(&g_recv_lock, NULL);
    pthread_mutex_init(&g_send_lock, NULL);

    Reactor *reactor = Reactor::get_instance();
    reactor->init(atoi(argv[1]), 60);

    start_listen();

    sint32 count = 0;
    while (count < 10000)
    {
        sleep(10);
        count++;
//        print();
    }

    reactor->destroy();
    return 0;
}
