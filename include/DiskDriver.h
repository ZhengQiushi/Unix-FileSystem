#ifndef DISK_DRIVER_H
#define DISK_DRIVER_H
#include "define.h"
#include "Tools.h"


const int devno = 12;
#define DEF_MODE S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH

class Inode;
class Buf;

class DiskBlock{
  private:
    //数据存放区域，大小为DISK_BLOCK_SIZE个字节
  public:
    DiskBlock(){
      memset(content, 0, DISK_BLOCK_SIZE);
    }
    uint8_t content[DISK_BLOCK_SIZE];
};


class DiskInode{
public:
  DiskInode();
  DiskInode(unsigned int d_mode,int d_nlink,short d_uid,short d_gid,int d_size,int i_addr[10]);
  DiskInode(Inode inode); //转换构造函数
  unsigned int d_mode;
  int d_nlink;
  short d_uid;
  short d_gid;
  int d_size;
  int d_addr[10];         //混合索引表
  int padding[2];
};


class DiskDriver{
private:
  FileFd disk_img_fd; //挂载磁盘文件的句柄
  DiskBlock *disk_mem_addr;
  const char *TAG;

public:
  DiskDriver();
  ~DiskDriver();
  VFS_Status mountImg();                             //安装img磁盘
  void unmount();                                    //卸载磁盘
  DiskBlock *getBlk(int block_num);                   //获得指向块的指针
  void readBlk(int block_num, DiskBlock *dst);        //读取块
  void writeBlk(int block_num, const DiskBlock &blk); //写入块
  DiskBlock *getDiskMemAddr();
};



class Inode
{
private:
  //NONE
public:
  enum INodeFlag
  {
    ILOCK = 0x1,  /* 索引节点上锁 */
    IUPD = 0x2,   /* 内存inode被修改过，需要更新相应外存inode */
    IACC = 0x4,   /* 内存inode被访问过，需要修改最近一次访问时间 */
    IMOUNT = 0x8, /* 内存inode用于挂载子文件系统 */
    IWANT = 0x10, /* 有进程正在等待该内存inode被解锁，清ILOCK标志时，要唤醒这种进程 */
    ITEXT = 0x20  /* 内存inode对应进程图像的正文段 */
  };
  unsigned int i_mode; //文件工作方式信息,通过&操作可以知道文件是属于什么文件类型
  int i_nlink;         //文件联结计数，即该文件在目录树中不同路径名的数量
  short i_uid;
  short i_gid;
  int i_size;     //文件大小
  int i_addr[10]; //混合索引表

  unsigned int i_flag; //状态的标志位，定义见enum INodeFlag
  int i_count;         //引用计数
  short i_dev;
  int i_number; //文件的inode号
  int i_lastr;  //存放最近一次读取文件的逻辑块号，用于判断是否需要预读

  static const unsigned int IALLOC = 0x8000; /* 文件被使用 */
  static const int ADDRESS_PER_INDEX_BLOCK = DISK_BLOCK_SIZE / sizeof(int);
  static const int SMALL_FILE_BLOCK = 6;                                                                                      /* 小型文件：直接索引表最多可寻址的逻辑块号 */
  static const int LARGE_FILE_BLOCK = ADDRESS_PER_INDEX_BLOCK * 2 + 6;                                                        /* 大型文件：经一次间接索引表最多可寻址的逻辑块号 */
  static const int HUGE_FILE_BLOCK = ADDRESS_PER_INDEX_BLOCK * ADDRESS_PER_INDEX_BLOCK * 2 + ADDRESS_PER_INDEX_BLOCK * 2 + 6; /* 巨型文件：经二次间接索引最大可寻址文件逻辑块号 */
  static const unsigned int IFMT = 0x6000;                                                                                    /*文件类型掩码*/
  static const unsigned int IFDIR = 0x4000;                                                                                   //文件类型：目录文件
  static const unsigned int IFCHR = 0x2000;                                                                                   //文件类型：字符设备
  static const unsigned int IFBLK = 0x6000;                                                                                   //块设备特殊类型文件，为0表示常规数据文件

  Inode();                                //构造函数
  Inode(DiskInode d_inode);               //转换构造函数
  Inode(DiskInode d_inode, int i_number); // 
  void copyInode(Buf* pb, int inode_id);

  void newInode(int flag, int inode_num);
  int Bmap(int lbn);                      //根据逻辑块号查混合索引表，得到物理块号。
};


class InodeCache{
  static const int NINODE = 100;
private:
  Inode inode_cache_area[NINODE];

public:
  void init();
  Inode *getInodeByID(int inode_id); //返回inodeCache块的缓存
  int addInodeCache(DiskInode inode, InodeId inode_id);
  int writeBackInode();

  int isLoaded(InodeId inode_id);
  /* 在内存INode表中寻找一个空闲的内存INode */
  int getFreeINode();
};

class SuperBlock{
public:
  SuperBlock();

  int total_block_num;            //总盘块数
  int total_inode_num;            //总inode数

  int free_inode_num;             //空闲inode
  InodeId s_inode[MAX_INODE_NUM]; //空闲inode栈，用于管理inode的分配和回收


  int free_block_bum;             //空闲盘块数
  InodeId s_free[MAX_INODE_NUM];  //空闲磁盘栈，用于管理磁盘块的分配和回收

  int	s_flock;		        // 封锁空闲盘块索引表标志
  int	s_ilock;		        // 封锁空闲INode表标志

  int	s_fmod;			        // 内存中super block副本被修改标志，意味着需要更新外存对应的Super Block
  int	s_ronly;		        // 本文件系统只能读出
  int	s_time;			        // 最近一次更新时间
  int	padding[47];	        // 填充使SuperBlock块大小等于1024字节，占据2个扇区

  BlkId balloc();
  void bfree(BlkId blk_num);
  void ballocCeratin(BlkId blk_num);

  InodeId ialloc();
  void ifree(InodeId inode_id);
  void init();
  void writeBackSuper();
};




#endif