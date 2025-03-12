#include <iostream>

#include "mprpcapplication.hpp"
#include "user.pb.h"
#include "friend.pb.h"

int main(int argc, char **argv)
{
    // 整个程序启动以后，想使用哦mprpc框架来享受rpc服务调用，一定需要先调用框架的初始化函数（只初始化一次）
    MprpcApplication::Init(argc, argv);

    // 演示调用远程发布的rpc方法Login
    protobuf::UserServiceRpc_Stub userservice_stub(new MprpcChannel());

    // rpc方法的请求参数
    protobuf::LoginRequest request;
    request.set_name("zhang san");
    request.set_pwd("123456");
    // rpc方法的响应
    protobuf::LoginResponse response;

    //创建rpccontroller管理对象
    MprpcController controller;

    controller.Reset();
    // 发起rpc方法的调用 同步的rpc调用过程    MprpcChannel::callMethod
    userservice_stub.Login(&controller, &request, &response, nullptr); // RpcChannel->RpcChannel::callMethod 集中来做所有rpc方法调用的参数序列化和网络发送

    if(controller.Failed())
    {
        std::cout << controller.ErrorText() << std::endl;
    }
    else
    {
        // 一次rpc调用完成，读调用的结果
        if (response.result().errcode() == 0)
        {
            // 调用成功
            std::cout << "rpc login response : " << response.sucess() << std::endl;
        }
        else
        {
            // 调用失败
            std::cout << "rpc login error : " << response.result().errmsg() << std::endl;
        }
    }

    // rpc方法的请求参数
    protobuf::RegisterRequest registerRequest;
    registerRequest.set_id(2000);
    registerRequest.set_name("zhang san");
    registerRequest.set_pwd("123456");
    // rpc方法的响应
    protobuf::RegisterResponse registerResponse;
    controller.Reset();
    // 发起rpc方法的调用 同步的rpc调用过程    MprpcChannel::callMethod
    userservice_stub.Register(&controller, &registerRequest, &registerResponse, nullptr); // RpcChannel->RpcChannel::callMethod 集中来做所有rpc方法调用的参数序列化和网络发送

    if(controller.Failed())
    {
        std::cout << controller.ErrorText() << std::endl;
    }
    else
    {
        // 一次rpc调用完成，读调用的结果
        if (registerResponse.result().errcode() == 0)
        {
            // 调用成功
            std::cout << "rpc register response : " << registerResponse.sucess() << std::endl;
        }
        else
        {
            // 调用失败
            std::cout << "rpc register error : " << registerResponse.result().errmsg() << std::endl;
        }
    }

    // 演示调用远程发布的rpc方法GetFriendList
    friendlist::FriendServiceRpc_Stub friendServiceRpc_Stub(new MprpcChannel());

    // rpc方法的请求参数
    friendlist::GetFriendListRequest getFriendListRequest;
    getFriendListRequest.set_id(1000);
    // rpc方法的响应
    friendlist::GetFriendListResponse getFriendListResponse;
    controller.Reset();
    // 发起rpc方法的调用 同步的rpc调用过程    MprpcChannel::callMethod
    friendServiceRpc_Stub.GetFriendList(&controller, &getFriendListRequest, &getFriendListResponse, nullptr); // RpcChannel->RpcChannel::callMethod 集中来做所有rpc方法调用的参数序列化和网络发送

    if(controller.Failed())
    {
        std::cout << controller.ErrorText() << std::endl;
    }
    else
    {
        // 一次rpc调用完成，读调用的结果
        if (getFriendListResponse.result().errcode() == 0)
        {
            // 调用成功
            std::cout << "rpc getfriendList response : " << getFriendListResponse.sucess() << " error msg:" << getFriendListResponse.result().errmsg() << std::endl;
            int count = getFriendListResponse.friends_size();
            for (int i = 0; i < count; i++)
            {
                std::cout << getFriendListResponse.friends(i) << std::endl;
            }
        }
        else
        {
            // 调用失败
            std::cout << "rpc getfriendList error : " << getFriendListResponse.result().errmsg() << std::endl;
        }
    }
    return 0;
}
