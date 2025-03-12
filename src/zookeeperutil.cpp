#include "zookeeperutil.hpp"
#include "mprpcapplication.hpp"
#include "logger.hpp"

#include <iostream>

// 全局的watcher观察器       zkserver 给 zkclient 的通知
void global_watcher(zhandle_t *zh, int type, int state, const char *path, void *watcherCtx)
{
    if (type == ZOO_SESSION_EVENT) // 回调的消息类型是和绘画相关的消息类型
    {
        if (state == ZOO_CONNECTED_STATE) // zkclient 与 zkserver连接成功。
        {
            // 创建成功的回调，通过信号量通知
            sem_t *sem = (sem_t *)zoo_get_context(zh);
            sem_post(sem);
        }
    }
}

ZkClient::ZkClient() : m_zhandle(nullptr)
{
}

ZkClient::~ZkClient()
{
    // 关闭 zk 客户端连接
    if (m_zhandle != nullptr)
        zookeeper_close(m_zhandle); // 关闭句柄，释放资源   MySQL_Conn
}

// zkclient 启动连接 zkserver
void ZkClient::Start()
{
    std::string host = MprpcApplication::getInstance().getConfigLoader().Load("zookeeperip");
    std::string port = MprpcApplication::getInstance().getConfigLoader().Load("zookeeperport");
    std::string connstr = host + ":" + port;

    // zhandle_t *zookeeper_init(const char *host, watcher_fn fn,int recv_timeout, const clientid_t *clientid, void *context, int flags);
    //  global_watcher实际上为回调函数
    /**
     * zookeeper_mt:多线程版本
     * zookeeper的API客户端提供了三个线程
     * API调用线程-zookeeper_init        网络I/O线程-pthread_create             watcher回调线程
     */
    m_zhandle = zookeeper_init(connstr.c_str(), global_watcher, 30000, nullptr, nullptr, 0);
    // 返回值为空，发起动作有问题，并不是创建失败，根本没有到创建连接，仅仅创建句柄(内存初始化成功)
    if (nullptr == m_zhandle)
    {
        LOG_ERR("zookeeper_init error!");
        exit(EXIT_FAILURE);
    }

    sem_t sem;
    sem_init(&sem, 0, 0);
    // 绑定一个信号量
    zoo_set_context(m_zhandle, &sem);

    // 等待信号量被通知
    sem_wait(&sem);
    LOG_INFO("zookeeper_init success!!");
}

// 在 zkserver 上根据指定的path创建 znode节点，默认为0---》永久性节点
void ZkClient::Create(const char *path, const char *data, int datalen, int state)
{
    char path_buffer[128];
    int bufferlen = sizeof(path_buffer);
    int flag;
    // 查找当前node节点是否存在
    // 先判断 path 表示的znode节点是否存在，如果存在，就不再重复创建了
    flag = zoo_exists(m_zhandle, path, 0, nullptr);
    if (ZNONODE == flag)    //表示path的znode节点不存在
    {
        //创建指定path的znode节点
        flag = zoo_create(m_zhandle, path, data, datalen,
                          &ZOO_OPEN_ACL_UNSAFE, state, path_buffer, bufferlen);
        if (flag == ZOK)
        {
            LOG_INFO("znode create success... path:%s", path);
        }
        else
        {
            LOG_ERR("flag:%d", flag);
            LOG_ERR("znode create error... path:%s", path);
            exit(EXIT_FAILURE);
        }
    }
}

// 根据参数指定的 znode 路径获取 znode 节点的值
std::string ZkClient::GetData(const char *path)
{
    char buffer[64];
    int bufferlen = sizeof(buffer);
    int flag = zoo_get(m_zhandle,path,0,buffer,&bufferlen,nullptr);
    if(flag!=ZOK)
    {
        LOG_ERR("get znode error... path:%s",path);
        return "";
    }
    else
    {
        return buffer;
    }
}
