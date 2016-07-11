#include "../reactor.h"
#include "../global_var.h"
#include "../message_processor.h"
#include "../timeout_job.h"
using namespace hpf;

class TestTimeout : public TimeoutHandler
{
    public:
        TestTimeout(){}
        ~TestTimeout(){}
        
        bool handle_timeout(uint32 handle, void *arg) 
        {
            sint32 sarg = (sint32)arg;
            LOGGER_DEBUG(g_framework_logger, "handle = " << handle << ", arg=" << sarg);
            return true;
        }
};

void test_timeout(Reactor *reactor, sint32 is_rotate, sint32 sleeptime)
{
    TestTimeout tt;
    timeval tv;
    tv.tv_sec = 3;
    tv.tv_usec = 0;
    sint32 arg = 1;
    uint32 handle = 0;
    reactor->add_timer(&tt, (void*)arg, tv, is_rotate, handle);
    LOGGER_DEBUG(g_framework_logger, "ret handle = " << handle);
    sleep(sleeptime);
    Reactor::get_instance()->cancel_timer(handle);
}

int main()
{
    LOG_INIT_EX("../conf/log4cplus.conf");

    Reactor *reactor = Reactor::get_instance();
    reactor->init(2, 60);

    test_timeout(reactor, 0, 4);
    test_timeout(reactor, 1, 10);

    reactor->destroy();

    return 0;
}
