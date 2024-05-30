#include "mprpcchannel.h"
#include "mprpcapplication.h"
#include "rpcheader.pb.h"
#include "zookeeperutil.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

void MprpcChannel::CallMethod(const google::protobuf::MethodDescriptor *method,
                              google::protobuf::RpcController *controller,
                              const google::protobuf::Message *request,
                              google::protobuf::Message *response,
                              google::protobuf::Closure *done)
{
  const google::protobuf::ServiceDescriptor *sd = method->service();
  std::string serviceName = sd->name();
  std::string methodName = method->name();

  // 获取参数的序列化字符串长度
  int argsSize = 0;
  std::string argsStr;
  if (!request->SerializeToString(&argsStr)) {
    controller->SetFailed("Serialize request error!");
    return;
  }
  argsSize = argsStr.size();

  // 定义Rpc的请求header
  mprpc::RpcHeader rpcHeader;
  rpcHeader.set_servicename(serviceName);
  rpcHeader.set_methodname(methodName);
  rpcHeader.set_argssize(argsSize);

  uint32_t headerSize = 0;
  std::string rpcHeaderStr;
  if (!rpcHeader.SerializeToString(&rpcHeaderStr)) {
    controller->SetFailed("serialize rpc header error!");
    return;
  }
  headerSize = rpcHeaderStr.size();

  // 组织待发送的rpc请求字符串
  std::string sendRpcStr;
  sendRpcStr.insert(0, std::string((char *)&headerSize, 4));
  sendRpcStr += rpcHeaderStr;
  sendRpcStr += argsStr;

  std::cout << "=====================================================" << std::endl;
  std::cout << "headerSize: " << headerSize << std::endl;
  std::cout << "rpcHeaderStr: " << rpcHeaderStr << std::endl;
  std::cout << "serviceName: " << serviceName << std::endl;
  std::cout << "methodName: " << methodName << std::endl;
  std::cout << "argsStr: " << argsStr << std::endl;
  std::cout << "=====================================================" << std::endl;

  // 使用tcp完成Rpc远程调用
  int clientFd = socket(AF_INET, SOCK_STREAM, 0);
  if (clientFd == -1) {
    char errText[512] = {0};
    sprintf(errText, "create socket error! errno: %d", errno);
    controller->SetFailed(errText);
    return;
  }

  // std::string ip = MprpcApplication::getInstance().getConfig().Load("rpcserverip");
  // uint16_t port = atoi(MprpcApplication::getInstance().getConfig().Load("rpcserverport").c_str());

  ZkClient zkCli;
  zkCli.Start();
  std::string methodPath = "/" + serviceName + "/" + methodName;
  std::string hostData = zkCli.GetData(methodPath.c_str());
  if (hostData == "") {
    controller->SetFailed(methodPath + "is not exist!");
    return;
  }
  int idx = hostData.find(":");
  if (idx == -1) {
    controller->SetFailed(methodPath + " address is invalid!");
    return;
  }

  std::string ip = hostData.substr(0, idx);
  uint16_t port = atoi(hostData.substr(idx + 1, hostData.size() - idx).c_str());

  struct sockaddr_in serviceAddr;
  serviceAddr.sin_family = AF_INET;
  serviceAddr.sin_port = htons(port);
  serviceAddr.sin_addr.s_addr = inet_addr(ip.c_str());
  if (connect(clientFd, (struct sockaddr *)&serviceAddr, sizeof(serviceAddr)) == -1) {
    close(clientFd);
    char errText[512] = {0};
    sprintf(errText, "connect error! errno: %d", errno);
    controller->SetFailed(errText);
    return;
  }

  // 发送Rpc请求
  if (send(clientFd, sendRpcStr.c_str(), sendRpcStr.size(), 0) == -1) {

    close(clientFd);
    char errText[512] = {0};
    sprintf(errText, "send error! errno: %d", errno);
    controller->SetFailed(errText);
    return;
  }

  // 接收Rpc请求响应
  char recvBuf[1024] = {0};
  int recvSize = -1;
  if ((recvSize = recv(clientFd, recvBuf, 1024, 0)) == -1) {

    close(clientFd);
    char errText[512] = {0};
    sprintf(errText, "recv error! errno: %d", errno);
    controller->SetFailed(errText);
    return;
  }

  // std::string responseStr(recvBuf, 0, recvSize);

  // 数据反序列化
  // if (!response->ParseFromString(responseStr))
  if (!response->ParseFromArray(recvBuf, recvSize)) {

    close(clientFd);
    char errText[512] = {0};
    sprintf(errText, "parse error! responseStr: %d", errno);
    controller->SetFailed(errText);
    return;
  }
  close(clientFd);
}