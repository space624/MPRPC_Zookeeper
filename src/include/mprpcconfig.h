#pragma once

#include <unordered_map>
#include <string>

/*
    rpcserverip
    rpcserverport
    zookeeperip
    zookeeperport
*/

//框架读取配置文件类
class MprpcConfig {
    public:
    //加载或解析配置文件
    void LoadConfigFile(const char* configFile);
    //查询配置项信息
    std::string Load(const std::string& key);
    private:
    std::unordered_map<std::string , std::string> configMap_;
    //去掉字符串前后空格
    void Trim(std::string& srcBuf);
};