#pragma once
#include "mprpcconfig.hpp"
#include "mprpcchannel.hpp"
#include "mprpccontroller.hpp"
#include "mprpcprovider.hpp"


//mprpc框架的初始化类，单例模式
class MprpcApplication
{
public:
    //负责框架的初始化操作
    static void Init(int argc,char **argv);

    //单例模式
    static MprpcApplication& getInstance();
    static MprpcConfig& getConfigLoader();

    MprpcApplication(const MprpcApplication&) = delete;
    MprpcApplication(const MprpcApplication&&) = delete;
    MprpcApplication& operator=(const MprpcApplication&) = delete;
    MprpcApplication&& operator=(const MprpcApplication&&) = delete;

private:
    MprpcApplication() = default;
    ~MprpcApplication() = default;
    static MprpcConfig m_config;
};


