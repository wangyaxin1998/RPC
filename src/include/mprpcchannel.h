#pragma once
#include <string>

#include <unistd.h>
#include<arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <error.h>


#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>

#include "rpcheader.pb.h"
#include "mprpcapplication.h"

class MprpcChannle : public google::protobuf::RpcChannel
{
public:
    //所有通过stub代理对象调用的rpc方法，都走到这里，
    void CallMethod(const google::protobuf::MethodDescriptor *method,
                    google::protobuf::RpcController *controller,
                    const google::protobuf::Message *request,
                    google::protobuf::Message *response,
                    google::protobuf::Closure *done);
};