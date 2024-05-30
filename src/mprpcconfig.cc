#include "mprpcconfig.h"
#include <iostream>
#include <string>

void MprpcConfig::LoadConfigFile(const char* configFile) {
    FILE* pf = fopen(configFile , "r");
    if (!pf) {
        std::cout << configFile << "is note exist!" << std::endl;
        exit(EXIT_FAILURE);
    }

    while (!feof(pf)) {
        char buf[512] = { 0 };
        fgets(buf , 512 , pf);
        //去掉字符串多余空格
        std::string readBuf(buf);
        Trim(readBuf);

        //# 注释
        if (readBuf[0] == '#' || readBuf.empty()) {
            continue;
        }

        //解析配置项
        int idx = readBuf.find('=');
        if (idx == -1) {
            //配置不合法
            continue;
        }
        std::string key;
        std::string value;
        key = readBuf.substr(0 , idx);

        Trim(key);
        int endidx = readBuf.find('\n' , idx);

        value = readBuf.substr(idx + 1 , endidx - idx - 1);
        Trim(value);
        configMap_.insert({ key , value });
    }
}

std::string MprpcConfig::Load(const std::string& key) {
    auto it = configMap_.find(key);
    if (it == configMap_.end()) {
        return "";
    }
    return it->second;
}

void MprpcConfig::Trim(std::string& srcBuf) {
    int idx = srcBuf.find_first_not_of(' ');
    if (idx != -1) {
        //字符串有空格
        srcBuf = srcBuf.substr(idx , srcBuf.size() - idx);
    }
    //去掉字符串后面多余的空格
    idx = srcBuf.find_last_not_of(' ');
    if (idx != -1) {
        srcBuf = srcBuf.substr(0 , idx + 1);
    }

}
