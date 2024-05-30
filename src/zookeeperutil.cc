#include "zookeeperutil.h"
#include "mprpcapplication.h"

#include <iostream>
#include <semaphore.h>

//全局 watcher观察器
void globalWatcher(zhandle_t *zh, int type, int state, const char *path, void *watcherCtx)
{
  if (type == ZOO_SESSION_EVENT) {
    if (state == ZOO_CONNECTED_STATE) {
      sem_t *sem = (sem_t *)zoo_get_context(zh);
      sem_post(sem);
    }
  }
}

ZkClient::ZkClient() : zhandle_(nullptr)
{
}

ZkClient::~ZkClient()
{
  if (zhandle_ != nullptr)
    zookeeper_close(zhandle_);
}

void ZkClient::Start()
{
  std::string host = MprpcApplication::getInstance().getConfig().Load("zookeeperip");
  std::string port = MprpcApplication::getInstance().getConfig().Load("zookeeperport");
  std::string connstr = host + ":" + port;

  zhandle_ = zookeeper_init(connstr.c_str(), globalWatcher, 3000, nullptr, nullptr, 0);
  if (zhandle_ == nullptr) {
    std::cout << "zookeeper_init error!" << std::endl;
    exit(EXIT_FAILURE);
  }

  sem_t sem;
  sem_init(&sem, 0, 0);
  zoo_set_context(zhandle_, &sem);

  sem_wait(&sem);
  std::cout << "zookeeper_init success!" << std::endl;
}

void ZkClient::Create(const char *path, const char *data, int datalen, int state)
{
  char path_buffer[128];
  int bufferlen = sizeof(path_buffer);
  int flag;
  // 先判断path表示的znode节点是否存在，如果存在，就不再重复创建了
  flag = zoo_exists(zhandle_, path, 0, nullptr);
  if (ZNONODE == flag) // 表示path的znode节点不存在
  {
    // 创建指定path的znode节点了
    flag = zoo_create(zhandle_, path, data, datalen, &ZOO_OPEN_ACL_UNSAFE, state, path_buffer, bufferlen);
    if (flag == ZOK) {
      std::cout << "znode create success... path:" << path << std::endl;
    }
    else {
      std::cout << "flag:" << flag << std::endl;
      std::cout << "znode create error... path:" << path << std::endl;
      exit(EXIT_FAILURE);
    }
  }
}

std::string ZkClient::GetData(const char *path)
{
  char buffer[64];
  int bufferlen = sizeof(buffer);
  int flag = zoo_get(zhandle_, path, 0, buffer, &bufferlen, nullptr);
  if (flag != ZOK) {
    std::cout << "get znode error... path:" << path << std::endl;
    return "";
  }
  else {
    return buffer;
  }
}
