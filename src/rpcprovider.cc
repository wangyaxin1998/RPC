#include "rpcprovider.h"
#include "logger.h"
#include "zookeeperutil.h"

/*service_name->service描述->service*记录服务对象->method_name->method方法对象*/

//这里是框架提供给外部使用的，可以发布rpc方法的函数接口
void RpcProvider::NotifyService(google::protobuf::Service *service)
{
    /*
    接收一个rpc调用请求时，它怎么知道要调用应用程序的哪个服务对象的哪个rpc方法呢？
    需要生成一张表，记录服务对象和其发布的所有的服务方法
    Service类->对象 Method类->方法
    UserService Login Register
    FriendService AddFriends
    */
    ServiceInfo service_info;

    //获取服务对象的描述信息
    const google::protobuf::ServiceDescriptor *pserviceDesc = service->GetDescriptor();
    //获取服务对象service方法的数量
    int methodCount = pserviceDesc->method_count();
    //获取服务的名字
    std::string service_name = pserviceDesc->name();
    LOG_INFO("service_name:%s",service_name.c_str());
    for (int i = 0; i < methodCount; ++i)
    {
        //获取了服务对象指定下标的服务方法的描述（抽象描述）
        const google::protobuf::MethodDescriptor *pmethodDesc = pserviceDesc->method(i);
        std::string method_name = pmethodDesc->name();
        service_info.m_methodMap.insert({method_name, pmethodDesc}); //对象，Method
        LOG_INFO("method_name:%s",method_name.c_str());
    }
    service_info.m_service = service;

    m_serviceInfoMap.insert({service_name, service_info}); //Class，对象
}

//启动rpc服务节点，开始提供rpc远程网络服务
void RpcProvider::Run()
{
    std::string ip = MprpcApplication::GetConfig().Load("rpcserverip");
    uint16_t port = atoi(MprpcApplication::GetConfig().Load("rpcserverport").c_str());
    muduo::net::InetAddress address(ip, port);

    //创建TcpServer对象
    muduo::net::TcpServer server(&m_eventLoop, address, "RpcProvider");

    //绑定连接回调和消息读写回调方法 分离了网络代码和业务代码
    server.setConnectionCallback(std::bind(&RpcProvider::onConnection, this, std::placeholders::_1));
    server.setMessageCallback(std::bind(&RpcProvider::onMessage, this,
                                        std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    //设置muduo库的线程数量
    server.setThreadNum(2);

    //把当前rpc节点上要发布的服务全部注册到zk上边，让rpc client可以从zk上发现服务
    //session timeout 30s  zkclient 网络I/O线程 1/3*timeout时间发送ping消息，证明服务尚且提供
    ZkClient zkCli;
    zkCli.Start();
    // /service_name为永久性节点 method_name为临时性节点
    for(auto& sp:m_serviceInfoMap)
    {
        //service_name /类名
        std::string service_path="/"+sp.first;
        zkCli.Create(service_path.c_str(),nullptr,0);
        for(auto& mp:sp.second.m_methodMap)
        {
            // /service_name/method_name /类名/方法名
            std::string method_path=service_path+"/"+mp.first;
            char method_path_data[128]={0};
            sprintf(method_path_data,"%s:%d",ip.c_str(),port);
            //ZOO_EPHEMERAL表示znode是一个临时性节点
            zkCli.Create(method_path.c_str(),method_path_data,strlen(method_path_data),ZOO_EPHEMERAL);
        }
    }
    std::cout << "RpcProvider start service at ip:" << ip << " port:" << port << std::endl;
    //启动网络服务
    server.start();
    m_eventLoop.loop();
}

//新的socket连接方法
void RpcProvider::onConnection(const muduo::net::TcpConnectionPtr &conn)
{
    if (!conn->connected())
    {
        //和rpc client的连接断开了
        conn->shutdown();
    }
}

/*在框架内部，RpcProvider和RpcConsumer协商好之前通信用的protobuf数据类型
service_name method_name args 定义proto的messgae类型，进行数据的序列化和反序列化
防止出现粘包问题  service_name method_name args_size
header_size(4字节)+header_str

*/

//已建立连接用户的读写事件回调
void RpcProvider::onMessage(const muduo::net::TcpConnectionPtr &conn,
                            muduo::net::Buffer *buffer,
                            muduo::Timestamp)
{
    //网络上接收的远程rpc调用请求的字符流  方法名和参数
    std::string recv_buf = buffer->retrieveAllAsString();
    //从字符流中读取前4个字节的内容
    uint32_t header_size = 0;
    //从recv_buf的第0个字节开始，拷贝四个字节至header_size
    recv_buf.copy((char *)&header_size, 4, 0);

    //根据header_size读取数据头的原始字符流,反序列化数据，得到rpc请求的详细信息
    std::string rpc_header_str = recv_buf.substr(4, header_size);
    mprpc::RpcHeader rpcHeader;
    std::string service_name;
    std::string method_name;
    uint32_t args_size;
    if (rpcHeader.ParseFromString(rpc_header_str))
    {
        //数据头反序列化成功
        service_name = rpcHeader.service_name();
        method_name = rpcHeader.method_name();
        args_size = rpcHeader.args_size();
    }
    else
    {
        //数据头反序列化失败
        LOG_ERR("rpc_header_str:%s parse error!",rpc_header_str.c_str());
        return;
    }
    //获取rpc方法参数的字符流数据
    std::string args_str = recv_buf.substr(4 + header_size, args_size);

    //打印调试信息
    std::cout << "======================================================" << std::endl;
    std::cout << "header_size：" << header_size << std::endl;
    std::cout << "rpc_header_str: " << rpc_header_str << std::endl;
    std::cout << "service_name: " << service_name << std::endl;
    std::cout << "method_name: " << method_name << std::endl;
    std::cout << "args_str: " << args_str << std::endl;
    std::cout << "======================================================" << std::endl;

    //获取service对象
    auto it = m_serviceInfoMap.find(service_name);
    if (it == m_serviceInfoMap.end())
    {
        LOG_ERR("%s is not exist!",service_name.c_str());
        return;
    }

    //获取method对象
    auto mit = it->second.m_methodMap.find(method_name);
    if (mit == it->second.m_methodMap.end())
    {
        LOG_ERR("%s is not exist!",method_name.c_str());
        return;
    }

    google::protobuf::Service *service = it->second.m_service;      //获取service对象
    const google::protobuf::MethodDescriptor *method = mit->second; //获取method对象

    //生成rpc方法调用的请求request和响应response参数
    google::protobuf::Message *request = service->GetRequestPrototype(method).New(); //请求对象
    if (!request->ParseFromString(args_str))
    {
        LOG_ERR("request parse error,content:%s",args_str.c_str());
        return;
    }
    google::protobuf::Message *response = service->GetResponsePrototype(method).New(); //响应对象

    //给下面的method方法调用，绑定一个Closure回调函数
    google::protobuf::Closure *done = google::protobuf::NewCallback<RpcProvider,
                                                                    const muduo::net::TcpConnectionPtr &,
                                                                    google::protobuf::Message *>(this, &RpcProvider::SendRpcResponse, conn, response);

    //在框架上根据远端rpc请求，调用当前rpc节点上发布的方法
    service->CallMethod(method, nullptr, request, response, done);
}

//Closure的回调操作，用于序列化rpc的响应和网络发送
void RpcProvider::SendRpcResponse(const muduo::net::TcpConnectionPtr &conn, google::protobuf::Message *response)
{
    std::string response_str;
    if (response->SerializeToString(&response_str))
    {
        //序列化成功后，通过网络把rpc方法执行的结果发送给rpc的调用方
        //发送的是字符流，最后接收方还会反序列化的
        conn->send(response_str);
    }
    else LOG_ERR("serialize response_str error!"); 
    conn->shutdown(); //模拟http的短链接服务，由rpcprovider主动断开连接
}