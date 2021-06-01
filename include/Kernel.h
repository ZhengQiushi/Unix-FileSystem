#ifndef KERNEL_H
#define KERNEL_H
#include "define.h"

#include "FileSystem.h"
#include "DiskDriver.h"
#include "BufferCache.h"



class File{
	/**
	* @brief 打开文件控制块File类.记录了进程打开文件的读、写请求类型，文件读写位置等等。
	*/
public:
	enum FileFlags{
		FREAD = 0x1,  /* 读请求类型 */
		FWRITE = 0x2, /* 写请求类型 */
		FPIPE = 0x4   /* 管道类型 */
	};
public:
	File();
	~File();
	unsigned int f_flag; /* 对打开文件的读、写操作要求 */
	int f_count;		 /* 当前引用该文件控制块的进程数量 */
	Inode*	f_inode;			/* 指向打开文件的内存Inode指针 */
	InodeId f_inode_id;
	int f_offset; /* 文件读写位置指针 */
};


class OpenFiles{
	/**
	* @brief 进程打开文件描述符表(OpenFiles)的定义
	* 进程的u结构中包含OpenFiles类的一个对象，维护了当前进程的所有打开文件。
	*/
public:
	static const int NOFILES = 15; /* 进程允许打开的最大文件数 */

public:
	OpenFiles();
	~OpenFiles();

	int AllocFreeSlot();
	File *GetF(int fd);
	void SetF(int fd, File *pFile);
private:
	File *ProcessOpenFileTable[NOFILES]; /* File对象的指针数组，指向系统打开文件表中的File对象 */
};


class IOParameter{
	/**
	* @brief 文件I/O的参数类
	* 对文件读、写时需用到的读、写偏移量、字节数以及目标区域首地址参数。
	*/
public:
	IOParameter();
	~IOParameter();
public:
	unsigned char *m_Base; /* 当前读、写用户目标区域的首地址 */
	int m_Offset;		   /* 当前读、写文件的字节偏移量 */
	int m_Count;		   /* 当前还剩余的读、写字节数量 */
};


class OpenFileTable{
public:
    static const int NFILE = 100; /* 打开文件控制块File结构的数量 */
public:
    OpenFileTable();
    ~OpenFileTable();

    File *FAlloc();
    void CloseF(File *pFile);
public:
    File m_File[NFILE]; /* 系统打开文件表，为所有进程共享，进程打开文件描述符表
								中包含指向打开文件表中对应File结构的指针。*/
};




class User{
	/**
	 * @brief User类是从Unix v6++中copy过来的，只保留与文件相关的数据结构
	 */
public:
	User();
	Inode *u_cdir; /* 指向当前目录的Inode指针 */
	Inode *u_pdir; /* 指向父目录的Inode指针 */

	DirectoryEntry u_dent; /* 当前目录的目录项 */

	InodeId curDirInodeId;

	/* 进程的用户标识 */
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



class Kernel{
	/**
	 * @brief 核心成员，是所有部件的中心 
	 */
private:
  Kernel();
  /* 静态成员 */
  static Kernel kernelInstance;

  /* 六大核心 */
  InodeCache inodeCache;   
  SuperBlock superBlock;   
  BufferCache bufferCache; 
  DiskDriver diskDriver;   
  VFS fileSystem;          
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

	void my_man(std::string cur_inst);

  int mkdir(const char *dirName); //返回分配的Inode编号
  int cd(const char *dirName);    //返回进入的dir的Inode
  void ls(const char *dirName);
  void ls(InodeId dirInodeID);
  int open(const myPath& path, int mode);
  int close(int fd);
  int read(int fd, uint8_t *content, int length);  //用户层面，文件必须先打开才可读
  int write(int fd, uint8_t *content, int length); //用户层面，文件必须先打开才可写
  bool eof(FileFd fd);
};

#endif
