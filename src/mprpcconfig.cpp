#include "mprpcconfig.hpp"

#include <iostream>
#include <string>

// 负责解析加载配置文件
void MprpcConfig::LoadConfigFile(const char *config_file)
{
    FILE *pf = fopen(config_file, "r");
    if (nullptr == pf)
    {
        std::cout << config_file << " is not exist!" << std::endl;
        exit(EXIT_FAILURE);
    }

    // 开始读取
    while (!feof(pf))
    {
        // 1.注释
        // 2.正确的配置项 =
        // 3.去除多余的空格
        char buffer[512] = {0};
        fgets(buffer, 512, pf);

        
        std::string read_buf(buffer);
        // 去掉字符串前面多余的空格
        Trim(read_buf);

        //判断#号注释
        if(read_buf[0]=='#' || read_buf.empty())
            continue;

        //读取配置    key=value
        int idx = read_buf.find('=');
        if(idx == -1)
        {
            //配置项不合法
            continue;
        }
        std::string key;
        std::string value;
        key = read_buf.substr(0,idx);
        Trim(key);
        int endIdx = read_buf.find('\n',idx);
        //去除换行符
        value = read_buf.substr(idx+1,endIdx-idx-1);
        Trim(value);
        //存入 map中
        m_configMap.emplace(key,value);
    }
}

// 查询配置项信息
std::string MprpcConfig::Load(const std::string key)
{
    if (m_configMap.find(key) != m_configMap.end())
        return m_configMap[key];
    else
        return "";
}

//去掉字符串前后的空格
void MprpcConfig::Trim(std::string &src_buf)
{
    int idx = src_buf.find_first_not_of(' ');
    if (idx != -1)
    {
        // 说明字符串前面有空格
        src_buf = src_buf.substr(idx, src_buf.size() - idx);
    }
    // 去掉字符串后面多余的空格
    idx = src_buf.find_last_not_of(' ');
    if (idx != -1)
    {
        // 说明字符串后面有空格
        src_buf = src_buf.substr(0, idx+1);
    }
}
