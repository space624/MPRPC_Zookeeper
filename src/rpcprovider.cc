#include "rpcprovider.h"
#include "logger.h"
#include "mprpcapplication.h"
#include "rpcheader.pb.cc"
#include "zookeeperutil.h"

#include <string>

void RpcProvider::NotifyService(google::protobuf::Service *service)
{
  ServiceInfo serviceInfo;

  // 获取服务对象描述信息
  const google::protobuf::ServiceDescriptor *pserviceDesc = service->GetDescriptor();

  // 获取服务信息
  std::string serviceName = pserviceDesc->name();
  int methodCnt = pserviceDesc->method_count();

  // std::cout << "serviceName" << serviceName << std::endl;
  LOG_INFO("serviceName : %s", serviceName.c_str());
  for (int i = 0; i < methodCnt; ++i) {
    // 服务对象下标的服务方法
    const google::protobuf::MethodDescriptor *pmethodDesc = pserviceDesc->method(i);
    std::string methodName = pmethodDesc->name();
    serviceInfo.methodMap_.insert({methodName, pmethodDesc});
    // std::cout << "methodName" << methodName << std::endl;
    LOG_INFO("methodName : %s", methodName.c_str());
  }
  serviceInfo.service_ = service;
  serviceMap_.insert({serviceName, serviceInfo});
}
// 启动rpc服务节点
void RpcProvider::Run()
{
  std::string ip = MprpcApplication::getInstance().getConfig().Load("rpcserverip");
  uint16_t port = atoi(MprpcApplication::getInstance().getConfig().Load("rpcserverport").c_str());
  muduo::net::InetAddress address(ip, port);

  // 创建Tcp
  muduo::net::TcpServer server(&eventLoop_, address, "RpcProvider");

  // 绑定连接回调和消息读写回调
  server.setConnectionCallback(std::bind(&RpcProvider::OnConnection, this, std::placeholders::_1));
  server.setMessageCallback(std::bind(&RpcProvider::OnMessag, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

  // 设置muduo库的线程数量
  server.setThreadNum(4);

  // Rpc节点发布的服务全部注册到zk中,让rpc,client可以从zk发现服务
  ZkClient zkCli;
  zkCli.Start();
  for (auto &sp : serviceMap_) {
    std::string servicePath = "/" + sp.first;
    zkCli.Create(servicePath.c_str(), nullptr, 0);
    for (auto &mp : sp.second.methodMap_) {
      std::string methodPath = servicePath + "/" + mp.first;
      char methodPathData[128] = {0};
      sprintf(methodPathData, "%s:%d", ip.c_str(), port);
      // ZOO_EPHEMERAL表示临时节点
      zkCli.Create(methodPath.c_str(), methodPathData, strlen(methodPathData), ZOO_EPHEMERAL);
    }
  }
  std::cout << "RpcProvider start service at ip:" << ip << "\tport:" << port << std::endl;
  // 启动
  server.start();
  eventLoop_.loop();
}

void RpcProvider::OnConnection(const muduo::net::TcpConnectionPtr &conn)
{
}

void RpcProvider::OnMessag(const muduo::net::TcpConnectionPtr &conn, muduo::net::Buffer *buffer, muduo::Timestamp)
{
  std::string recvBuf = buffer->retrieveAllAsString();

  // 从字符流中读取前四个字节的内容(int)
  uint32_t headerSize = 0;
  recvBuf.copy((char *)&headerSize, 4, 0);

  // 根据headerSize读取数据头的原始字节流   反序列化数据的到rpc请求的详细信息
  std::string rpcHeaderStr = recvBuf.substr(4, headerSize);

  mprpc::RpcHeader rpcHeader;
  std::string serviceName;
  std::string methodName;
  uint32_t argsSize;

  if (!rpcHeader.ParseFromString(rpcHeaderStr)) {
    // 数据头反序列化失败
    std::cout << "rpcHeaderStr: " << rpcHeaderStr << "parse error!" << std::endl;
    return;
  }
  // 数据头反序列化成功
  serviceName = rpcHeader.servicename();
  methodName = rpcHeader.methodname();
  argsSize = rpcHeader.argssize();

  std::string argsStr = recvBuf.substr(4 + headerSize, argsSize);

  std::cout << "=====================================================" << std::endl;
  std::cout << "headerSize" << headerSize << std::endl;
  std::cout << "rpcHeaderStr" << rpcHeaderStr << std::endl;
  std::cout << "serviceName" << serviceName << std::endl;
  std::cout << "methodName" << methodName << std::endl;
  std::cout << "argsStr" << argsStr << std::endl;
  std::cout << "=====================================================" << std::endl;

  // 获取service对象和method对象
  auto it = serviceMap_.find(serviceName);
  if (it == serviceMap_.end()) {
    std::cout << serviceName << "is not exist!" << std::endl;
    return;
  }

  auto mit = it->second.methodMap_.find(methodName);
  if (mit == it->second.methodMap_.end()) {
    std::cout << serviceName << " : " << methodName << " is not exist!" << std::endl;
    return;
  }

  // 获取service对象
  google::protobuf::Service *service = it->second.service_;
  // 获取method对象
  const google::protobuf::MethodDescriptor *method = mit->second;

  // 生成rpc方法调用的请求request和响应response参数
  google::protobuf::Message *request = service->GetRequestPrototype(method).New();
  if (!request->ParseFromString(argsStr)) {
    std::cout << "request parse error! content : " << argsStr << std::endl;
  }
  google::protobuf::Message *response = service->GetResponsePrototype(method).New();
  // Closure回调函数
  google::protobuf::Closure *done = google::protobuf::NewCallback<RpcProvider, const muduo::net::TcpConnectionPtr &, google::protobuf::Message *>(this, &RpcProvider::SendRpcResponse, conn, response);

  // 在框架上根据远端Rpc请求,调用当前Rpc节点上发布的方法
  // new UserService().Login(controller, request, response, done)

  service->CallMethod(method, nullptr, request, response, done);
}

void RpcProvider::SendRpcResponse(const muduo::net::TcpConnectionPtr &conn, google::protobuf::Message *response)
{
  std::string responeStr;
  if (response->SerializeToString(&responeStr)) {
    // 序列化后,通过网络吧Rpc方法执行结果发送方给Rpc调用方
    conn->send(responeStr);
  }
  else {
    std::cout << "serialize responseStr error!" << std::endl;
  }
  // 断开连接
  conn->shutdown();
}