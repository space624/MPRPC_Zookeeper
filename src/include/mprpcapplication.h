#pragma once

#include "mprpcconfig.h"
#include "mprpcchannel.h"
#include "mprpccontroller.h"
//mprpc框架初始化类
class MprpcApplication {
    public:
    static void Init(int argc , char** argv);
    static MprpcApplication& getInstance();
    static MprpcConfig& getConfig();
    
    private:
    static MprpcConfig config_;

    MprpcApplication() { };
    MprpcApplication(const MprpcApplication&) = delete;
    MprpcApplication(MprpcApplication&&) = delete;
};