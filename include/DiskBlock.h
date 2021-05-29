#ifndef DISKBLOCK_H
#define DISKBLOCK_H

#include "define.h"
class DiskBlock{
  private:
    
    //数据存放区域，大小为DISK_BLOCK_SIZE个字节
  public:
    uint8_t content[DISK_BLOCK_SIZE];

};

#endif