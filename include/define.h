#ifndef OS_MAIN_DEFINE 
#define OS_MAIN_DEFINE
//#define IS_DEBUG //调试状态

#include <iostream>
#include <iomanip>
#include <vector>
#include <unordered_map>

#include <stdio.h>
#include <cstring>   
#include <string>     
#include <stdint.h>  
#include <fcntl.h>    
#include <unistd.h>   
#define WINDOWS

#if defined(WINDOWS) 
#include "/mingw/include/sys/mman.h"
#else
#include <sys/mman.h>
#endif


#include <sys/stat.h> 
#include <stdlib.h>  
#include <time.h>     


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
#define ERROR_NOTSPEC -1 
#define ERROR_CANCEL -2
#define ERROR_LBN_OVERFLOW -5 
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
    FORMAT,
    CD,
    LS,
    RM,
    MKDIR,
    TOUCH,
    CLEAR,
    MAN,
    EXIT,
    STORE,
    LOAD,
    RMDIR,
    FOPEN,
    FCREAT,
    FREAD,
    FWRITE,
    FCLOSE,
    FSEEK,
    NOHUP
};



const int INST_NUM = 20;
//NOTE 注意，如果改了上面的枚举类型，那么下面的这个数字也需要相应修改

static const char *instructStr[]{
    "", 
    "format",
    "cd",
    "ls",
    "rm",
    "mkdir",
    "touch",
    "clear",
    "man",
    "exit",
    "store",
    "load",
    "rmdir",
    "fopen",
    "fcreat",
    "fread",
    "fwrite",
    "fclose",
    "fseek",
    ""
    };

    static std::string m_man =
        "当前指令       :  man  \n"
        "功能简介       :  求助那个男人 \n"
        "使用方法       :  man [指令] \n"
        "可用参数       :  可用[指令] 如下：  \n"
        "                 format       :  格式化镜像 \n"
        "                 exit         :  正确退出 \n"
        "                 touch        :  新建文件（有读写权限） \n"
        "                 mkdir        :  新建目录 \n"
        "                 rm           :  删除文件 \n"
        "                 rmdir        :  删除路径 \n"
        "                 cd           :  进入目录 \n"
        "                 ls           :  列出目录及文件的详细信息 \n"
        "                 fcreat       :  新建文件并打开文件 \n"
        "                 fopen        :  打开文件 \n"
        "                 fread        :  从文件中读出 \n"
        "                 fwrite       :  写入到文件中 \n"
        "                 fseek        :  移动当前指针位置 \n"
        "                 fclose       :  关闭文件 \n"
        "                 store        :  从外部写入整个文件 \n"
        "                 load         :  读出整个文件到外部 \n"

        "[你可以这样使用]:  man format \n"
        ;

    static std::string m_format =
        "当前指令       :  format \n"
        "功能简介       :  清空镜像文件并恢复到初始状态。 \n"
        "使用方法       :  format \n"
        "可用参数       :  无 \n"
        "[你可以这样使用]:  format \n"
        ;

    static std::string m_exit =
        "当前指令       :  exit \n"
        "功能简介       :  退出时，保存现场信息。由于有缓存队列的原因，直接关闭或'CTRL+C'强行断电会导致缓存内容丢失； \n"
        "                 如果您希望退出并保存这时的镜像的内容，请使用exit进行安全退出。\n"
        "使用方法       :  exit \n"
        "可用参数       :  无 \n"
        "[你可以这样使用]:  exit \n"
        ;

    static std::string m_mkdir =
        "当前指令       :  mkdir\n"
        "功能简介       :  新建一个目录。\n"
        "使用方法       :  mkdir <目录名> \n"
        "可用参数       :  <目录名> 可以是相对路径，也可以是绝对路径 \n"
        "[你可以这样使用]:  mkdir dirName \n"
        "                 mkdir ../dirName \n"
        "                 mkdir ../../dirName \n"
        "                 mkdir /dirName \n"
        "                 mkdir /dir1/dirName \n"
        "当目录已存在、或其父目录已不存在时，会产生错误提示 \n"
        ;

    static std::string m_ls =
        "当前指令       :  ls \n"
        "功能简介       :  列出当前目录中包含的文件名或目录名。 \n"
        "                 第一列为文件类型 D 代表目录，F 代表文件 \n"
        "                 第二列为文件名 \n"
        "                 第三列为inode节点号 \n"
        "                 第四列为文件大小，单位为byte，目录大小为0。 \n"
        "使用方法       :  <目录名> 可缺省。可以是相对路径，也可以是绝对路径。默认为当前目录。 \n"
        "[你可以这样使用]:  ls \n"
        "                 ls dirName \n"
        "                 ls ../dirName \n"
        "                 ls ../../dirName \n"
        "                 ls /dirName \n"
        "                 ls /dir1/dirName \n"
        "当对象是文件、或目录路径不存在时，会产生错误提示 \n"
        ;

    static std::string m_cd =
        "当前指令       :  cd \n"
        "功能简介       :  进入目录。 \n"
        "使用方法       :  cd <目录名> \n"
        "可用参数       :  <目录名> 可以是相对路径，也可以是绝对路径 \n"
        "[你可以这样使用]:  cd ../dirName \n"
        "                 cd ../../dirName \n"
        "                 cd /dirName \n"
        "                 cd /dir1/dirName \n"
        "当对象是文件、或目录路径不存在时，会产生错误提示 \n"
        ;

    static std::string m_fcreat =
        "当前指令       :  fcreat \n"
        "功能简介       :  新建并打开文件。同时赋予相应的操作权限 \n"
        "使用方法       :  fcreat <文件名> <选项> \n"
        "可用参数       :  <文件名> 当前目录 \n"
        "                 <选项> -r 只读属性 \n"
        "                 <选项> -w 只写属性 \n"
        "                 <选项> -rw == -r -w 读写属性 \n"
        "[你可以这样使用]:  fcreat newFileName -rw \n"
        "当非法文件名、或非法权限时，会产生错误提示 \n"
        ;

    static std::string m_rm =
        "当前指令       :  rm \n"
        "功能简介       :  删除一个文件。\n"
        "使用方法       :  rm <文件名> \n"
        "可用参数       :  <文件名> 可以包含相对路径或绝对路径 \n"
        "[你可以这样使用]:  rm fileName \n"
        "                 rm ../fileName \n"
        "                 rm ../../fileName \n"
        "                 rm /fileName \n"
        "                 rm /dir1/fileName \n"
        "当非法文件名、目录路径不存在时，会产生错误提示 \n"
        ;

    static std::string m_fopen =
        "当前指令       :  fopen \n"
        "功能简介       :  打开一个文件。若要进行文件的读写必须先open。\n"
        "                 成功打开后，会返回相应的文件描述符fd。\n"
        "使用方法       :  fopen <文件名> <选项> \n" 
        "可用参数       :  <文件名> 当前目录 \n"
        "                 <选项> -r 只读属性 \n"
        "                 <选项> -w 只写属性 \n"
        "                 <选项> -rw == -r -w 读写属性 \n"
        "[你可以这样使用]:  fopen fileName -r \n"
        "当非法文件名过长、或目录路径不存在，会产生错误提示 \n"
        ;

    static std::string m_fclose =
        "当前指令       :  fclose \n"
        "功能简介       :  关闭一个文件。读写完毕后，可以对已经打开的文件进行关闭。 \n"
        "使用方法       :  fclose <fd> \n"
        "可用参数       :  <fd> 文件描述符 \n"
        "[你可以这样使用]:  fclose 1 \n"
        "当出现非法的、或不存在的文件描述符时，会产生错误提示 \n"
        ;

    static std::string m_fseek =
        "当前指令       :  fseek \n"
        "功能简介       :  修改当前打开文件的指针位置。可以进行前后移动 \n"
        "使用方法       :  fseek <fd> <offset> \n"
        "可用参数       :  <fd> fopen返回的文件描述符 \n"
        "                 <offset> 指定从当前位置要进行的偏移量。正数代表后移，负数代表前移。 \n"
        "                          本操作保证offset之后，也不会超过当前文件的范围"
        "[你可以这样使用]:  fseek 1 500\n"
        "当出现非法的、或不存在的文件描述符时，会产生错误提示 \n"
        ;

    static std::string m_fwrite =
        "当前指令       :  fwrite \n"
        "功能简介       :  写入一个已经打开的文件中。可以写入终端输入值，也可以写入外部文件。\n"
        "                 如果是写入外部文件，可以指定写入的大小或是全部写入。\n"
        "                 注意每一次写入都会修改文件指针的位置！！\n"
        "使用方法       :  fwrite <fd> <输入字符> \n"
        "                 fwrite <fd> -d <外部文件名> <写入大小> \n"
        "                 fwrite <fd> -d <外部文件名> -f \n"
        "可用参数       :  <fd> fopen返回的文件描述符 \n"
        "                 <输入字符> 终端输入的字符串 \n"
        "                 -d <外部文件名> 从外部文件写入 \n"
        "                 <写入大小> 指定写入字节数；当大小超过文件大小时，保证写入仍是文件大小。\n"
        "                 -f  全部写入"
        "[你可以这样使用]:  fwrite 1 123asda \n"
        "                 fwrite 1 -d FileName 100 \n"
        "                 fwrite 1 -d FileName -f \n"
        "当出现非法的、或不存在的外部文件、错误的操作权限时，会产生错误提示 \n"
        ;

    static std::string m_fread =
        "当前指令       :  fread \n"
        "功能简介       :  从一个已经打开的文件中读取。可以直接读取并打印到终端，也可以指定使输出到外部的目标文件。 \n"
        "                 如果是读出并保存到外部文件，可以指定读出的大小或是全部读出。\n"
        "                 注意每一次读出都会修改文件指针的位置！！\n   "
        "使用方法       :  fread <fd> <读出大小>  \n"
        "                 fread <fd> -f \n"
        "                 fread <fd> -o <外部文件名> <读出大小>\n"
        "                 fread <fd> -o <外部文件名> -f \n"
        "可用参数       :  <fd> fopen返回的文件描述符 \n"
        "                 -o <外部文件名> 输出到外部文件 \n"
        "                 <读出大小> 指定读取字节数 \n"
        "                 -f 全部读出 \n"

        "[你可以这样使用]:  fread 1 100 \n"
        "                 fread 1 -f \n"
        "                 fread 1 -o FileName 100 \n"
        "                 fread 1 -o FileName -f \n"
        "当出现非法的、或不存在的外部文件、错误的操作权限时，会产生错误提示 \n"
        ;

    static std::string m_touch =
        "当前指令       :  touch \n"
        "功能简介       :  创建文件，拥有读写权限。\n   "
        "使用方法       :  touch <文件名>\n" 
        "可用参数       :  <文件名> 在当前目录创建 \n"
        "[你可以这样使用]:  touch a \n"
        ;

    static std::string m_rmdir =
        "当前指令       :  rmdir \n"
        "功能简介       :  删除文件夹。递归删除所有内容。\n"
        "使用方法       :  rmdir <目录名>\n" 
        "可用参数       :  <文件名> 在当前目录创建 \n"
        "[你可以这样使用]:  rmdir folder \n"
        "当操作对象为文件、或不存在的路径时，会产生错误提示 \n"
        ;

    static std::string m_store =
        "当前指令       :  store \n"
        "功能简介       :  整个从外部写入文件。\n   "
        "使用方法       :  store <外部文件名> <内部文件名>\n" 
        "可用参数       :  <外部文件名> 需要写入的文件\n"
        "                 <内部文件名> 内部生成的文件\n"
        "[你可以这样使用]:  store ../asset/readme  read.me \n"
        "当操作对象为不存在的文件时，会产生错误提示 \n"
        ;

    static std::string m_load =
        "当前指令       :  load \n"
        "功能简介       :  从内部整个文件读出到外部。\n   "
        "使用方法       :  load <内部文件名> <外部文件名>\n" 
        "可用参数       :  <内部文件名> 需要读出的文件\n"
        "                 <外部文件名> 外部生成的文件\n"
        "[你可以这样使用]:  load read.me ../asset/readme \n"
        "当操作对象为不存在的文件时，会产生错误提示 \n"
        ;

static std::unordered_map<std::string, std::string*>my_man_map({
        { "format", &m_format },
        { "exit", &m_exit },
        { "mkdir", &m_mkdir },
        { "cd", &m_cd },
        { "ls", &m_ls },
        { "fcreat", &m_fcreat },
        { "rm", &m_rm },
        { "fopen", &m_fopen },
        { "fclose", &m_fclose },
        { "fseek", &m_fseek },
        { "fwrite", &m_fwrite },
        { "fread", &m_fread },
        { "store", &m_store },
        { "load", &m_load },
        { "touch", &m_touch },
        { "rmdir", &m_rmdir },
        { "man", &m_man },

        });

enum FileType {
    NORMAL_FILE,
    DIRECTORY,
    DEVICE
};

#endif
