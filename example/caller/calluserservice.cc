#include "mprpcapplication.h"
#include "user.pb.h"
#include "mprpcchannel.h"

#include <iostream>

int main(int argc, char **argv)
{
    MprpcApplication::Init(argc, argv);

    // 演示登录
    fixbug::UserServiceRpc_Stub stub(new MprpcChannel());
    fixbug::LoginRequest request;
    request.set_name("zhang san");
    request.set_pwd("123456");

    fixbug::LoginResponse response;
    stub.Login(nullptr, &request, &response, nullptr);

    if (response.result().errcode() == 0)
    {
        std::cout << "rpc login respone success: " << response.sucess() << std::endl;
    }
    else
    {
        std::cout << "rpc login response error: " << response.result().errmsg() << std::endl;
    }

    // 演示注册
    fixbug::RegisterRequest req;
    req.set_id(2000);
    req.set_name("mprpc");
    req.set_pwd("666666");
    fixbug::RegisterResponse rsp;
    // 同步方式发起Rpc,等待返回结果
    stub.Register(nullptr, &req, &rsp, nullptr);
    if (response.result().errcode() == 0)
    {
        std::cout << "rpc Register respone success: " << response.sucess() << std::endl;
    }
    else
    {
        std::cout << "rpc Register response error: " << response.result().errmsg() << std::endl;
    }

    return 0;
}