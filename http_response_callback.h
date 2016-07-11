/*
 * filename      : http_response_callback.h
 * Copyright (c) 2003-2012 Xunlei Inc.
 * author        : fengyajie@xunlei.com
 * descriptor    :  
 * create time   : 2012-09-28 01:16
 * modify list   :
 */
#ifndef HTTP_RESPONSE_CALLBACK_H_
#define HTTP_RESPONSE_CALLBACK_H_
namespace hpf
{
class HttpResponseCallBack
{
public:
    HttpResponseCallBack(){}
    virtual ~HttpResponseCallBack(){}

    virtual void response_callback(bool success, char *result, int len, void *param) = 0;
};

}

#endif
