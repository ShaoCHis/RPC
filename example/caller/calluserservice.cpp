#include <iostream>

#include "mprpcapplication.hpp"
#include "user.pb.h"
#include "mprpcchannel.hpp"

int main(int argc,char **argv)
{
    //整个程序启动以后，想使用哦mprpc框架来享受rpc服务调用，一定需要先调用框架的初始化函数（只初始化一次）
    MprpcApplication::Init(argc,argv);

    //演示调用远程发布的rpc方法Login
    protobuf::UserServiceRpc_Stub userservice_stub(new MprpcChannel());

    //rpc方法的请求参数
    protobuf::LoginRequest request;
    request.set_name("zhang san");
    request.set_pwd("123456");
    //rpc方法的响应
    protobuf::LoginResponse response;
    //发起rpc方法的调用 同步的rpc调用过程    MprpcChannel::callMethod
    userservice_stub.Login(nullptr,&request,&response,nullptr);   // RpcChannel->RpcChannel::callMethod 集中来做所有rpc方法调用的参数序列化和网络发送

    //一次rpc调用完成，读调用的结果
    if(response.result().errcode()==0)
    {
        //调用成功
        std::cout << "rpc login response : " << response.sucess() << std::endl;
    }
    else
    {
        //调用失败
        std::cout << "rpc login error : " << response.result().errmsg() << std::endl;
    }
    return 0;
}


