#pragma once

#include "lockqueue.h"

#define LOG_INFO(logMsgFormat, ...)                 \
  do {                                              \
    Logger &logger = Logger::GetInstance();         \
    logger.SetLogLevel(INFO);                       \
    char c[1024] = {0};                             \
    snprintf(c, 1024, logMsgFormat, ##__VA_ARGS__); \
    logger.Log(c);                                  \
  }                                                 \
  while (0);

#define LOG_ERR(logMsgFormat, ...)                  \
  do {                                              \
    Logger &logger = Logger::GetInstance();         \
    logger.SetLogLevel(ERROR);                      \
    char c[1024] = {0};                             \
    snprintf(c, 1024, logMsgFormat, ##__VA_ARGS__); \
    logger.Log(c);                                  \
  }                                                 \
  while (0);

enum LogLevel
{
  INFO,
  ERROR,
};

// MpRpc提供日志
class Logger
{
public:
  static Logger &GetInstance();

  void SetLogLevel(LogLevel level);
  void Log(std::string msg);

private:
  Logger();
  Logger(const Logger &) = delete;
  Logger(Logger &&) = delete;

private:
  int logLevel_;                     // 日志等级
  LockQueue<std::string> lockQueue_; // 日志缓冲队列
};