#ifndef BITMAP_H
#define BITMAP_H
#include "define.h"

class TimeHelper
{
public:
    static int getCurTime(); //获取当前时间
};


/**
 * Path是一个路径类，是将原始路径字符串解析构造而来
 */

class Path
{
private:
    char *path_str, *temp_str;
    int sec_len, i_len, l_len;
    //上面都是临时的，外部不要用
public:
    char path[MAX_PATH_LEVEL][MAX_FILENAME_LEN];
    bool from_root;
    int level;
    //Operations:
    Path();
    Path(const Path &full_path);
    explicit Path(const char *raw_path); //explicit关键字避免隐式类型转换
    bool isSingleName() const;
    std::string toString();
    const char *getInodeName() const;
};

class Logcat
{
private:
  enum LOGCAT_STYLE
  {
    DEFAULT
  };

public:
  static void log(const char *str);
  static void log(const char *tag, const char *str);
  static void devlog(const char *str);
  static void devlog(const char *tag, const char *str);
};
std::string gengerString(int n);

bool genTestsFile(std::string res_dir, const int test_str_len = 800);

/**
 * BitMap标记的是盘块的使用情况，不是字节的。
 * //修正上面的，将Bitmap改造为更加通用的，在inode区管理也可以用用
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
  bool getBitStat(int elemID);
  int getAFreeBitNum();
  void clear();
  int getElemNum();
};

#endif