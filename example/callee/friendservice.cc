#include "friend.pb.h"
#include "logger.h"
#include "mprpcapplication.h"
#include "rpcprovider.h"

#include <iostream>
#include <string>
#include <vector>

class FriendService : public fixbug::FriendServiceRpc
{
public:
  std::vector<std::string> GetFriendsList(uint32_t userid)
  {
    std::cout << "do GetFriendsList service! userId: " << userid << std::endl;
    std::vector<std::string> vec;
    vec.push_back("gao yang");
    vec.push_back("liu hong");
    vec.push_back("wang shuo");
    return vec;
  }

  // 重写基类方法
  void GetFriendsList(::google::protobuf::RpcController *controller,
                      const ::fixbug::GetFriendsListRequest *request,
                      ::fixbug::GetFriendsListResponse *response,
                      ::google::protobuf::Closure *done)
  {
    uint32_t userid = request->userid();
    std::vector<std::string> friendsList = GetFriendsList(userid);

    response->mutable_result()->set_errcode(0);
    response->mutable_result()->set_errmsg("");

    for (std::string &name : friendsList) {
      std::string *p = response->add_friends();
      *p = name;
    }
    done->Run();
  }
};

int main(int argc, char **argv)
{
  // 调用框架初始化
  MprpcApplication::Init(argc, argv);

  // 发布服务    provider是一个rpc网络服务对象
  RpcProvider provider;
  provider.NotifyService(new FriendService());

  // 启动   进程进入阻塞状态,等待远程rpc调用返回
  provider.Run();

  return 0;
}