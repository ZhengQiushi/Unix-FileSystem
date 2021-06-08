#ifndef TOOLS_H
#define TOOLS_H
#include "define.h"

class myPath{
public:
  std::vector<std::string> path;
  bool from_root;
  //int level;
  myPath();
  myPath(const myPath &full_path);
  explicit myPath(const std::string& raw_path); //explicit关键字避免隐式类型转换
  std::string toString() const;
  const std::string getInodeName() const;
  int getLevel() const;
  std::string my_pop();
};

std::string gengerString(int n);

bool genTestsFile(std::string res_dir, const int test_str_len = 800);



#endif