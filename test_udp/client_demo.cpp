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
const char* g_ip = "10.10.196.172";
int g_cmd_send = 0;
int g_cmd_recv = 0;
int g_send_failed = 0;
pthread_mutex_t g_send_lock;
pthread_mutex_t g_recv_lock;
const int CMDLEN = 2048;


void *start_routine(void *arg)
{
    sockaddr_in addr;
//    int fd = SDSocketUtility::bind_to(g_ip, 8888, &addr);
    int fd = SDSocketUtility::create_socket_udp();
    SDSocketUtility::make_address(&addr, g_ip, 8088);
    char buff[1024] = "foiuhepwohewuwepuwiup";
    if (SDSocketUtility::send_data(fd, buff, strlen(buff), (sockaddr*)&addr, sizeof(addr)) < 0)
    {
        printf("send failed:%s", strerror(errno));
    }
    memset(&addr, 0, sizeof(addr));
    socklen_t addrlen = sizeof(addr);
    int len = SDSocketUtility::recv_data(fd, buff, sizeof(buff), (sockaddr*)&addr, &addrlen);
    if (len < 0)
    {
        printf("recv failed:%s\n", strerror(errno));
        return;
    }
    printf("recv:%s\n", string(buff, len).c_str());
    return 0;
} 

void print()
{
    pthread_mutex_lock(&g_send_lock);
    pthread_mutex_lock(&g_recv_lock);
    printf("cmd send:%d\n", g_cmd_send);
    printf("cmd recv:%d\n", g_cmd_recv);
    printf("cmd send failed:%d\n", g_send_failed);
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
    }
    signal(SIGPIPE, SIG_IGN);
    LOG_INIT_EX("../conf/client_log4cplus.conf");

    pthread_mutex_init(&g_recv_lock, NULL);
    pthread_mutex_init(&g_send_lock, NULL);
    Reactor *reactor = Reactor::get_instance();
    reactor->init(atoi(argv[1]), 60);

    int threads = atoi(argv[2]);
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
//        print();
    }

    reactor->destroy();
    return 0;
}
