#pragma once

#include <semaphore.h>
#include <zookeeper/zookeeper.h>
#include <string>

// 服务调用方在请求某个服务时，就可以通过zk的客户端去zk上查找相关服务所在的服务器信息
//  封装的zk客户端类
class ZkClient
{
public:
    ZkClient();
    ~ZkClient();
    // zkclient 启动连接 zkserver
    void Start();
    // 在 zkserver 上根据指定的path创建 znode节点，默认为0---》永久性节点
    void Create(const char *path, const char *data, int datalen, int state = 0);
    // 根据参数指定的 znode 路径获取 znode 节点的值
    std::string GetData(const char *path);

private:
    // zk的客户端句柄
    zhandle_t *m_zhandle;
};
