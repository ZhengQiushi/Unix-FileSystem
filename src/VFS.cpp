#include "VFS.h"
#include "Logcat.h"
#include "Kernel.h"

#include "SuperBlockCache.h"
#include "SuperBlock.h"
#include "Kernel.h"
#include "OpenFileTable.h"
#include "Kernel.h"

#include "InodeCache.h"
#include "Kernel.h"
#include "Inode.h"
#include "Kernel.h"

#include "File.h"

#include "DirectoryEntry.h"
#include "DirectoryCache.h"


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
	const User &u = Kernel::instance()->getUser();

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
	const User &u = Kernel::instance()->getUser();

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
            phyBlkno = Kernel::instance()->getSuperBlockCache().balloc();
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
            int newBlkNum = Kernel::instance()->getSuperBlockCache().balloc();
            /* 分配一空闲盘块存放间接索引表 */
            if (newBlkNum<0)
            {
                return ERROR_OUTOF_BLOCK; /* 分配失败 */
            }
            /* i_addr[index]中记录间接索引表的物理盘块号 */
            this->i_addr[index] = newBlkNum;
            pFirstBuf=Kernel::instance()->getBufferCache().GetBlk(newBlkNum);
        }
        else
        {
            /* 读出存储间接索引表的字符块 */
            pFirstBuf = Kernel::instance()->getBufferCache().Bread(phyBlkno);

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
                BlkNum newBlkNum = Kernel::instance()->getSuperBlockCache().balloc();
                if (newBlkNum<0)
                {
                    /* 分配一次间接索引表磁盘块失败，释放缓存中的二次间接索引表，然后返回 */
                    Kernel::instance()->getSuperBlockCache().bfree(newBlkNum);
                    //bufMgr.Brelse(pFirstBuf);
                    return ERROR_OUTOF_BLOCK;
                }
                /* 将新分配的一次间接索引表磁盘块号，记入二次间接索引表相应项 */
                iTable[index] = newBlkNum;
                pSecondBuf=Kernel::instance()->getBufferCache().GetBlk(newBlkNum);
                /* 将更改后的二次间接索引表延迟写方式输出到磁盘 */
                Kernel::instance()->getBufferCache().Bdwrite(pFirstBuf);
            }
            else
            {
                /* 释放1次间接索引表占用的缓存，并读入2次间接索引表 */
                Kernel::instance()->getBufferCache().Brelse(pFirstBuf);
                pSecondBuf = Kernel::instance()->getBufferCache().Bread(phyBlkno);
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
        if ((phyBlkno = iTable[index]) == 0 && (newBlk3 = Kernel::instance()->getSuperBlockCache().balloc()) >= 0)
        {
            /* 将分配到的文件数据盘块号登记在一次间接索引表中 */
            phyBlkno = newBlk3;
            iTable[index] = phyBlkno;
            /* 将数据盘块、更改后的一次间接索引表用延迟写方式输出到磁盘 */
            pSecondBuf=Kernel::instance()->getBufferCache().GetBlk(newBlk3);
            Kernel::instance()->getBufferCache().Bdwrite(pSecondBuf);
            Kernel::instance()->getBufferCache().Bdwrite(pFirstBuf);
        }
        else
        {
            /* 释放一次间接索引表占用缓存 */
            Kernel::instance()->getBufferCache().Brelse(pFirstBuf);
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

void InodeCache::clearCache()
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
  return &inodeCacheArea[addInodeCache((Kernel::instance()->getExt2().getDiskInodeByNum(inodeID)), inodeID)];

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
      Kernel::instance()->getExt2().updateDiskInode(inodeCacheArea[ramdom_i].i_number, inodeCacheArea[ramdom_i]);
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
        Kernel::instance()->getExt2().updateDiskInode(inodeCacheArea[i].i_number, inodeCacheArea[i]);
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
SuperBlockCache::SuperBlockCache() : disk_block_bitmap(DISK_SIZE / DISK_BLOCK_SIZE)
{
}

/**
 * 分配一个空闲盘块.如果返回-1表示分配失败！
 */
BlkNum SuperBlockCache::balloc()
{
    dirty = true;
    int ret = disk_block_bitmap.getAFreeBitNum();
    if (ret != -1)
    {
        disk_block_bitmap.setBit(ret);
        free_block_bum--;
    }

    return ret;
}

/**
     * 回收一个盘块
     */
void SuperBlockCache::bfree(BlkNum blknum)
{
    dirty = true;
    free_block_bum++;
    disk_block_bitmap.unsetBit(blknum);
}
void SuperBlockCache::bsetOccupy(BlkNum blkNum)
{

    dirty = true;
    if (disk_block_bitmap.getBitStat(blkNum))
    {
    }
    else
    {
        disk_block_bitmap.setBit(blkNum);
        free_block_bum--;
    }
}
void SuperBlockCache::flushBack()
{
    SuperBlock tempSuperBlock;
    tempSuperBlock.disk_block_bitmap = this->disk_block_bitmap;
    tempSuperBlock.free_block_bum = this->free_block_bum;
    tempSuperBlock.free_inode_num = this->free_inode_num;
    tempSuperBlock.total_block_num = this->total_block_num;
    tempSuperBlock.total_inode_num = this->total_inode_num;
    tempSuperBlock.SuperBlockBlockNum = this->SuperBlockBlockNum;
    memcpy(tempSuperBlock.s_inode, this->s_inode, sizeof(this->s_inode));
    Buf *pBuf = Kernel::instance()->getBufferCache().GetBlk(0);
    SuperBlock *p_superBlock = (SuperBlock *)pBuf->b_addr;
    *p_superBlock = tempSuperBlock; //没有动态申请，不用管深浅拷贝
    Kernel::instance()->getBufferCache().Bdwrite(pBuf);

    //下面是硬写入（不经过缓存）
    // DiskBlock *diskMemAddr = Kernel::instance()->getDiskDriver().getDiskMemAddr();
    // SuperBlock *p_superBlock = (SuperBlock *)diskMemAddr;
    // *p_superBlock = tempSuperBlock; //没有动态申请，不用管深浅拷贝
}

InodeId SuperBlockCache::ialloc()
{
    dirty = true;
    if (free_inode_num != 0)
    {
        return s_inode[--free_inode_num];
    }
    else
    {
        return ERROR_OUTOF_INODE;
    }
}
void SuperBlockCache::ifree(InodeId inodeId)
{
    dirty = true;

    s_inode[free_inode_num++] = inodeId;
}


VFS::VFS()
{
}

VFS::~VFS()
{
}
InodeCache* VFS::getInodeCache(){
    return inodeCache;
}
Ext2* VFS::getFilesystem(){
    return p_ext2;
};  
void VFS::mount()
{
    /**
     * 装载磁盘的最上层命令调用函数：
     * 硬盘装载的步骤：
     * ①内存inodeCache初始化
     * ②DiskDriver打开虚拟磁盘img，mmap，进入就绪状态
     * ③装载SuperBlock到VFS的SuperBlock缓存
     * 
     *  */
    inodeCache->clearCache(); //完成①
    if (OK == p_ext2->registerFs())
    {
        Mounted = true;
    } //完成②③
    // 成功的话将Mounted设置为true
}
void VFS::unmount()
{
    if (!Mounted)
    {
        printf("ERROR!磁盘未装载，无需卸载！\n");
    }
    else
    {
        //刷回InodeCache,SuperBlockCache
        inodeCache->flushAllCacheDirtyInode();
        if (superBlockCache->dirty)
        {
            superBlockCache->flushBack();
        }
        p_ext2->unregisterFs();
        Mounted = false;
        //刷回磁盘缓存
    }
}

int VFS::format()
{
    if (!Mounted)
    {
        printf("ERROR!磁盘未装载！\n");
        return ERROR_NOTSPEC;
    }
    else
    {
        switch (p_ext2->getExt2Status())
        {
        case Ext2_UNINITIALIZED:
            printf("ERROR!磁盘装载错误！\n");
            break;
        case Ext2_NOFORM:
            p_ext2->format();
            break;
        case Ext2_READY:
            printf("WARNING!磁盘可能已有数据！确定要格式化吗？\n");
            printf("Press \"y\" for yes, \"n\" for no:");
            char temp_cmd;
            while (temp_cmd = getchar())
            {
                if (temp_cmd == 'y')
                {
                    p_ext2->format();
                    break;
                }
                else if (temp_cmd == 'n')
                {
                    return ERROR_CANCEL;
                    break;
                }
                else
                {
                    printf("\nPress \"y\" for yes, \"n\" for no:");
                }
            }
            break;
        default:
            break;
        }
    }
    return OK;
}
/**
 * 在当前目录下创建文件，
 * 文件名为fileName,返回新创建文件的inodeId
 */
InodeId VFS::createFile(const char *fileName)
{
    InodeId newFileInode = -1;

    //Step0:查看有无同名的，若有则创建失败
    Path path(fileName);
    InodeId checkExsistInodeId = p_ext2->locateInode(path);
    if (checkExsistInodeId > 0)
    {
        return ERROR_FILENAME_EXSIST;
    }

    //Step1:为新文件分配新inode
    newFileInode = superBlockCache->ialloc(); //得到inode号
    if (newFileInode <= 0)
    {
        return newFileInode;
    }
    Inode *p_inode = inodeCache->getInodeByID(newFileInode); //并将这个inode写入inodeCache

    p_inode->i_flag = Inode::IUPD | Inode::IACC;
    p_inode->i_size = 0;
    p_inode->i_mode = 0;
    p_inode->i_nlink = 1;
    p_inode->i_uid = 1;//Kernel::instance()->Getuid();
    p_inode->i_gid = 1;//Kernel::instance()->Getgid();
    p_inode->i_number = newFileInode;
    //Step2:在当前目录文件中写入新的目录项
    Inode *p_dirInode = inodeCache->getInodeByID(Kernel::instance()->getUser().curDirInodeId);
    int blkno = p_dirInode->Bmap(0); //Bmap查物理块号
    Buf *pBuf;
    pBuf = Kernel::instance()->getBufferCache().Bread(blkno);
    DirectoryEntry *p_directoryEntry = (DirectoryEntry *)pBuf->b_addr;

    int i;
    for (i = 0; i < DISK_BLOCK_SIZE / sizeof(DirectoryEntry); i++)
    {
        if ((p_directoryEntry->m_ino == 0)) //找到目录文件中可以见缝插针的地方，填入县创建的inode信息
        {

            p_directoryEntry->m_ino = newFileInode;
            strcpy(p_directoryEntry->m_name, fileName);
            //std::cout << p_directoryEntry->m_name << " ";
            break;
        } //ino==0表示该文件被删除

        p_directoryEntry++;
    }
    if (i == DISK_BLOCK_SIZE / sizeof(DirectoryEntry))
    {
        return ERROR_NOTSPEC;
    }
    Kernel::instance()->getBufferCache().Bdwrite(pBuf);
    //Kernel::instance()->getBufferCache().Brelse(pBuf);

    //Step3:暂时未分配盘块

    return newFileInode;
}

InodeId VFS::deleteDir(const char *dirName)
{
    //目录文件和普通文件要分别处理！
    Path path(dirName);
    InodeId deleteFileInode = p_ext2->locateInode(path);
    if (deleteFileInode < 0)
    {
        return deleteFileInode;
    }

    Inode *p_delete_inode = inodeCache->getInodeByID(deleteFileInode);
    Inode *p_dirInode = inodeCache->getInodeByID(p_ext2->locateDir(path));
    if ((p_delete_inode->i_mode & Inode::IFMT) == Inode::IFDIR) //目录文件
    {
        //递归删除该目录下的所有文件
        int blkno = p_delete_inode->Bmap(0); //Bmap查物理块号
        Buf *pBuf;
        pBuf = Kernel::instance()->getBufferCache().Bread(blkno);
        DirectoryEntry *p_directoryEntry = (DirectoryEntry *)pBuf->b_addr;

        int de_i;
        for (de_i = 0; de_i < DISK_BLOCK_SIZE / sizeof(DirectoryEntry); de_i++)
        {
            if ((p_directoryEntry->m_ino != 0)) //找到目录文件中可以见缝插针的地方，填入县创建的inode信息
            {
                if (!strcmp(p_directoryEntry->m_name, ".") || !strcmp(p_directoryEntry->m_name, ".."))
                {
                    continue;
                }
                else
                {
                    if ((inodeCache->getInodeByID(p_directoryEntry->m_ino)->i_mode & Inode::IFMT) == Inode::IFDIR)
                    {
                        deleteDir(p_directoryEntry->m_name);
                    }
                    else
                    {
                        deleteFile(p_directoryEntry->m_name);
                    }
                }

            } //ino==0表示该文件被删除

            p_directoryEntry++;
        }
        Kernel::instance()->getBufferCache().Bdwrite(pBuf);
        //删除该目录本身
        deleteDirect(dirName);
    }
    else
    {
        Logcat::log("非法删除!");
        return ERROR_DELETE_FAIL;
    }
    return deleteFileInode;
}

/**
 * 删除文件
 */
InodeId VFS::deleteFile(const char *fileName)
{

    //目录文件和普通文件要分别处理！
    Path path(fileName);
    InodeId deleteFileInode = p_ext2->locateInode(path);
    if (deleteFileInode < 0)
    {
        return deleteFileInode;
    }
    Inode *p_delete_inode = inodeCache->getInodeByID(deleteFileInode);
    Inode *p_dirInode = inodeCache->getInodeByID(p_ext2->locateDir(path));
    if ((p_delete_inode->i_mode & Inode::IFMT) == 0) //普通文件
    {

        return deleteDirect(fileName);
    }
    else
    {
        Logcat::log("非法删除!");
        return ERROR_DELETE_FAIL;
    }
}

/**
 * 直接删除
 */
InodeId VFS::deleteDirect(const char *fileName)
{

    Path path(fileName);
    InodeId deleteFileInode = p_ext2->locateInode(path);
    if (deleteFileInode < 0)
    {
        return ERROR_DELETE_FAIL;
    }

    Inode *p_delete_inode = inodeCache->getInodeByID(deleteFileInode);
    Inode *p_dirInode = inodeCache->getInodeByID(p_ext2->locateDir(path));

    BlkNum phyno;
    //Step1 释放盘块
    for (int lbn = 0; (phyno = p_delete_inode->Bmap(lbn)) <= 0; lbn++)
    {
        superBlockCache->bfree(phyno);
    }
    //Step2 删除目录项
    int dirblkno = p_dirInode->Bmap(0); //Bmap查物理块号
    Buf *pBuf;
    pBuf = Kernel::instance()->getBufferCache().Bread(dirblkno);
    DirectoryEntry *p_directoryEntry = (DirectoryEntry *)pBuf->b_addr;

    int de_i;
    for (de_i = 0; de_i < DISK_BLOCK_SIZE / sizeof(DirectoryEntry); de_i++)
    {
        if ((p_directoryEntry->m_ino == p_delete_inode->i_number)) //找到目录文件中可以见缝插针的地方，填入县创建的inode信息
        {

            p_directoryEntry->m_ino = 0;
            break;
        } //ino==0表示该文件被删除

        p_directoryEntry++;
    }
    if (de_i == DISK_BLOCK_SIZE / sizeof(DirectoryEntry))
    {
        return ERROR_DELETE_FAIL;
    }
    Kernel::instance()->getBufferCache().Bdwrite(pBuf);
    //Step3 释放inode
    p_delete_inode->i_flag = 0; //这里是为了不再把删除的inode刷回，只用在superblock中标记inode删除即可
    superBlockCache->ifree(deleteFileInode);
    return deleteFileInode;
}

/**
 * 创建目录
 */
int VFS::mkDir(const char *dirName)
{
    int newDirInodeId = createFile(dirName);
    if (newDirInodeId < 0)
    {
        return ERROR_FILENAME_EXSIST;
    }

    Inode *p_inode = inodeCache->getInodeByID(newDirInodeId);
    p_inode->i_mode = Inode::IFDIR;

    DirectoryEntry tempDirectoryEntry;
    Buf *pBuf;

    BlkNum blkno = p_inode->Bmap(0);
    pBuf = Kernel::instance()->getBufferCache().Bread(blkno);
    DirectoryEntry *p_directoryEntry = (DirectoryEntry *)pBuf->b_addr;

    strcpy(tempDirectoryEntry.m_name, ".");
    tempDirectoryEntry.m_ino = newDirInodeId;
    *p_directoryEntry = tempDirectoryEntry;
    p_directoryEntry++;
    strcpy(tempDirectoryEntry.m_name, "..");
    tempDirectoryEntry.m_ino = Kernel::instance()->getUser().curDirInodeId;
    *p_directoryEntry = tempDirectoryEntry;
    Kernel::instance()->getBufferCache().Bdwrite(pBuf);
    return OK;
}
int VFS::cd(const char *dirName)
{
    Path path(dirName);
    InodeId targetInodeId = p_ext2->locateInode(path);
    if (targetInodeId <= 0)
    {
        Logcat::log("目录查找失败！");
    }
    else if ((inodeCache->getInodeByID(targetInodeId)->i_mode & Inode::IFMT) != Inode::IFDIR)
    {
        Logcat::log("ERROR! cd 命令的参数必须是目录！");
    }
    else
    {
        Kernel::instance()->getUser().curDirInodeId = targetInodeId;
    }

    //df
    return targetInodeId;
}

void VFS::ls(InodeId dirInodeID)
{
    //首先要获得这个inode->访问这个目录文件
    //step1: 检查inodeCache中有没有，有则直接用，没有则向Ext2模块要
    Inode &inode = *inodeCache->getInodeByID(dirInodeID);
    if (inode.i_mode & Inode::IFMT != Inode::IFDIR)
    {
        printf("ERROR! ls的参数只能为空或者目录名！\n");
        return;
    }

    inode.i_flag |= Inode::IACC;
    //Step2：读这个目录文件到缓存块中（可能已经存在于缓存块中,规定目录文件不能超过4096B）
    int blkno = inode.Bmap(0); //Bmap查物理块号
    Buf *pBuf;
    pBuf = Kernel::instance()->getBufferCache().Bread(blkno);
    DirectoryEntry *p_directoryEntry = (DirectoryEntry *)pBuf->b_addr;
    //Step3：访问这个目录文件中的entry，打印出来（同时缓存到dentryCache中）
    //TODO 缓存到dentryCache中
    for (int i = 0; i < DISK_BLOCK_SIZE / sizeof(DirectoryEntry); i++)
    {
        if ((p_directoryEntry->m_ino != 0))
        {
            std::cout << p_directoryEntry->m_name << " ";
        } //ino==0表示该文件被删除

        p_directoryEntry++;
    }
    std::cout << std::endl;
    Kernel::instance()->getBufferCache().Brelse(pBuf);
}

void VFS::ls(const char *dirName)
{
    //首先要根据目录名，确定inode号
    //step1 在DirectoryEntry中查找有没有现成的
    InodeId dirInodeId;

    Path path(dirName); //解析dirName转化为Path对象
    //先查一下directoryCache中有没有存dirName的目录项
    //TODO 先暂时不做VFS层的dentry缓存

    //没有，则向Ext模块要
    dirInodeId = p_ext2->locateInode(path);
    if ((inodeCache->getInodeByID(dirInodeId)->i_mode & Inode::IFMT) == Inode::IFDIR)
    {
        ls(dirInodeId);
    }
    else
    {
        Logcat::log("ERROR!ls指令只能对目录");
    }
}

/**
 * 打开一个普通文件,返回文件的句柄
 */
FileFd VFS::open(Path path, int mode)
{
    FileFd fd;
    //Step1. 查找该文件的inode号
    InodeId openFileInodeId = p_ext2->locateInode(path);

    // 返回-1 说明打开失败
    if(openFileInodeId < 0){
        return -1;
    }
    //Step2. 检查打开合法性(省略了文件本身读写的限定)
    Inode *p_inodeOpenFile = inodeCache->getInodeByID(openFileInodeId);
    if (p_inodeOpenFile->i_mode & Inode::IFMT != 0)
    {
        return ERROR_OPEN_ILLEGAL; //在本程序中，只有普通文件可以open
    }
    p_inodeOpenFile->i_flag |= Inode::IACC;
    //Step3. 分配FILE结构
    File *pFile = Kernel::instance()->m_OpenFileTable.FAlloc();
    if (pFile == NULL)
    {
        //分配失败
        return ERROR_OUTOF_OPENFILE;
    }
    //Step4. 建立钩连关系,u_ofile[]中的一项指向FILE
    User &u = Kernel::instance()->getUser();
    /* 在进程打开文件描述符表中获取一个空闲项 */
    fd = u.u_ofiles.AllocFreeSlot();
    if (fd < 0) /* 如果寻找空闲项失败 */
    {
        return ERROR_OUTOF_FILEFD;
    }
    u.u_ofiles.SetF(fd, pFile);

    pFile->f_flag = mode & (File::FREAD | File::FWRITE);
    
    pFile->f_inode_id = openFileInodeId; //NOTE 这里有没有问题？如果inode被替换出内存了呢？
    return fd;
}


int VFS::close(FileFd fd)
{

    User &u = Kernel::instance()->getUser();

    /* 获取打开文件控制块File结构 */
    File *pFile = u.u_ofiles.GetF(fd);
    if (NULL == pFile)
    {
        return ERROR_CLOSE_FAIL;
    }

    /* 释放打开文件描述符fd，递减File结构引用计数 */
    u.u_ofiles.SetF(fd, NULL);
    Kernel::instance()->m_OpenFileTable.CloseF(pFile);
    return OK;
}

/**
 * 从文件fd中读出length字节放到content缓冲区中。
 * 返回读出的字节数，如果fd剩下的字节小于length，则只把剩下的读出
 */
int VFS::read(int fd, uint8_t *content, int length)
{
    //分析：length可能大于、小于、等于盘块的整数倍
    int readByteCount = 0;

    User &u = Kernel::instance()->getUser();
    File *p_file = u.u_ofiles.GetF(fd);
    Inode *p_inode = inodeCache->getInodeByID(p_file->f_inode_id);
    p_inode->i_flag |= Inode::IUPD;
    Buf *pBuf;

    if (length > p_inode->i_size - p_file->f_offset + 1)
    {
        length = p_inode->i_size - p_file->f_offset + 1;
    }

    while (readByteCount < length && p_file->f_offset <= p_inode->i_size) //NOTE 这里是<还是<=再考虑一下
    {
        BlkNum logicBlkno = p_file->f_offset / DISK_BLOCK_SIZE; //逻辑盘块号
        BlkNum phyBlkno = p_inode->Bmap(logicBlkno);            //物理盘块号
        int offsetInBlock = p_file->f_offset % DISK_BLOCK_SIZE; //块内偏移
        pBuf = Kernel::instance()->getBufferCache().Bread(phyBlkno);
        uint8_t *p_buf_byte = (uint8_t *)pBuf->b_addr;
        p_buf_byte += offsetInBlock;
        if (length - readByteCount <= DISK_BLOCK_SIZE - offsetInBlock + 1)
        { //要读大小<=当前盘块剩下的,读需要的大小

            memcpy(content, p_buf_byte, length - readByteCount);
            p_file->f_offset += length - readByteCount;
            readByteCount = length;
            content += length - readByteCount;
            //修改offset
        }
        else
        { //把剩下的全部读出来
            memcpy(content, p_buf_byte, DISK_BLOCK_SIZE - offsetInBlock + 1);
            p_file->f_offset += DISK_BLOCK_SIZE - offsetInBlock + 1;
            readByteCount += DISK_BLOCK_SIZE - offsetInBlock + 1;
            content += DISK_BLOCK_SIZE - offsetInBlock + 1;
            //修改offset
        }
        Kernel::instance()->getBufferCache().Brelse(pBuf);
    }

    return readByteCount;
}
int VFS::write(int fd, uint8_t *content, int length)
{
    //分析：length可能大于、小于、等于盘块的整数倍
    int writeByteCount = 0;

    User &u = Kernel::instance()->getUser();

    File *p_file = u.u_ofiles.GetF(fd);

    Inode *p_inode = inodeCache->getInodeByID(p_file->f_inode_id);
    p_inode->i_flag |= Inode::IUPD;

    Buf *pBuf;
    while (writeByteCount < length) //NOTE 这里是<还是<=再考虑一下
    {
        

        BlkNum logicBlkno = p_file->f_offset / DISK_BLOCK_SIZE; //逻辑盘块号
        if (logicBlkno == 1030)
        {
            printf("暂时停下");
        }
        BlkNum phyBlkno = p_inode->Bmap(logicBlkno);            //物理盘块号

        int offsetInBlock = p_file->f_offset % DISK_BLOCK_SIZE; //块内偏移
        //NOTE:可能要先读后写！！！
        //当写不满一个盘块的时候，就要先读后写
        if (offsetInBlock == 0 && length - writeByteCount >= DISK_BLOCK_SIZE)
        {
            //这种情况不需要先读后写
            pBuf = Kernel::instance()->getBufferCache().GetBlk(phyBlkno);
        }
        else
        {
            //先读后写
            pBuf = Kernel::instance()->getBufferCache().Bread(phyBlkno);
        }

        

        uint8_t *p_buf_byte = (uint8_t *)pBuf->b_addr;
        p_buf_byte += offsetInBlock;
        if (length - writeByteCount <= DISK_BLOCK_SIZE - offsetInBlock + 1)
        { //要读大小<=当前盘块剩下的,读需要的大小
            memcpy(p_buf_byte, content, length - writeByteCount);
            p_file->f_offset += length - writeByteCount;
            writeByteCount = length;
            //修改offset
        }
        else
        { //把剩下的全部读出来
            memcpy(p_buf_byte, content, DISK_BLOCK_SIZE - offsetInBlock + 1);
            p_file->f_offset += DISK_BLOCK_SIZE - offsetInBlock + 1;
            writeByteCount += DISK_BLOCK_SIZE - offsetInBlock + 1;
            //length = 0; /// 单纯为了不死循环...
            //修改offset
        }
        Kernel::instance()->getBufferCache().Bdwrite(pBuf);
    }

    return writeByteCount;
}

/**
 * 判断是否到达文件尾部
 */
bool VFS::eof(FileFd fd)
{
    User &u = Kernel::instance()->getUser();
    File *p_file = u.u_ofiles.GetF(fd);
    Inode *p_inode = inodeCache->getInodeByID(p_file->f_inode_id); //TODO错误处理?
    if (p_file->f_offset == p_inode->i_size + 1)
        return true;
    else
        return false;
}

void VFS::registerExt2(Ext2 *p_ext2)
{
    this->p_ext2 = p_ext2;
}
void VFS::unregisterExt2()
{
}

void VFS::bindSuperBlockCache(SuperBlockCache *superBlockCache)
{
    this->superBlockCache = superBlockCache;
}
void VFS::bindInodeCache(InodeCache *inodeCache)
{
    this->inodeCache = inodeCache;
}
void VFS::bindDirectoryInodeCache(DirectoryCache *directoryCache)
{
    this->directoryCache = directoryCache;
}

bool VFS::isMounted()
{
    return Mounted;
}

// Path VFS::convertPathToAbsolute(Path &path){
//     if(path.from_root){
//         return path;
//     }else{

//     }
// }
