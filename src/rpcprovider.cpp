#include "rpcprovider.hpp"
#include "mprpcapplication.hpp"

/**
 * service_name => service描述
 *                      =》 service* 记录服务对象
 *                             =》method_name => method方法对象
 */

// 传入基类，service
// 这里是框架提供给外部使用的，可以发布rpc方法的函数接口
void RpcProvider::NotifyService(google::protobuf::Service *service)
{
    /**
     * 获取服务对象的描述信息
     * const google::protobuf::ServiceDescriptor *pserviceDesc = service->GetDescriptor()
     * 获取服务的名字
     * std::string service_name = pserviceDesc->name();
     * 获取服务对象的方法数量
     * int service_method_count = pserviceDesc->method_count();
     */
    ServiceInfo service_info;
    service_info.m_service = service;
    // 获取服务对象的描述信息
    const google::protobuf::ServiceDescriptor *pserviceDesc = service->GetDescriptor();
    // 获取服务的名字
    std::string service_name = pserviceDesc->name();
    // 获取服务对象的方法数量
    int service_method_count = pserviceDesc->method_count();

    std::cout << "service_name:" << service_name << std::endl;

    for (int i = 0; i < service_method_count; i++)
    {
        // 获取了服务对象指定下标的服务方法的描述（抽象描述）
        const google::protobuf::MethodDescriptor *pmethodDesc = pserviceDesc->method(i);
        std::string method_name = pmethodDesc->name();
        // 存入map中
        service_info.m_methodMap.emplace(method_name, pmethodDesc);
        std::cout << "method_name:" << method_name << std::endl;
    }
    // 存入一个服务
    m_serviceMap.emplace(service_name, service_info);
}

// 启动rpc服务节点，开始提供rpc远程网络调用服务
void RpcProvider::Run()
{
    std::string ip = MprpcApplication::getConfigLoader().Load("rpcserverip");
    uint16_t port = atoi(MprpcApplication::getConfigLoader().Load("rpcserverport").c_str());
    muduo::net::InetAddress address(ip, port);

    // 创建TcpServer对象
    muduo::net::TcpServer server(&m_eventLoop, address, "RpcProvider");
    // 绑定连接回调和消息读写回调方法，分离了网络代码和业务代码
    // 连接回调
    server.setConnectionCallback(std::bind(&RpcProvider::onConnection, this, std::placeholders::_1));

    server.setMessageCallback(std::bind(&RpcProvider::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    // 设置muduo库的线程数量
    server.setThreadNum(4);

    std::cout << "RpcProvider start service at ip:127.0.0.1 port:8000" << std::endl;

    // 启动网络服务
    server.start();
    m_eventLoop.loop(); // epoll_wait
}

// 新的socket连接回调
void RpcProvider::onConnection(const muduo::net::TcpConnectionPtr &conn)
{
    // 和 rpc client的连接断开了
    if (!conn->connected())
        conn->shutdown();
}

/**
 * 在框架内部，RpcProvider和RpcConsumer协商好之间通信用的protobuf数据结构
 * service_name method_name args    定义proto的message类型，进行数据头（service_name method_name）的序列化和反序列化
 *                                  service_name method_name args_size
 * 16UserServiceLoginzhang san123456
 *
 * header_size(4Bytes，二进制存储) + header_str + args_str
 * 定义长度，防止粘包的问题
 *
 * 10 “10”
 * 10000 “10000”
 * std::string insert和copy方法---》二进制存储
 */
// 已建立连接用户的读写事件回调
// 如果远程有一个rpc服务的调用请求，那么onmessage方法就会响应
void RpcProvider::onMessage(const muduo::net::TcpConnectionPtr &conn,
                            muduo::net::Buffer *buf,
                            muduo::Timestamp timestamp)
{
    // 网络上接收的远程rpc调用请求的字符流      Login(MethodName)   args(参数)
    std::string recv_buf = buf->retrieveAllAsString();

    // 从字符流中读取前4个字节的内容
    uint32_t header_size = 0;
    recv_buf.copy((char *)&header_size, 4, 0);

    // 根据header_size读取数据头的原始字符流，反序列化数据，得到rpc请求的详细信息
    std::string rpc_header_str = recv_buf.substr(4, header_size);
    mprpc::RpcHeader rpcHeader;
    // 服务于方法的相关信息
    std::string service_name;
    std::string method_name;
    uint32_t args_size;
    if (rpcHeader.ParseFromString(rpc_header_str))
    {
        // 数据头反序列化成功
        service_name = rpcHeader.service_name();
        method_name = rpcHeader.method_name();
        args_size = rpcHeader.args_size();
    }
    else
    {
        // 数据头反序列化失败
        std::cout << "rpc_header_str:" << rpc_header_str << " parse error!" << std::endl;
        return;
    }

    // 获取rpc方法参数的字符流数据
    std::string args = recv_buf.substr(4 + header_size, args_size);

    // 打印调试信息
    std::cout << "==========================================" << std::endl;
    std::cout << "header_size:" << header_size << std::endl;
    std::cout << "rpc_header_str:" << rpc_header_str << std::endl;
    std::cout << "service_name:" << service_name << std::endl;
    std::cout << "method_name:" << method_name << std::endl;
    std::cout << "args_str:" << args << std::endl;
    std::cout << "==========================================" << std::endl;

    // 框架调用服务方法
    // 获取service对象和method对象
    auto it = m_serviceMap.find(service_name);
    if (it == m_serviceMap.end())
    {
        // 请求了Rpc节点上没有的服务对象
        std::cout << service_name << " is not exist!" << std::endl;
        return;
    }

    auto mit = it->second.m_methodMap.find(method_name);
    if (mit == it->second.m_methodMap.end())
    {
        // 请求了服务对象中没有的方法
        std::cout << service_name << ":" << method_name << " is not exist!" << std::endl;
        return;
    }
    // 拿到服务与方法
    google::protobuf::Service *service = it->second.m_service;      // 获取service对象  new UserService
    const google::protobuf::MethodDescriptor *method = mit->second; // 获取method对象   Login

    // 生成rpc方法调用的请求request和响应response参数
    google::protobuf::Message *request = service->GetRequestPrototype(method).New();
    if (!request->ParseFromString(args))
    {
        // 表示请求参数解析失败
        std::cout << "request args parse error! Content:" << args << std::endl;
        return;
    }
    google::protobuf::Message *response = service->GetResponsePrototype(method).New();

    // 给下面的method方法的调用，绑定一个Closure的回调函数
    /**
     * error: no matching function for call to
     * ‘NewCallback(RpcProvider*, void (RpcProvider::*)(const TcpConnectionPtr&, google::protobuf::Message*),
     *                                      const TcpConnectionPtr&, google::protobuf::Message*&)’
     * 最后的类型推导为 *&，匹配错误
     *
     * 解决方法：NewCallBack<>直接生命类型
     */
    google::protobuf::Closure *done = google::protobuf::NewCallback<RpcProvider,
                                                                    const muduo::net::TcpConnectionPtr &,
                                                                    google::protobuf::Message *>(this,
                                                                                                 &RpcProvider::sendRpcResponse,
                                                                                                 conn, response);

    // 在框架上根据远端rpc请求，调用当前rpc节点上发布的方法
    // new UserService().Login(controller,request,reponse,done)
    service->CallMethod(method, nullptr, request, response, done);
}

// Closure的回调操作，用于序列化rpc的响应和网络发送
void RpcProvider::sendRpcResponse(const muduo::net::TcpConnectionPtr &conn, google::protobuf::Message *response)
{
    std::string response_str;
    if (response->SerializeToString(&response_str)) // response进行序列化
    {
        // 序列化成功后，通过网络把rpc方法执行的结果发送回rpc的调用方
        conn->send(response_str);
    }
    else
    {
        std::cout << "Serialize response_str error!" << std::endl;
    }
    conn->shutdown(); // 模拟http的短连接服务，由rpcprovider主动断开连接
}
