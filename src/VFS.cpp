#include "VFS.h"
#include "Tools.h"
#include "Kernel.h"
#include "DiskDriver.h"

DirectoryCache::DirectoryCache() : directoryEntryCacheBitmap(DIRECTORY_ENTRY_CACHE_SIZE)
{
}

/**
 * 根据filepath来查找，如果有的话，返回该目录的inode号(若没有返回-1)
 */
InodeId DirectoryCache::findInodeIdByPath(Path path)
{
    return -1;
}
/* Constructors */
DirectoryEntry::DirectoryEntry()
{
	m_ino = 0;
}
// DirectoryEntry::DirectoryEntry(int m_ino,char *name){
// this->m_ino=m_ino;
// strcpy(this->m_name,name);
// }

/* Destructors */
DirectoryEntry::~DirectoryEntry()
{
}

/**
 * @comment:本文件的代码主要来自Unix V6++ File.cpp
 * 进行了一些删改
 * */

/*==============================class File===================================*/
File::File()
{
	this->f_count = 0;
	this->f_flag = 0;
	this->f_offset = 0;
	//this->f_inode = NULL;
	this->f_inode_id = 0;
}

File::~File()
{
	//nothing to do here
}

/*==============================class OpenFiles===================================*/
OpenFiles::OpenFiles()
{
}

OpenFiles::~OpenFiles()
{
}

int OpenFiles::AllocFreeSlot()
{
	int i;
	const User &u = Kernel::instance().getUser();

	for (i = 0; i < OpenFiles::NOFILES; i++)
	{
		/* 进程打开文件描述符表中找到空闲项，则返回之 */
		if (this->ProcessOpenFileTable[i] == NULL)
		{
			/* 设置核心栈现场保护区中的EAX寄存器的值，即系统调用返回值 */
			return i;
		}
	}

	return -1;
}

File *OpenFiles::GetF(int fd)
{
	File *pFile;
	const User &u = Kernel::instance().getUser();

	/* 如果打开文件描述符的值超出了范围 */
	if (fd < 0 || fd >= OpenFiles::NOFILES)
	{
		// u.u_error = User::EBADF;
		return NULL;
	}

	pFile = this->ProcessOpenFileTable[fd];
	if (pFile == NULL)
	{
		// u.u_error = User::EBADF;
	}

	return pFile; /* 即使pFile==NULL也返回它，由调用GetF的函数来判断返回值 */
}

void OpenFiles::SetF(int fd, File *pFile)
{
	if (fd < 0 || fd >= OpenFiles::NOFILES)
	{
		return;
	}
	/* 进程打开文件描述符指向系统打开文件表中相应的File结构 */
	this->ProcessOpenFileTable[fd] = pFile;
}

/*==============================class IOParameter===================================*/
IOParameter::IOParameter()
{
	this->m_Base = 0;
	this->m_Count = 0;
	this->m_Offset = 0;
}

IOParameter::~IOParameter()
{
	//nothing to do here
}



Inode::Inode()
{
    // dirty = false;
    // inode_id = 0;
    // memset(i_addr, 0, 10 * sizeof(int));
}

/**
 * 转换构造函数,
 * 将磁盘inode结构转换为内存inode结构
 * NOTE:需要对i_number进行单独的赋值
 */
Inode::Inode(DiskInode d_inode)
{
    this->i_mode = d_inode.d_mode;
    this->i_nlink = d_inode.d_nlink;
    this->i_uid = d_inode.d_uid;
    this->i_gid = d_inode.d_gid;
    this->i_size = d_inode.d_size;
    memcpy(this->i_addr, d_inode.d_addr, sizeof(d_inode.d_addr));
    this->i_flag = 0;
    this->i_count = 0;
    this->i_dev = 0;
    //this->i_number = ? ;s  注意！DISKINODE是没有INODE号这个属性的，一个DISKINODE的号是固定的，可以根据其位置算出来
    this->i_lastr = -1;
}

void Inode::newInode(int flag, int newFileInode){
    this->i_flag = flag;
    this->i_size = 0;
    this->i_mode = 0;
    this->i_nlink = 1;
    this->i_uid = 1;//Kernel::instance().Getuid();
    this->i_gid = 1;//Kernel::instance().Getgid();
    this->i_number = newFileInode;
}
/**
 * 根据混合索引表，用逻辑块号，查出物理盘块号.
 * NOTE 功能不止查。
 * TODO bmap暂时不做
 */
int Inode::Bmap(int lbn)
{

    int phyBlkno; /* 转换后的物理盘块号 */
    int *iTable;  /* 用于访问索引盘块中一次间接、两次间接索引表 */
    int index;
    //User &u = Kernel::Instance().GetUser();

    /**
     * 超出支持的最大文件块数
     */
    if (lbn >= Inode::HUGE_FILE_BLOCK)
    {
        //u.u_error = User::EFBIG;
        return ERROR_LBN_OVERFLOW;
    }

    if (lbn < 6) /* 如果是小型文件，从基本索引表i_addr[0-5]中获得物理盘块号即可 */
    {
        phyBlkno = this->i_addr[lbn];

        /*
    	 * 如果该逻辑块号还没有相应的物理盘块号与之对应，则分配一个物理块。
    	 * 这通常发生在对文件的写入，当写入位置超出文件大小，即对当前
    	 * 文件进行扩充写入，就需要分配额外的磁盘块，并为之建立逻辑块号
    	 * 与物理盘块号之间的映射。
    	 */
        if (phyBlkno == 0)
        {
            phyBlkno = Kernel::instance().getSuperBlockCache().balloc();
            if (phyBlkno == -1)
            {
                //分配失败。可能没有空闲空间了
            }
            else
            {
                //分配盘块成功，这里的superblock已经改过了哟
                //bufMgr.Bdwrite(pFirstBuf);
                //phyBlkno = pFirstBuf->b_blkno;
                /* 将逻辑块号lbn映射到物理盘块号phyBlkno */
                this->i_addr[lbn] = phyBlkno;
                this->i_flag |= Inode::IUPD;
            }
            /*
    		 * 因为后面很可能马上还要用到此处新分配的数据块，所以不急于立刻输出到
    		 * 磁盘上；而是将缓存标记为延迟写方式，这样可以减少系统的I/O操作。
    		 */
        }

        return phyBlkno;
    }
    else /* lbn >= 6 大型、巨型文件 */
    {
        
        Buf *pFirstBuf,*pSecondBuf;
        /* 计算逻辑块号lbn对应i_addr[]中的索引 */

        // if(lbn==1048){
        //     printf("终于等到你");
        // }
        if (lbn < Inode::LARGE_FILE_BLOCK) // 大型文件
        {
            index = (lbn - Inode::SMALL_FILE_BLOCK) / Inode::ADDRESS_PER_INDEX_BLOCK + 6;
        }
        else /* 巨型文件: 长度介于263 - (128 * 128 * 2 + 128 * 2 + 6)个盘块之间 */
        {
            index = (lbn - Inode::LARGE_FILE_BLOCK) / (Inode::ADDRESS_PER_INDEX_BLOCK * Inode::ADDRESS_PER_INDEX_BLOCK) + 8;
        }

        phyBlkno = this->i_addr[index];
        /* 若该项为零，则表示不存在相应的间接索引表块 */
        if (0 == phyBlkno)
        {
            this->i_flag |= Inode::IUPD;
            int newBlkNum = Kernel::instance().getSuperBlockCache().balloc();
            /* 分配一空闲盘块存放间接索引表 */
            if (newBlkNum<0)
            {
                return ERROR_OUTOF_BLOCK; /* 分配失败 */
            }
            /* i_addr[index]中记录间接索引表的物理盘块号 */
            this->i_addr[index] = newBlkNum;
            pFirstBuf=Kernel::instance().getBufferCache().GetBlk(newBlkNum);
        }
        else
        {
            /* 读出存储间接索引表的字符块 */
            pFirstBuf = Kernel::instance().getBufferCache().Bread(phyBlkno);

        }
        /* 获取缓冲区首址 */
        iTable = (int *)pFirstBuf->b_addr;

        if (index >= 8) /* ASSERT: 8 <= index <= 9 */
        {
            /*
    		 * 对于巨型文件的情况，pFirstBuf中是二次间接索引表，
    		 * 还需根据逻辑块号，经由二次间接索引表找到一次间接索引表
    		 */
            index = ((lbn - Inode::LARGE_FILE_BLOCK) / Inode::ADDRESS_PER_INDEX_BLOCK) % Inode::ADDRESS_PER_INDEX_BLOCK;

            /* iTable指向缓存中的二次间接索引表。该项为零，不存在一次间接索引表 */
            phyBlkno = iTable[index];
            if (0 == phyBlkno)
            {
                BlkNum newBlkNum = Kernel::instance().getSuperBlockCache().balloc();
                if (newBlkNum<0)
                {
                    /* 分配一次间接索引表磁盘块失败，释放缓存中的二次间接索引表，然后返回 */
                    Kernel::instance().getSuperBlockCache().bfree(newBlkNum);
                    //bufMgr.Brelse(pFirstBuf);
                    return ERROR_OUTOF_BLOCK;
                }
                /* 将新分配的一次间接索引表磁盘块号，记入二次间接索引表相应项 */
                iTable[index] = newBlkNum;
                pSecondBuf=Kernel::instance().getBufferCache().GetBlk(newBlkNum);
                /* 将更改后的二次间接索引表延迟写方式输出到磁盘 */
                Kernel::instance().getBufferCache().Bdwrite(pFirstBuf);
            }
            else
            {
                /* 释放1次间接索引表占用的缓存，并读入2次间接索引表 */
                Kernel::instance().getBufferCache().Brelse(pFirstBuf);
                pSecondBuf = Kernel::instance().getBufferCache().Bread(phyBlkno);
            }

            pFirstBuf = pSecondBuf;
            /* 令iTable指向一次间接索引表 */
            iTable = (int *)pSecondBuf->b_addr;
        }

        /* 计算逻辑块号lbn最终位于一次间接索引表中的表项序号index */

        if (lbn < Inode::LARGE_FILE_BLOCK)
        {
            index = (lbn - Inode::SMALL_FILE_BLOCK) % Inode::ADDRESS_PER_INDEX_BLOCK;
        }
        else
        {
            index = (lbn - Inode::LARGE_FILE_BLOCK) % Inode::ADDRESS_PER_INDEX_BLOCK;
        }


        int newBlk3;
        if ((phyBlkno = iTable[index]) == 0 && (newBlk3 = Kernel::instance().getSuperBlockCache().balloc()) >= 0)
        {
            /* 将分配到的文件数据盘块号登记在一次间接索引表中 */
            phyBlkno = newBlk3;
            iTable[index] = phyBlkno;
            /* 将数据盘块、更改后的一次间接索引表用延迟写方式输出到磁盘 */
            pSecondBuf=Kernel::instance().getBufferCache().GetBlk(newBlk3);
            Kernel::instance().getBufferCache().Bdwrite(pSecondBuf);
            Kernel::instance().getBufferCache().Bdwrite(pFirstBuf);
        }
        else
        {
            /* 释放一次间接索引表占用缓存 */
            Kernel::instance().getBufferCache().Brelse(pFirstBuf);
        }
        /* 找到预读块对应的物理盘块号，如果获取预读块号需要额外的一次for间接索引块的IO，不合算，放弃 */
        // Inode::rablock = 0;
        // if (index + 1 < Inode::ADDRESS_PER_INDEX_BLOCK)
        // {
        //     Inode::rablock = iTable[index + 1];
        // }
        return phyBlkno;
    }
} //根据逻辑块号查混合索引表，得到物理块号。

void InodeCache::init()
{
  /**
   * 很简单，不需要把InodeCache区覆盖什么的，
   * 只需要清空bitmap
   */
  inodeCacheBitmap.clear();
}

Inode *InodeCache::getInodeByID(int inodeID)
{
  /**
   * 工序：
   * 遍历查找InodeCache区域看有没有inodeID吻合的inode，若有则取出
   * 若没有，先向磁盘取这个inode，放入inodeCacheArea，（可能会出现替换）
   */
  for (int i = 0; i < INODE_CACHE_SIZE; i++)
  {
    if (inodeCacheBitmap.getBitStat(i) && inodeCacheArea[i].i_number == inodeID) //首先需要这个inodeCache是有效的
    {
      return &inodeCacheArea[i];
    }
  }

  //没有在inodeCache中找到，需要从ext要，写入inodeCache
  return &inodeCacheArea[addInodeCache((Kernel::instance().getExt2().getDiskInodeByNum(inodeID)), inodeID)];

} //返回inodeCache块的缓存

/**
 * 将磁盘Inode拷贝到InodeCache中，注意DiskInode和内存Inode数据结构的区别
 * 若InodeCache未满，则直接放入
 * 否则,发生替换.
 * addInodeCache返回值的含义：放入位置的下标
 * 
 */
int InodeCache::addInodeCache(DiskInode inode, InodeId inodeId)
{
  int pos = inodeCacheBitmap.getAFreeBitNum();
  if (pos < 0)
  {
    //空间不够，需要替换某个inode
    //替换哪个呢？暂时采取随机替换的策略。TODO
    srand((unsigned)time(NULL));
    int ramdom_i = (rand() % (INODE_CACHE_SIZE - 10)) + 10;
    //①确定替换的下标。保留前几个inode不替换，因为比较常用

    if ((inodeCacheArea[ramdom_i].i_flag & (Inode::IUPD | Inode::IACC)) != 0)
    {
      Kernel::instance().getExt2().updateDiskInode(inodeCacheArea[ramdom_i].i_number, inodeCacheArea[ramdom_i]);
    }
    //②可能发生脏inode写回

    inodeCacheArea[pos] = Inode(inode);
    inodeCacheArea[pos].i_number = inodeId;
    inodeCacheBitmap.setBit(pos);
    //③用新的inode覆盖掉

    pos = ramdom_i;
  }
  else
  {

    inodeCacheArea[pos] = Inode(inode);
    inodeCacheArea[pos].i_number = inodeId;
    inodeCacheBitmap.setBit(pos);
  }
  return pos;
}

/**
 * 修改指定inodeID的缓存
 */
void InodeCache::replaceInodeCache(DiskInode inode, int replacedInodeID)
{
  //TODO 暂时好像没有需要
}

//TODO 这个函数的动机存疑。
/**
 * 释放某指定inodeID的缓存.
 * 返回值：若为-1表示没找到要释放的
 * 否则，返回值表示被释放的inodeCache的数组下标
 */
int InodeCache::freeInodeCache(int inodeID)
{
  //首先要找到这个inodeID的缓存放在哪个位置
  //然后修改bitmap
  for (int i = 0; i < INODE_CACHE_SIZE; i++)
  {
    if (inodeCacheBitmap.getBitStat(i) && inodeCacheArea[i].i_number == inodeID) //首先需要这个inodeCache是有效的
    {
      inodeCacheBitmap.unsetBit(i);
      return i;
    }
  }

  return -1; //表示没有找到要释放的。
}

/**
 * 刷回所有的Inode缓存回磁盘（可能所谓刷回只是留在缓存中，
 * 所以如果是卸载磁盘的话，刷回缓存是最后做的）
 */
int InodeCache::flushAllCacheDirtyInode()
{
  //遍历inodeCache，查找存在的并且是脏的inode
  for (int i = 0; i < inodeCacheBitmap.getElemNum(); i++)
  {
    if (inodeCacheBitmap.getBitStat(i))
    { //该inode缓存有意义
      if (inodeCacheArea[i].i_flag & (Inode::IUPD | Inode::IACC))
      {
        Kernel::instance().getExt2().updateDiskInode(inodeCacheArea[i].i_number, inodeCacheArea[i]);
      }
    }
  }
  return OK;
}

/*==============================class OpenFileTable===================================*/
/* 系统全局打开文件表对象实例的定义 */
OpenFileTable g_OpenFileTable;

OpenFileTable::OpenFileTable()
{
    //nothing to do here
}

OpenFileTable::~OpenFileTable()
{
    //nothing to do here
}

/*作用：进程打开文件描述符表中找的空闲项  之 下标  写入 u_ar0[EAX]*/
File *OpenFileTable::FAlloc()
{

    for (int i = 0; i < OpenFileTable::NFILE; i++)
    {
        /* f_count==0表示该项空闲 */
        if (this->m_File[i].f_count == 0)
        {
            /* 建立描述符和File结构的勾连关系 */
            //u.u_ofiles.SetF(fd, &this->m_File[i]);
            /* 增加对file结构的引用计数 */
            this->m_File[i].f_count++;
            /* 清空文件读、写位置 */
            this->m_File[i].f_offset = 0;
            return (&this->m_File[i]);
        }
    }

    printf("No Free File Struct\n");
    //u.u_error = User::ENFILE;
    return NULL;
}

void OpenFileTable::CloseF(File *pFile)
{
    Inode *pNode;
    //ProcessManager &procMgr = Kernel::Instance().GetProcessManager();

    if (pFile->f_count <= 1)
    {
        /*
    	 * 如果当前进程是最后一个引用该文件的进程，
    	 * 对特殊块设备、字符设备文件调用相应的关闭函数
    	 */
        // pFile->f_inode->CloseI(pFile->f_flag & File::FWRITE);
        // g_InodeTable.IPut(pFile->f_inode);
        pFile->f_inode_id = 0;
        pFile->f_offset = 0;
    }

    /* 引用当前File的进程数减1 */
    pFile->f_count--;
}


