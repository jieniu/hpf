/*
 * filename      : message_processor.h
 * descriptor    :  
 * author        : fengyajie
 * create time   : 2010-12-21 14:06
 * modify list   :
 * +----------------+---------------+---------------------------+
 * | date			| who			| modify summary			|
 * +----------------+---------------+---------------------------+
 */
#ifndef _MESSAGE_PROCESSOR_H_
#define _MESSAGE_PROCESSOR_H_

#include "base_header.h"
#include <sys/socket.h>
class ByteBuffer;

namespace hpf
{

class UdpServer;
class HccConnectionInfo;

class MessageProcessor
{
    public:
        MessageProcessor()
        {}
        virtual ~MessageProcessor()
        {}

        virtual sint32 process_input(ByteBuffer *bb, sint32 handle) = 0;

        virtual sint32 connect(sint32 handle, const string &ip, uint16 port) { return 0; }

        virtual sint32 disconnect(sint32 handle) { return 0; }

        virtual sint32 handle_timeout(sint32 handle) { return 0; }

        virtual sint32 process_udp_message(ByteBuffer *bb, UdpServer *server, 
                        sockaddr* peer, socklen_t peerlen) { return 0; }
};

}
#endif
