#include <iostream>
#include <string>

#include "mprpcapplication.h"
#include "rpcprovider.h"
#include "user.pb.h"

class UserService : public fixbug::UserServiceRpc
{
public:
    bool Login(std::string name, std::string pwd)
    {
        std::cout << "doing local service: Login" << std::endl;
        std::cout << "name : " << name << "pwd : " << pwd << std::endl;
        return true;
    }

    bool Register(uint32_t id, std::string name, std::string pwd)
    {
        std::cout << "doing local service: Register" << std::endl;
        std::cout << "id: " << id << " name: " << name << " pwd: " << pwd << std::endl;
        return true;
    }

    // protobuf框架直接调用
    // 重写基类虚函数
    void Login(::google::protobuf::RpcController *controller,
               const ::fixbug::LoginRequest *request,
               ::fixbug::LoginResponse *response,
               ::google::protobuf::Closure *done)
    {
        // 上报请求参数LoginRequest,应用获取响应数据做本地业务
        std::string name = request->name();
        std::string pwd = request->pwd();

        // 本地业务
        bool login_result = Login(name, pwd);

        // 返回响应
        fixbug::ResultCode *code = response->mutable_result();
        code->set_errcode(0);
        code->set_errmsg("");
        response->set_sucess(login_result);

        // 回调
        done->Run();
    }

    void Register(::google::protobuf::RpcController *controller,
                  const ::fixbug::RegisterRequest *request,
                  ::fixbug::RegisterResponse *response,
                  ::google::protobuf::Closure *done)
    {
        uint32_t id = request->id();
        std::string name = request->name();
        std::string pwd = request->pwd();
        bool ret = Register(id, name, pwd);

        response->mutable_result()->set_errcode(0);
        response->mutable_result()->set_errmsg("");
        response->set_sucess(ret);

        done->Run();
    }
};

int main(int argc, char **argv)
{
    // 调用框架初始化
    MprpcApplication::Init(argc, argv);

    // 发布服务    provider是一个rpc网络服务对象
    RpcProvider provider;
    provider.NotifyService(new UserService());

    // 启动   进程进入阻塞状态,等待远程rpc调用返回
    provider.Run();

    return 0;
}