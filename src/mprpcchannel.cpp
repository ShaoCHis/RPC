#include "mprpcchannel.hpp"

/**
 * header_size + service_name method_name args_size + args
 */
// 所有通过stub代理对象调用的rpc方法，都走到这里了，统一做rpc方法调用的数据序列化和网络发送
// 调用方中 done 对象无用
void MprpcChannel::CallMethod(const google::protobuf::MethodDescriptor *method,
                              google::protobuf::RpcController *controller, const google::protobuf::Message *request,
                              google::protobuf::Message *response, google::protobuf::Closure *done)
{
    const google::protobuf::ServiceDescriptor *sd = method->service();
    std::string service_name = sd->name();    // service_name
    std::string method_name = method->name(); // method_name

    // 获取参数的序列化字符串长度 args_size
    std::string args_str;
    uint32_t args_size = 0;
    if (!request->SerializePartialToString(&args_str))
    {
        std::cout << "Serialize request failed!" << std::endl;
        return;
    }
    // 参数的长度
    args_size = args_str.size();

    // 定义rpc的请求header
    mprpc::RpcHeader rpcHeader;
    rpcHeader.set_service_name(service_name);
    rpcHeader.set_method_name(method_name);
    rpcHeader.set_args_size(args_size);

    // 进行header的序列化
    std::string rpc_header_str;
    uint32_t rpc_header_size;
    if (!rpcHeader.SerializePartialToString(&rpc_header_str))
    {
        std::cout << "Serialize rpcHeader failed!" << std::endl;
        return;
    }
    rpc_header_size = rpc_header_str.size();

    // 组织待发送的rpc请求字符串
    std::string send_rpc_str;
    send_rpc_str.insert(0, std::string((char *)&rpc_header_size, 4)); // header_size
    send_rpc_str += rpc_header_str;                                   // rpcHeader
    send_rpc_str += args_str;

    // 打印调试信息
    std::cout << "==========================================" << std::endl;
    std::cout << "header_size:" << rpc_header_size << std::endl;
    std::cout << "rpc_header_str:" << rpc_header_str << std::endl;
    std::cout << "service_name:" << service_name << std::endl;
    std::cout << "method_name:" << method_name << std::endl;
    std::cout << "args_str:" << args_str << std::endl;
    std::cout << "==========================================" << std::endl;

    // 使用tcp编程，完成对rpc请求的调用
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientfd == 1)
    {
        std::cout << "create socket error! errno:" << errno << std::endl;
        exit(EXIT_FAILURE);
    }

    // 服务器相关信息
    std::string ip = MprpcApplication::getConfigLoader().Load("rpcserverip");
    uint16_t port = atoi(MprpcApplication::getConfigLoader().Load("rpcserverport").c_str());

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip.c_str());

    // 连接rpc服务节点
    if (-1 == connect(clientfd, (struct sockaddr *)&server_addr, sizeof(sockaddr)))
    {
        // 连接失败
        std::cout << "connect socket error! errno:" << errno << std::endl;
        close(clientfd);
        exit(EXIT_FAILURE);
    }

    // 发送rpc请求
    if (-1 == send(clientfd, send_rpc_str.c_str(), send_rpc_str.size(), 0))
    {
        // 发送消息失败
        std::cout << "send request error! errno:" << errno << std::endl;
        close(clientfd);
        return;
    }

    // 接收rpc请求的响应值
    char buf[1024] = {0};
    int recv_size = 0;
    if ((recv_size = recv(clientfd, &buf, 1024, 0)) == -1)
    {
        // 接收消息失败
        std::cout << "recv response error! errno:" << errno << std::endl;
        close(clientfd);
        return;
    }

    std::string response_str(buf, 0, recv_size);
    //进行数据的反序列化
    if(!response->ParseFromString(response_str))
    {
        //数据反序列化失败
        std::cout << "parse response error! errno:" << errno << std::endl;
        close(clientfd);
        return;
    }
    close(clientfd);
}