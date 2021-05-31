#ifndef EXT2_H
#define EXT2_H
#include "define.h"

#include "DiskDriver.h"
#include "BufferCache.h"
#include "Tools.h"


class DirectoryEntry
{
	/* static members */
public:
	static const int DIRSIZ = 28;	/* 目录项中路径部分的最大字符串长度 */

	/* Functions */
public:
	/* Constructors */
	//DirectoryEntry(int m_ino,char *name);
	DirectoryEntry();
	/* Destructors */
	~DirectoryEntry();

	/* Members */
public:
	int m_ino;		/* 目录项中Inode编号部分 */
	char m_name[DIRSIZ];	/* 目录项中路径名部分 */
};


/**
 * 缓存最近使用过的目录项，减少访问磁盘的次数
 */

class DirectoryCache
{

private:
  DirectoryEntry directoryEntryCacheArea[DIRECTORY_ENTRY_CACHE_SIZE];
  Bitmap directoryEntryCacheBitmap;

public:
  DirectoryCache();
  InodeId findInodeIdByPath(Path path); //根据filepath来查找，如果有的话，返回该目录的inode号(若没有返回-1)
};


#define ROOT_INODE_ID 1

class Ext2
{
private:
  //SuperBlock *p_superBlock;
  //磁盘SuperBlock的内存映像的指针。
  //因为神奇的mmap，这里可以用这个指针指向虚拟磁盘的superblock
  //InodePool *p_InodePool;     //DiskInode池
  BufferCache *p_bufferCache; //绑定的BufferCache,ext2不直接和DiskDriver打交道，透过这个缓存层
  Ext2_Status ext2_status = Ext2_UNINITIALIZED;

public:
  void format(); //格式化
  
  int unregisterFs();
  
  Ext2_Status getExt2Status();
  Ext2_Status setExt2Status(Ext2_Status ext2_status);

  int setBufferCache(BufferCache *p_bufferCache);
  int allocNewInode(); //分配一个新的inode
  DiskInode getDiskInodeByNum(int inodeID);
  void updateDiskInode(int inodeID, DiskInode diskInode);

  InodeId locateInode(const Path& path);
  InodeId locateDir(const Path& path);
  InodeId getInodeIdInDir(InodeId dirInodeId, FileName fileName);

  int bmap(int inodeNum, int logicBlockNum); //文件中的地址映射。查混合索引表，确定物理块号。
  //逻辑块号bn=u_offset/512
  void loadSuperBlock(SuperBlock &superBlock);
};
#endif