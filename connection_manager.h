#ifndef  _CONNECTION_MANAGER_H_
#define  _CONNECTION_MANAGER_H_

#include "base_header.h"
#include "event_message.h"

namespace hpf
{

class ConnectionInfo;
class AsynOperation;
class ConnectionManager : public MessageHandler
{
    public:
        typedef map<sint32, ConnectionInfo*> ConnectionInfos;

        sint32 get_handle(ConnectionInfo *hci);

        ConnectionInfo *get_connection(sint32 seq);

        void erase_connection(sint32 seq); 

        sint32 send_data(sint32 handle, AsynOperation *ao);

        static ConnectionManager* get_instance();

        static void destroy_instance();
        
        virtual void process_message(sint32 no, void *arg1, void *arg2);

        sint32 close_connection(sint32 handle);
    private:
        ConnectionManager();
        virtual ~ConnectionManager();

        static ConnectionManager* s_instance;

        // seq_id <-> connection_info
        ConnectionInfos m_connections; 
        sint32  m_seqid;
        pthread_mutex_t m_mutex;
};

}
#endif

