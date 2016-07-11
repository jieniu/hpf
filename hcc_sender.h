/*
 * filename      : hcc_sender.h
 * descriptor    :  
 * author        : fengyajie
 * create time   : 2011-08-24 20:39
 * modify list   :
 * +----------------+---------------+---------------------------+
 * | date			| who			| modify summary			|
 * +----------------+---------------+---------------------------+
 */
#ifndef _HCC_SENDER_H_
#define _HCC_SENDER_H_
#include "sender.h"

namespace hpf
{

class HccSender : public Sender
{
    public:
        HccSender(){}
        virtual ~HccSender(){} 
        
        virtual void clean(); 
};

}
#endif
