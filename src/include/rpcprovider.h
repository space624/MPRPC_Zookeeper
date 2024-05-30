#pragma once

#include "google/protobuf/service.h"

#include <functional>
#include <google/protobuf/descriptor.h>
#include <muduo/base/Timestamp.h>
#include <muduo/net/Buffer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/TcpConnection.h>
#include <muduo/net/TcpServer.h>
#include <unordered_map>

// 框架提供的专门服务发布rpc服务网络对象类
class RpcProvider
{
public:
  void NotifyService(google::protobuf::Service *service);

  // 启动rpc服务节点
  void Run();

private:
  void OnConnection(const muduo::net::TcpConnectionPtr &);
  void OnMessag(const muduo::net::TcpConnectionPtr &, muduo::net::Buffer *, muduo::Timestamp);
  // Closure回调操作,序列化Rpc的响应和网络发送
  void SendRpcResponse(const muduo::net::TcpConnectionPtr &, google::protobuf::Message *);

private:
  // 组合EventLoop
  muduo::net::EventLoop eventLoop_;

  // 服务类型信息
  struct ServiceInfo
  {
    google::protobuf::Service *service_;
    std::unordered_map<std::string, const google::protobuf::MethodDescriptor *> methodMap_;
  };

  // 注册成功的服务对象和服务方法的信息
  std::unordered_map<std::string, ServiceInfo> serviceMap_;
};