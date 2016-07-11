/*
 * filename      : event_message.h
 * descriptor    : 传递给event_thread的消息
 * author        : fengyajie
 * create time   : 2011-08-03 14:55
 * modify list   :
 * +----------------+---------------+---------------------------+
 * | date			| who			| modify summary			|
 * +----------------+---------------+---------------------------+
 */
#ifndef _EVENT_MESSAGE_H_
#define _EVENT_MESSAGE_H_
#include "base_header.h"
#include "fifo.h"

namespace hpf
{

class MessageHandler
{
    public:
        MessageHandler(){}
        virtual ~MessageHandler(){}

        virtual void process_message(sint32 no, void *arg1, void *arg2) = 0;
};

class EventMessage;
bool operator==(const EventMessage &, const EventMessage &);
bool operator!=(const EventMessage &, const EventMessage &);

class EventMessage
{
    public:
        EventMessage()
            : m_handler(0), m_arg1(0), m_arg2(0)
        {}
        EventMessage(MessageHandler *handler, void *arg1, void *arg2)
            : m_handler(handler), m_arg1(arg1), m_arg2(arg2)
        {}
        ~EventMessage()
        {}

        sint32 handle_message(sint32 no);

        friend bool operator==(const EventMessage &, const EventMessage &);
        friend bool operator!=(const EventMessage &, const EventMessage &);

    private:
        MessageHandler *m_handler;
        void *m_arg1;
        void *m_arg2;
};

typedef Fifo<EventMessage> MessageQueue;
typedef Fifo<EventMessage *> PMessageQueue;

inline
bool operator==(const EventMessage &left, const EventMessage &right)
{
    return (left.m_handler == right.m_handler) 
        && (left.m_arg1 == right.m_arg1)
        && (left.m_arg2 == right.m_arg2);
}

inline
bool operator!=(const EventMessage &left, const EventMessage &right)
{
    return !(left == right);
}

inline
sint32 EventMessage::handle_message(sint32 no)
{
    if (m_handler)
    {
        m_handler->process_message(no, m_arg1, m_arg2);
        return 0;
    }
    return -1;
}

}
#endif
