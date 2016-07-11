#include "hcc_sender.h"
#include "asyn_operation.h"

namespace hpf
{

void HccSender::clean()
{
    AsynOperation *ao;
    while (-1 != m_send_buffer_list.pop(ao))
    {
        // doc
        ao->on_operation_complete(-2, 0);
    }
}

}
