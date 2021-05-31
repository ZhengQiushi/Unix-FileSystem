#ifndef BUFFER_CACHE_H
#define BUFFER_CACHE_H
#include "define.h"
#include "DiskDriver.h"





/**
 * 从V6++中搬过来的类，缓存控制块。
 * 缓存控制块buf定义
 * 记录了相应缓存的使用情况等信息；
 * 同时兼任I/O请求块，记录该缓存相关的I/O请求和执行结果
**/

class Buf
{
public:
    enum BufFlag /* b_flags中标志位 */
    {
        B_WRITE = 0x1,   /* 写操作。将缓存中的信息写到硬盘上去 */
        B_READ = 0x2,    /* 读操作。从盘读取信息到缓存中 */
        B_DONE = 0x4,    /* I/O操作结束 */
        B_ERROR = 0x8,   /* I/O因出错而终止 */
        B_BUSY = 0x10,   /* 相应缓存正在使用中 */
        B_WANTED = 0x20, /* 有进程正在等待使用该buf管理的资源，清B_BUSY标志时，要唤醒这种进程 */
        B_ASYNC = 0x40,  /* 异步I/O，不需要等待其结束 */
        B_DELWRI = 0x80  /* 延迟写，在相应缓存要移做他用时，再将其内容写到相应块设备上 */
    };

public:
    unsigned int b_flags; /* 缓存控制块标志位 */

    int padding; /* 4字节填充，使得b_forw和b_back在Buf类中与Devtab类
							 * 中的字段顺序能够一致，否则强制转换会出错。 */
    /* 缓存控制块队列勾连指针 */
    //有钩自由的，也有钩设备的
    Buf *b_forw;
    Buf *b_back;
    Buf *av_forw;
    Buf *av_back;

    short b_dev;       /* 主、次设备号，其中高8位是主设备号，低8位是次设备号 */
    int b_wcount;      /* 需传送的字节数 */
    DiskBlock *b_addr; /* 指向该缓存控制块所管理的缓冲区的首地址 */
    int b_blkno;       /* 磁盘逻辑块号 */
    int b_error;       /* I/O出错时信息 */
    int b_resid;       /* I/O出错时尚未传送的剩余字节数 */
};

struct DevTab
{
  Buf *b_forw;
};
class BufferCache
{
private:
  Buf bFreeList; //自由缓存队列控制块，将缓存控制块中b_addr指向相应缓冲区首地址。
  //需要设备缓存队列吗？毕竟只有一个块设备
  //DevTab devtab;
  Buf m_Buf[BUFFER_CACHE_NUM];        //缓存控制块数组
  DiskBlock Buffer[BUFFER_CACHE_NUM]; //缓存块空间
  //BufferLruList bufferLruList;
  DiskDriver *diskDriver;

public:
  BufferCache(){
  }
  void init(); //缓存控制块的初始化。
  void setDiskDriver(DiskDriver *diskDriver);
  void unmount();        //unmount的时候需要把脏缓存刷回
  Buf *Bread(int blk_num); //将物理盘块一整块读入diskBlockPool
  void Bwrite(Buf *bp);  //写一个磁盘块
  void Bdwrite(Buf *bp); //延迟写磁盘块
  void Bflush();         //将dev指定设备队列中延迟写的缓存全部输出到磁盘.可能是在卸载磁盘的时候，需要全部刷回
  //void writeBlk(int blk_num, const DiskBlock &contentToWrite); //将内存的一块区域，写入缓冲区（如果不在缓冲区的话，需要先读）
  Buf *GetBlk(int blk_num); /* 申请一块缓存，用于读写设备dev上的字符块blkno。*/
  void Brelse(Buf *bp);   /* 释放缓存控制块buf */
  Buf &GetBFreeList();    //获取自由缓存队列控制块Buf对象引用
  void NotAvail(Buf *bp);
  /* 清空缓冲区内容 */
  void Bclear(Buf *bp);
};

#endif