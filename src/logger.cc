#include "logger.h"

#include <iostream>
#include <time.h>

Logger &Logger::GetInstance()
{
  static Logger logger;
  return logger;
}

Logger::Logger()
{
  // start write thread
  std::thread writeLoTask([&]() {
    for (;;) {
      time_t now = time(nullptr);
      tm *nowtm = localtime(&now);

      char fileName[128];
      sprintf(fileName, "%d-%d-%d-log.txt", nowtm->tm_year + 1900, nowtm->tm_mon + 1, nowtm->tm_mday);

      FILE *pf = fopen(fileName, "a+");
      if (pf == nullptr) {
        std::cout << "logger file : " << fileName << " open error!" << std::endl;
        exit(EXIT_FAILURE);
      }

      std::string msg = lockQueue_.Pop();

      char timeBuf[128] = {0};
      sprintf(timeBuf, "%d:%d:%d => [%s] ", nowtm->tm_hour, nowtm->tm_min, nowtm->tm_sec, (logLevel_ == INFO ? "info" : "error"));
      msg.insert(0, timeBuf);
      msg.append("\n");

      fputs(msg.c_str(), pf);
      fclose(pf);
    }
  });

  writeLoTask.detach();
}

void Logger::SetLogLevel(LogLevel level)
{
  logLevel_ = level;
}

void Logger::Log(std::string msg)
{
  lockQueue_.Push(msg);
}