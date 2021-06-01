#ifndef OS_MAIN_DEFINE 
#define OS_MAIN_DEFINE
//#define IS_DEBUG //调试状态

#include <iostream>
#include <iomanip>
#include <vector>
#include <stdio.h>
#include <cstring>   
#include <string>     
#include <stdint.h>  
#include <fcntl.h>    
#include <unistd.h>   
//#define WINDOWS

#if defined(WINDOWS) 
#include <sys/mman.h>
#else
#include <sys/mman.h>
#endif


#include <sys/stat.h> 
#include <stdlib.h>  
#include <time.h>     
#include <string>


#define DISK_BLOCK_SIZE 4096                         
#define DISK_SIZE (64 * 1024 * 1024)                 
#define DISK_BLOCK_NUM (DISK_SIZE / DISK_BLOCK_SIZE) 
#define DISK_IMG_DIR "./1851447.img"
#define BITMAP_PERBLOCK_SIZE 8
#define BUFFER_CACHE_NUM 20
#define DISKINODE_SIZE 64
#define INODE_SIZE 64
#define MAX_BITMAP_ELEM_NUM DISK_BLOCK_NUM                   //这个Bitmap静态改造的一部分，原本的bitmap是动态申请的，但是放到磁盘很难办，于是去一个最大值
#define MAX_INODE_NUM (2 * DISK_BLOCK_SIZE / DISKINODE_SIZE) //用两块磁盘块存放inode，表示的是inode的最大数量，不是最大序号
#define MAX_PATH_LEVEL 10                                    //最大目录层次
#define MAX_FILENAME_LEN 28                                  //最长文件名
#define INODE_CACHE_SIZE 128                                 //系统可以缓存这么多inode
#define DIRECTORY_ENTRY_CACHE_SIZE 128                       //系统可以缓存这么多目录项
#define MIXED_ADDR_TABLE_SIZE (10 * sizeof(int))             //混合索引表数组所占空间的大小
#define OK 0
#define ERROR_OFR -4
#define ERROR_NOTSPEC -1 //并不想指明哪一种错误，但是是错误
#define ERROR_CANCEL -2
#define ERROR_LBN_OVERFLOW -5 //文件的逻辑块号大小溢出
#define ERROR_PATH_NFOUND -6
#define ERROR_OUTOF_INODE -7
#define ERROR_OPEN_ILLEGAL -8
#define ERROR_OUTOF_OPENFILE -9
#define ERROR_OUTOF_FILEFD -10
#define ERROR_FILENAME_EXSIST -11
#define ERROR_CLOSE_FAIL -12
#define ERROR_OUTOF_BLOCK -13
#define ERROR_DELETE_FAIL -14
#define ERROR_NO_FOLDER_EXSIT -15

typedef int FileFd;  //文件句柄，实际上就是一个int
typedef int InodeId; //inode号，实际上是一个int
typedef int BlkId;  //扇区号

typedef const char *FileName; //文件名

enum VFS_Status {
  VFS_UNINITIALIZED,
  VFS_NOFORM,
  VFS_READY
}; //未初始化，完成挂载但是未格式化，挂载且格式化（或事先有格式）

enum INSTRUCT {
    ERROR_INST = -1,
    MOUNT,
    UNMOUNT,
    FORMAT,
    CD,
    LS,
    RM,
    MKDIR,
    TOUCH,
    CLEAR,
    HELP,
    EXIT,
    VERSION,
    STORE,
    WITHDRAW,
    RMDIR,
    FOPEN,
    FCREAT,
    FREAD,
    FWRITE,
    FCLOSE,
    FSEEK,
    NOHUP
};



const int INST_NUM = 23;
//NOTE 注意，如果改了上面的枚举类型，那么下面的这个数字也需要相应修改

static const char *instructStr[]{
    "error", //实际上会从下表1开始查找，这个"error"是为了占位置
    "mount",
    "unmount",
    "format",
    "cd",
    "ls",
    "rm",
    "mkdir",
    "touch",
    "clear",
    "help",
    "exit",
    "version",
    "store",
    "withdraw",
    "rmdir",
    "fopen",
    "fcreat",
    "fread",
    "fwrite",
    "fclose",
    "fseek",
    ""
    };

enum FileType {
    NORMAL_FILE,
    DIRECTORY,
    DEVICE
};

#endif
