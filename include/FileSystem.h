#ifndef EXT2_H
#define EXT2_H
#include "define.h"

#include "DiskDriver.h"
#include "BufferCache.h"
#include "Tools.h"


class DirectoryEntry{
public:
	static const int DIRSIZ = 28;	/* 目录项中路径部分的最大字符串长度 */
public:
	DirectoryEntry();
	~DirectoryEntry();
public:
	int m_ino;		/* 目录项中Inode编号部分 */
	char m_name[DIRSIZ];	/* 目录项中路径名部分 */
};


#define ROOT_INODE_ID 1

class VFS{
private:
  BufferCache *p_bufferCache; //绑定的BufferCache,ext2不直接和DiskDriver打交道，透过这个缓存层
  VFS_Status ext2_status = VFS_UNINITIALIZED;

public:
  void format(); //格式化
  
  VFS_Status getExt2Status();
  VFS_Status setExt2Status(VFS_Status ext2_status);

  int setBufferCache(BufferCache *p_bufferCache);
  DiskInode getDiskInodeById(int inode_id);
  void writeBackDiskInode(int inode_id, Inode inode);

  InodeId locateInode(const myPath& path);
  InodeId locateParDir(const myPath& path);
  InodeId getInodeIdInDir(InodeId par_inode_id, FileName fileName);

  //逻辑块号bn=u_offset/512
  void loadSuperBlock(SuperBlock &superBlock);
};
#endif