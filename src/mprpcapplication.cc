#include "mprpcapplication.h"
#include "mprpccontroller.h"

#include <unistd.h>
#include <iostream>
#include <string>


MprpcConfig MprpcApplication::config_;

void ShowArgsHelp()
{
  std::cout << "format: command -i <configfile>" << std::endl;
}

void MprpcApplication::Init(int argc, char **argv)
{
  if (argc < 2)
  {
    ShowArgsHelp();
    exit(EXIT_FAILURE);
  }

  int c = 0;
  std::string configFile;
  while ((c = getopt(argc, argv, "i:")) != -1)
  {
    switch (c)
    {
    case 'i':
      configFile = optarg;
      break;
    case '?':
      ShowArgsHelp();
      break;
    case ':':
      std::cout << "nead <configfile>" << std::endl;
      ShowArgsHelp();
      exit(EXIT_FAILURE);
    default:
      break;
    }
  }

  // load config
  config_.LoadConfigFile(configFile.c_str());
}

MprpcApplication &MprpcApplication::getInstance()
{
  static MprpcApplication app;
  return app;
}

MprpcConfig &MprpcApplication::getConfig()
{
  return config_;
}