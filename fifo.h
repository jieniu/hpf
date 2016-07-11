/*
 * filename      : fifo.h
 * descriptor    :  fifo implemented by list
 * author        : fengyajie
 * create time   : 2010-12-20 23:17
 * modify list   :
 * +----------------+---------------+---------------------------+
 * | date			| who			| modify summary			|
 * +----------------+---------------+---------------------------+
 */
#ifndef _FIFO_H_
#define _FIFO_H_
#include <pthread.h>
#include <assert.h>

template<class T>
class Fifo
{
    typedef struct _Item
    {
        T _data;
        struct _Item *_next;
    }Item;

    public:
        Fifo()
        {
            pthread_mutex_init(&_mutex, NULL);
            _head = NULL;
            _tail = NULL;
            _number = 0;
        }
        ~Fifo()
        {
            pthread_mutex_destroy(&_mutex);
            while (_head)
            {
                Item *tmp = _head;
                _head = _head->_next;
                delete tmp;
            }
        }

        void push_back(const T &data)
        {
            Item *item = new Item;        
            item->_data = data;
            item->_next = NULL;
            pthread_mutex_lock(&_mutex);

            if (NULL == _head || _number==0)
            {
                _head = item;
                _tail = item;
            }
            else
            {
                _tail->_next = item;
                _tail = item ;
            }
            _number++;
            pthread_mutex_unlock(&_mutex);
        }

        void push_front(const T &data)
        {
            Item *item = new Item;        
            item->_data = data;
            item->_next = NULL;
            pthread_mutex_lock(&_mutex);
            if (NULL == _head)
            {
                _head = item;
                _tail = item;
            }
            else
            {
                item->_next = _head;
                _head = item;
            }
            _number++;
            pthread_mutex_unlock(&_mutex);
        }

        int pop(T &data)
        {
            int ret = -1;
            pthread_mutex_lock(&_mutex);
            if (_head)
            {
                Item *tmp = _head;
                _head = _head->_next;
                _tail = (NULL == _head ? NULL : _tail);
                data = tmp->_data;
                delete tmp;
                _number--;
                ret = 0;
            }
            pthread_mutex_unlock(&_mutex);

            return ret;
        }

        // added by fengyajie: for delete a specific item, not effective
        int erase_latest(const T &data)
        {
            int ret = -1;
            pthread_mutex_lock(&_mutex);
            Item *tmp = _head;
            Item *prev = 0;
            while (tmp)
            {
                if (data == tmp->_data)
                {
                    if (prev)
                        prev->_next = tmp->_next;
                    else
                        _head = tmp->_next;

                    if (tmp->_next == NULL)
                        _tail = prev;

                    if (_head == NULL)
                        _tail = _head;

                    _number--;
                    ret = 0;
                    break;
                }
                prev = tmp;
                tmp = tmp->_next;
            }
            pthread_mutex_unlock(&_mutex);
            return ret;
        }

        int size()
        {
            pthread_mutex_lock(&_mutex);
            int ret = _number;
            if (_number == 0)
                assert(_head == NULL && _tail == NULL);
            pthread_mutex_unlock(&_mutex);
            return ret;
        }
    private:
        pthread_mutex_t _mutex;
        Item *_head; // out
        Item *_tail; // in
        int _number;
};


#endif
