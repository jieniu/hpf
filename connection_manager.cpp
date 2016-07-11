#include "connection_manager.h"
#include "connection_info.h"
#include "asyn_operation.h"
#include "worker_thread.h"

namespace hpf
{

ConnectionManager* ConnectionManager::s_instance = NULL;

ConnectionManager* ConnectionManager::get_instance()
{
    return (s_instance == NULL ? s_instance = new ConnectionManager : s_instance);
}

void ConnectionManager::destroy_instance()
{
    delete s_instance;
    s_instance = NULL;
}

ConnectionManager::ConnectionManager()
{
    m_seqid = 0;
    pthread_mutex_init(&m_mutex, NULL);
}


ConnectionManager::~ConnectionManager()
{
    pthread_mutex_destroy(&m_mutex);
}

sint32 ConnectionManager::get_handle(ConnectionInfo *hci)
{
    pthread_mutex_lock(&m_mutex);
    ++m_seqid;
    while (m_connections.count(m_seqid) != 0)
    {
        ++m_seqid;
    }
    sint32 ret = m_seqid;
    m_connections[m_seqid] = hci;
    pthread_mutex_unlock(&m_mutex);

    return ret;
}


ConnectionInfo* ConnectionManager::get_connection(sint32 seq) 
{
    ConnectionInfo* ret = NULL;
    pthread_mutex_lock(&m_mutex);
    ConnectionInfos::iterator it = m_connections.find(seq);
    if (it != m_connections.end())
    {
        ret = it->second;
    }
    pthread_mutex_unlock(&m_mutex);

    return ret;
}


// TODO we can use a reuse list here
void ConnectionManager::erase_connection(sint32 seq)
{
    pthread_mutex_lock(&m_mutex);

    ConnectionInfos::iterator it = m_connections.find(seq);
    if (it != m_connections.end())
    {
        m_connections.erase(it);
    }

    pthread_mutex_unlock(&m_mutex);
}

/*
 * Function Descript : 给指定connection发送数据
 * Create Time       : 2011-08-03 12:33
 * Parameter List    : 
 * 
 * Return            : 
 *  -1: 失败
 *  0: 成功
 * Modify Time       : 
 */
sint32 ConnectionManager::send_data(sint32 handle, AsynOperation *ao)
{
    sint32 ret = -1;
    pthread_mutex_lock(&m_mutex);
    ConnectionInfos::iterator it = m_connections.find(handle);
    if (it != m_connections.end())
    {
        ConnectionInfo *ci = it->second;
        if (ci->is_connect())
        {
            ret = ci->deliver_message(this, ci, ao);
        }
        else
        {
            ret = -1;
            /* 发送message失败后，自动删除message */
            ao->on_operation_complete(-2, 0);
        }
    }
    else
    {
        ret = -1;
        /* 发送message失败后，自动删除message */
        ao->on_operation_complete(-2, 0);
    }
    pthread_mutex_unlock(&m_mutex);

    return ret;
}

sint32 ConnectionManager::close_connection(sint32 handle)
{
    sint32 ret = -1;
    pthread_mutex_lock(&m_mutex);
    ConnectionInfos::iterator it = m_connections.find(handle);
    if (it != m_connections.end())
    {
        ConnectionInfo *ci = it->second;
        ret = ci->close();
    }
    pthread_mutex_unlock(&m_mutex);

    return ret;
}

void ConnectionManager::process_message(sint32 no, void *arg1, void *arg2)
{
    ConnectionInfo *ci = (ConnectionInfo*)arg1;
    AsynOperation *ao = (AsynOperation *)arg2;
    if (ci->is_connect())
    {
        ci->write_data(no, ao);
    }
    else
    {
        ao->on_operation_complete(-2, 0);
    }
}

}
