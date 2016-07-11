/*
 * filename      : message_parser.h
 * descriptor    :  
 * author        : fengyajie
 * create time   : 2011-08-25 09:25
 * modify list   :
 * +----------------+---------------+---------------------------+
 * | date			| who			| modify summary			|
 * +----------------+---------------+---------------------------+
 */
#ifndef _MESSAGE_PARSER_H_
#define _MESSAGE_PARSER_H_
#include "bytebuffer/bytebuffer.h"
#include "fifo.h"
#include "type.h"

namespace hpf
{

class MessageParser
{
    public:
        MessageParser(){}
        virtual ~MessageParser(){}

        virtual xl::sint32 parse_message(ByteBuffer &bb, Fifo<ByteBuffer*> &packets) = 0;
};

}
#endif
