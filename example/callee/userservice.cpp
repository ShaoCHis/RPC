#include <iostream>
#include <string>
#include <vector>

#include "user.pb.h"
#include "mprpcapplication.hpp"
#include "friend.pb.h"
#include "logger.hpp"

/**
 * UserService原来是一个本地服务，提供了两个进程内的本地方法，Login和Register
 */

class UserService : public protobuf::UserServiceRpc // 使用在rpc服务发布端（rpc服务提供者）
{
public:
    bool Login(std::string name, std::string pwd)
    {
        std::cout << "doing local service : Login " << std::endl;
        std::cout << "name:" << name << " pwd:" << pwd << std::endl;
        return true;
    }

    bool Register(uint32_t id,std::string name,std::string pwd)
    {
        std::cout << "doing local service : Register " << std::endl;
        std::cout << "id:" << id << " name:" << name << " pwd:" << pwd << std::endl;
        return true;
    }

    // 这里是直接使用框架
    // 重写基类 UserServiceRpc的虚函数  下面这些方法都是框架直接调用的
    /**
     * 1. caller   ====》   Login(LoginRequest)  => muduo => callee
     * 2. callee   ====>    Login(LoginRequest)  => 交到下面重写的这个Login方法上了
     */
    void Login(::google::protobuf::RpcController *controller,
               const ::protobuf::LoginRequest *request,
               ::protobuf::LoginResponse *response,
               ::google::protobuf::Closure *done)
    {
        // 框架给业务上报了请求参数 LoginRequest ，应用程序获取相应数据做本地业务
        std::string name = request->name();
        std::string pwd = request->pwd();

        // 做本地业务
        bool login_result = Login(name, pwd);

        // 把响应写入 response 返回，包括错误码，错误消息、返回值
        protobuf::ResultCode *code = response->mutable_result();
        code->set_errcode(0);
        code->set_errmsg("");
        response->set_sucess(login_result);

        // 执行回调操作   执行响应对象数据的序列化和网络发送（都是由框架来完成的）
        done->Run();
    }

    void Register(::google::protobuf::RpcController *controller,
               const ::protobuf::RegisterRequest *request,
               ::protobuf::RegisterResponse*response,
               ::google::protobuf::Closure *done)
    {
        // 框架给业务上报了请求参数 RegisterRequest ，应用程序获取相应数据做本地业务
        uint32_t id = request->id();
        std::string name = request->name();
        std::string pwd = request->pwd();

        // 做本地业务
        bool register_result = Register(id,name, pwd);

        // 把响应写入 response 返回，包括错误码，错误消息、返回值
        protobuf::ResultCode *code = response->mutable_result();
        code->set_errcode(0);
        code->set_errmsg("");
        response->set_sucess(register_result);

        // 执行回调操作   执行响应对象数据的序列化和网络发送（都是由框架来完成的）
        done->Run();
    }
};


/**
 * FriendService是一个本地服务，提供了进程内的本地方法，GetFriendList方法
 */
class FriendService : public friendlist::FriendServiceRpc
{
public:
    std::vector<std::string> getFriendList(uint32_t id)
    {
        std::cout << "do local getFriendList method,id:" << id << std::endl;
        return {"zhangsan","gaoyang","lisi","wangwu"};
    }

    void GetFriendList(google::protobuf::RpcController* controller,
                    const ::friendlist::GetFriendListRequest* request,
                    ::friendlist::GetFriendListResponse* response,
                    ::google::protobuf::Closure* done)
    {
        uint32_t id = request->id();
        //执行本地
        std::vector<std::string> result = getFriendList(id);
        //封装response
        for(int i = 0;i<result.size();i++)
        {
            std::string* temp = response->add_friends();
            *temp = std::move(result[i]);
        }
        friendlist::ResultCode *code = response->mutable_result();
        code->set_errcode(0);
        code->set_errmsg("query success!!");
        done->Run();
    }
};


int main(int argc,char **argv)
{
    LOG_INFO("first log message");
    LOG_ERR("%s:%s,%d",__FILE__,__FUNCTION__,__LINE__)
    //调用框架的初始化操作     provider -i config.conf读取配置文件操作
    MprpcApplication::Init(argc,argv);

    //框架上让用户发布服务
    //provider是一个rpc网络服务对象。把UserService对象发布到rpc节点上
    MprpcProvider provider;
    provider.NotifyService(new UserService());
    provider.NotifyService(new FriendService());


    //启动一个rpc服务发布节点，Run以后，    进程进入阻塞状态，等待远程的rpc请求
    provider.Run();

    return 0;
}
