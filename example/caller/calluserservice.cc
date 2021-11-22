#include <iostream>
#include "mprpcapplication.h"
#include "user.pb.h"

int main(int argc, char **argv)
{
    //整个程序启动以后，想使用mprpc框架来享受rpc调用服务，一定要先调用框架的初始化函数(只初始化一次)
    MprpcApplication::Init(argc, argv);

    //演示调用远程发布的rpc方法Login
    MprpcController controller;
    wyx::UserServiceRpc_Stub stub(new MprpcChannle());
    wyx::LoginRequest request;
    request.set_name("zhang san");
    request.set_pwd("123456");
    wyx::LoginResponse response;
    stub.Login(&controller, &request, &response, nullptr); //RpcChannel->RpcChannel::callMethod 集中来做所有

    //一次rpc调用完成，读调用的结果
    if (controller.Failed())
    {
        std::cout << controller.ErrorText() << std::endl;
    }
    else
    {
        if (0 == response.result().errcode())
        {
            std::cout << "rpc login response sucess:" << response.sucess() << std::endl;
        }
        else
        {
            std::cout << "rpc login response error:" << response.result().errmsg() << std::endl;
        }
    }

    wyx::RegisterRequest req;
    req.set_id(1059299268);
    req.set_name("yimuchenglin");
    req.set_pwd("123456abc");
    wyx::RegisterResponse rsp;
    //以同步的方式发起rpc调用请求，读取调用结果
    stub.Register(nullptr, &req, &rsp, nullptr);
    if (0 == rsp.result().errcode())
    {
        std::cout << "rpc register response sucess:" << response.sucess() << std::endl;
    }
    else
    {
        std::cout << "rpc register response error:" << response.result().errmsg() << std::endl;
    }
    return 0;
}
