#ifndef BITMAP_H
#define BITMAP_H
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

/**
 * BitMap
 * 如果是盘块Bitmap，则构造Bitmap(DISK_SIZE/DISK_BLOCK_SIZE/BITMAP_PERBLOCK_SIZE)
 */
class Bitmap{
private:
  uint8_t bitmap[MAX_BITMAP_ELEM_NUM / BITMAP_PERBLOCK_SIZE + 1]{0};
  int bitmapSize;
  //按位操作，又快又省空间
public:
  Bitmap(int elemNum);
  ~Bitmap();
  int setBit(int elemID);
  int unsetBit(int elemID);
  bool isAvai(int elemID);
  int getFreeBitId();
  void clear();
  int getMapSize();
};

#endif