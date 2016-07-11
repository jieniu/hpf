/*
 * filename      : hcc_connection_info.h
 * descriptor    :  
 * author        : fengyajie
 * create time   : 2010-12-15 23:02
 * modify list   :
 * +----------------+---------------+---------------------------+
 * | date			| who			| modify summary			|
 * +----------------+---------------+---------------------------+
 */
#ifndef _HCC_CONNECTION_INFO_H_
#define _HCC_CONNECTION_INFO_H_
#include "base_header.h"
#include "base_define.h"
#include "connection_info.h"
#include "fifo.h"

namespace hpf
{

class HccConnectionInfo : public ConnectionInfo
{
    public:
        HccConnectionInfo(sint32 fd, const char *ip, uint16 port, 
                          MessageProcessor *mp, sint32 is_reconnect, 
                          sint32 connect_timeout);
        virtual ~HccConnectionInfo();

        virtual sint32 write_data(sint32 no, AsynOperation *ao);

        virtual sint32 reconnect(uint32 handle);

        virtual sint32 close();
        
        virtual sint32 update_event();
        
        virtual sint32 deliver_message(MessageHandler *handler, void *arg1, void*arg2);
    public:
        void* operator new(size_t len);
        void operator delete(void* p, size_t len);
    protected:
        //只在工作线程中调用 
        virtual sint32 handle_close();

    private:
        static Fifo<HccConnectionInfo*> s_free_list;
};

}
#endif
