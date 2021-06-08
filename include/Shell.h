#ifndef SHELL_H
#define SHELL_H
#include "define.h"
#include "Tools.h"
#include "Kernel.h"

#define TTY_BUFFER_SIZE 4096
#define MAX_CMD_LEN 4096
#define MAX_PARAM_NUM 32
#define MAX_SINGLE_PARAM_LEN 128

class Shell
{
  //处理用户输入，解析指令，调用相关的内核函数
private:
  char tty_buffer[TTY_BUFFER_SIZE];
  char split_cmd[MAX_PARAM_NUM][MAX_SINGLE_PARAM_LEN]{};
  int param_num = 0;
  char const *TAG;
  Kernel& my_kernel;

public:
  Shell(Kernel& kernel);
  ~Shell();
  int run();
  void executeCmd();
  void parseCmd();
  INSTRUCT getInstType();
  char *getInstStr();
  char *getParam(int i);
  int getParamAmount();
  int getFileMode(std::string mode);

  void mount();   //挂载磁盘
  void unmount(); //卸载磁盘
  void format();
  void mkdir();
  void touch();
  void rm();
  void rmdir();
  void man();
  void mexit();
  void ls();
  void cd();
  void store();
  void load();
  void clear();
  void creat();
  void open();
  void close();
  void read();
  void write();
  void lseek();
};
#endif