#ifndef KERNEL_H
#define KERNEL_H
#include "define.h"

#include "FileSystem.h"
#include "DiskDriver.h"
#include "BufferCache.h"


/*
 * 打开文件控制块File类。
 * 该结构记录了进程打开文件
 * 的读、写请求类型，文件读写位置等等。
 */
class File
{
public:
	/* Enumerate */
	enum FileFlags
	{
		FREAD = 0x1,  /* 读请求类型 */
		FWRITE = 0x2, /* 写请求类型 */
		FPIPE = 0x4   /* 管道类型 */
	};

	/* Functions */
public:
	/* Constructors */
	File();
	/* Destructors */
	~File();

	/* Member */
	unsigned int f_flag; /* 对打开文件的读、写操作要求 */
	int f_count;		 /* 当前引用该文件控制块的进程数量 */
	Inode*	f_inode;			/* 指向打开文件的内存Inode指针 */
	InodeId f_inode_id;
	int f_offset; /* 文件读写位置指针 */
};

/*
 * 进程打开文件描述符表(OpenFiles)的定义
 * 进程的u结构中包含OpenFiles类的一个对象，
 * 维护了当前进程的所有打开文件。
 */
class OpenFiles
{
	/* static members */
public:
	static const int NOFILES = 15; /* 进程允许打开的最大文件数 */

	/* Functions */
public:
	/* Constructors */
	OpenFiles();
	/* Destructors */
	~OpenFiles();

	/* 
	 * @comment 进程请求打开文件时，在打开文件描述符表中分配一个空闲表项
	 */
	int AllocFreeSlot();

	/* 
	 * @comment Dup系统调用时复制打开文件描述符表中的描述符
	 */
	int Clone(int fd);

	/* 
	 * @comment 根据用户系统调用提供的文件描述符参数fd，
	 * 找到对应的打开文件控制块File结构
	 */
	File *GetF(int fd);
	/* 
	 * @comment 为已分配到的空闲描述符fd和已分配的打开文件表中
	 * 空闲File对象建立勾连关系
	 */
	void SetF(int fd, File *pFile);

	/* Members */
private:
	File *ProcessOpenFileTable[NOFILES]; /* File对象的指针数组，指向系统打开文件表中的File对象 */
};

/*
 * 文件I/O的参数类
 * 对文件读、写时需用到的读、写偏移量、
 * 字节数以及目标区域首地址参数。
 */
class IOParameter
{
	/* Functions */
public:
	/* Constructors */
	IOParameter();
	/* Destructors */
	~IOParameter();

	/* Members */
public:
	unsigned char *m_Base; /* 当前读、写用户目标区域的首地址 */
	int m_Offset;		   /* 当前读、写文件的字节偏移量 */
	int m_Count;		   /* 当前还剩余的读、写字节数量 */
};


class OpenFileTable
{
public:
    /* static consts */
    //static const int NINODE	= 100;	/* 内存Inode的数量 */
    static const int NFILE = 100; /* 打开文件控制块File结构的数量 */

    /* Functions */
public:
    /* Constructors */
    OpenFileTable();
    /* Destructors */
    ~OpenFileTable();

    // /*
    // * @comment 根据用户系统调用提供的文件描述符参数fd，
    // * 找到对应的打开文件控制块File结构
    // */
    File* GetF(int fd);
    /* 
	 * @comment 在系统打开文件表中分配一个空闲的File结构
	 */
    File *FAlloc();
    /* 
	 * @comment 对打开文件控制块File结构的引用计数f_count减1，
	 * 若引用计数f_count为0，则释放File结构。
	 */
    void CloseF(File *pFile);

    /* Members */
public:
    File m_File[NFILE]; /* 系统打开文件表，为所有进程共享，进程打开文件描述符表
								中包含指向打开文件表中对应File结构的指针。*/
};




class User
{

public:
	/**
	 * @comment:User类是从Unix v6++中copy过来的，只保留与文件相关的数据结构
	 * 
	 */
	User();
	/* 文件系统相关成员 */
	Inode *u_cdir; /* 指向当前目录的Inode指针 */
	Inode *u_pdir; /* 指向父目录的Inode指针 */

	DirectoryEntry u_dent; /* 当前目录的目录项 */
	//char u_dbuf[DirectoryEntry::DIRSIZ]; /* 当前路径分量 */
	//char u_curdir[128];					 /* 当前工作目录完整路径 */
	//Path u_curDir;
	InodeId curDirInodeId;

	/* 进程的用户标识 */
	/* 因为本应用没有多用户的概念，这里写死也无妨 */
	static const short u_uid = 1;  /* 有效用户ID */
	static const short u_gid = 1;  /* 有效组ID */
	static const short u_ruid = 1; /* 真实用户ID */
	static const short u_rgid = 1; /* 真实组ID */

	/* 文件系统相关成员 */
	OpenFiles u_ofiles; /* 进程打开文件描述符表对象 */

	/* 文件I/O操作 */
	IOParameter u_IOParam; /* 记录当前读、写文件的偏移量，用户目标区域和剩余字节数参数 */

	std::string cur_path;

};



class Kernel
{
private:
  Kernel();
  static Kernel kernelInstance;

  InodeCache inodeCache;
  DirectoryCache directoryCache;
  SuperBlock superBlock;

  BufferCache bufferCache;
  DiskDriver diskDriver;
  VFS ext2;

  User my_user;

public:
  OpenFileTable m_OpenFileTable;
  static Kernel& instance();

  VFS &getFileSystem();
  DiskDriver &getDiskDriver();
  BufferCache &getBufferManager();
  SuperBlock &getSuperBlock();
  InodeCache &getInodeCache();
  User &getUser();
  
  

  void initKernel();
  void relsKernel();


  void relseBlock(Inode *delete_inode);

  int format();
  InodeId kernelTouch(const char *fileName); //返回分配的Inode编号
  InodeId createFile(const InodeId &cur_Inode, const InodeId &par_inode, const std::string &node_name);
  InodeId deleteFile(const char *fileName); //删除文件
  InodeId deleteFile(const InodeId &cur_Inode, const InodeId &par_Inode);
  
  InodeId deleteFolder(const char *dirName);
  InodeId deleteObject(const char *fileName);
  InodeId deleteObject(const InodeId &cur_Inode, const InodeId &par_Inode);
  int mkDir(const char *dirName); //返回分配的Inode编号
  int cd(const char *dirName);    //返回进入的dir的Inode
  void ls(const char *dirName);
  void ls(InodeId dirInodeID);
  int open(const myPath& path, int mode);
  int close(int fd);
  int read(int fd, uint8_t *content, int length);  //用户层面，文件必须先打开才可读
  int write(int fd, uint8_t *content, int length); //用户层面，文件必须先打开才可写
  bool eof(FileFd fd);
  
  void unregisterExt2();         //注销加载的文件系统，要刷回脏inode和superblock

  void registerExt2(VFS *ext2); //注册文件系统，载入SuperBlock 
  void bindSuperBlockCache(SuperBlock *SuperBlock);
  void bindInodeCache(InodeCache *inodeCache);
  void bindDirectoryInodeCache(DirectoryCache *directoryCache);

  bool isMounted();
 


};

#endif
