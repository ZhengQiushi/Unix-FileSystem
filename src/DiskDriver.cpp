#include "DiskDriver.h"
#include "Tools.h"
#include "Kernel.h"

/* 
 * 出现的问题：数据块的释放！ 删除多个文件夹后，再重新创建目录文件，虽然可以继续分配
 * 问题分析：
 * 当我们进行rm操作时，只是将node节点还原，此时还并没有清空数据块,数据块的信息的清空是在后续，当这块数据再次被申请时
 * 会被清空！
 */
 
DiskDriver::DiskDriver()
{
   TAG = strdup("DiskDriver");
}
DiskDriver::~DiskDriver()
{
   if (TAG != nullptr)
      delete TAG;
}

VFS_Status DiskDriver::mountImg(){ 
   /**
    * @brief 映射镜像文件，存在就mmap映射，不存在就创建并强制初始化
    * @return VFS_Status
    */

   VFS_Status ret_status = VFS_UNINITIALIZED;
   //打开img文件,如果没有就尝试创建
   disk_img_fd = open(DISK_IMG_DIR, O_RDWR | O_CREAT, DEF_MODE);
   if (disk_img_fd == -1){
      ret_status = VFS_UNINITIALIZED;
      exit(-1);
   }

   /**
    * 新创建的文件，需要做扩容操作，扩大到指定虚拟磁盘大小
    */
   if (lseek(disk_img_fd, 0, SEEK_END) < DISK_SIZE){
      //说明是新创建的文件，需要改变文件的大小
      lseek(disk_img_fd, 0, SEEK_SET);
      ftruncate(disk_img_fd, DISK_SIZE);
      ret_status = VFS_NOFORM;
   }
   else{
      ret_status = VFS_READY;
   }

   /* 将一个文件映射进内存 */
   disk_mem_addr = (DiskBlock *)mmap(nullptr, DISK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, disk_img_fd, 0);
   if (disk_mem_addr == MAP_FAILED){
      #ifdef IS_DEBUG
         Logcat::log("[ERROR]镜像映射失败");
      #endif
      exit(-1);
      ret_status = VFS_UNINITIALIZED;
   }
   else{
      #ifdef IS_DEBUG
         Logcat::log("[INFO]镜像映射成功");
      #endif
   }

   return ret_status;
}

void DiskDriver::unmount(){
   /**
    * brief@ 结束文件的内存映射，关闭磁盘镜像文件。
    */
   if (munmap(disk_mem_addr, DISK_SIZE) == -1){
      #ifdef IS_DEBUG
         Logcat::log(TAG, "Munmap失败！");
      #endif
      return;
   }
   else{
      #ifdef IS_DEBUG
         Logcat::log(TAG, "Munmap成功！");
      #endif
      close(disk_img_fd);
   }
}

DiskBlock *DiskDriver::getBlk(int block_num){
   return disk_mem_addr + block_num;
}
void DiskDriver::readBlk(int block_num, DiskBlock *dst)
{
   memcpy(dst, disk_mem_addr + block_num, DISK_BLOCK_SIZE);
}
void DiskDriver::writeBlk(int block_num, const DiskBlock &blk)
{
   
   memcpy(disk_mem_addr + block_num, &blk, DISK_BLOCK_SIZE);
}


DiskBlock *DiskDriver::getDiskMemAddr()
{
   return disk_mem_addr;
}


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

/* Destructors */
DirectoryEntry::~DirectoryEntry()
{
}


File::File(){
	this->f_count = 0;
	this->f_flag = 0;
	this->f_offset = 0;
	this->f_inode_id = 0;
}

File::~File(){
	//nothing to do here
}

OpenFiles::OpenFiles()
{
}

OpenFiles::~OpenFiles()
{
}

int OpenFiles::AllocFreeSlot(){
	int i;
	const User &u = Kernel::instance().getUser();

	for (i = 0; i < OpenFiles::NOFILES; i++){
		/* 进程打开文件描述符表中找到空闲项，则返回之 */
		if (this->ProcessOpenFileTable[i] == NULL){
			/* 设置核心栈现场保护区中的EAX寄存器的值，即系统调用返回值 */
			return i;
		}
	}

	return -1;
}

File *OpenFiles::GetF(int fd){
	File *pFile;
	const User &u = Kernel::instance().getUser();

	/* 如果打开文件描述符的值超出了范围 */
	if (fd < 0 || fd >= OpenFiles::NOFILES){

		return NULL;
	}

	pFile = this->ProcessOpenFileTable[fd];
	if (pFile == NULL){

	}
  /* 即使pFile==NULL也返回它，由调用GetF的函数来判断返回值 */
	return pFile; 
}

void OpenFiles::SetF(int fd, File *pFile){
	if (fd < 0 || fd >= OpenFiles::NOFILES){
		return;
	}
	/* 进程打开文件描述符指向系统打开文件表中相应的File结构 */
	this->ProcessOpenFileTable[fd] = pFile;
}

IOParameter::IOParameter(){
	this->m_Base = 0;
	this->m_Count = 0;
	this->m_Offset = 0;
}

IOParameter::~IOParameter(){
	//nothing to do here
}



Inode::Inode(){
    // dirty = false;
    // inode_id = 0;
    // memset(i_addr, 0, 10 * sizeof(int));
}


Inode::Inode(DiskInode d_inode){
    /**
     * @brief 转换构造函数,将磁盘inode结构转换为内存inode结构
     *  
     * @note 需要对i_number进行单独的赋值
     */
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
Inode::Inode(DiskInode d_inode, int i_number){
    /**
     * @brief 转换构造函数,将磁盘inode结构转换为内存inode结构 
     * @note 需要对i_number进行单独的赋值
     */
    this->i_mode = d_inode.d_mode;
    this->i_nlink = d_inode.d_nlink;
    this->i_uid = d_inode.d_uid;
    this->i_gid = d_inode.d_gid;
    this->i_size = d_inode.d_size;
    memcpy(this->i_addr, d_inode.d_addr, sizeof(d_inode.d_addr));
    this->i_flag = 0;
    this->i_count = 0;
    this->i_dev = 0;
    this->i_number = i_number; //注意！DISKINODE是没有INODE号这个属性的，一个DISKINODE的号是固定的，可以根据其位置算出来
    this->i_lastr = -1;
}

void Inode::newInode(int flag, int new_file_node){
    this->i_flag = flag;
    this->i_size = 0;
    this->i_mode = 0;
    this->i_nlink = 1;
    this->i_uid = 1;
    this->i_gid = 1;
    this->i_number = new_file_node;
}

int Inode::Bmap(int lbn){
  /**
   * @param lbn 给定的盘块号
   * @brief 根据混合索引表，用逻辑块号，查出物理盘块号
   *        或着查不到，就自己申请新的物理盘块
   */
    /* 转换后的物理盘块号 */
    int phy_blk_id; 
    /* 用于访问索引盘块中一次间接、两次间接索引表 */
    int *inode_link_table;  
    int index;

    /* 超出支持的最大文件块数 */
    if (lbn >= Inode::HUGE_FILE_BLOCK){
        //u.u_error = User::EFBIG;
        return ERROR_LBN_OVERFLOW;
    }

    if (lbn < 6){ 
        /* 如果是小型文件，从基本索引表i_addr[0-5]中获得物理盘块号即可 */
        phy_blk_id = this->i_addr[lbn];
        /*如果该逻辑块号还没有相应的物理盘块号与之对应，则分配一个物理块*/
        if (phy_blk_id == 0){
          
            phy_blk_id = Kernel::instance().getSuperBlock().balloc();
            Buf* first_index_buf=Kernel::instance().getBufferManager().GetBlk(phy_blk_id);
            Kernel::instance().getBufferManager().Bclear(first_index_buf);
            if (phy_blk_id == -1){
                //分配失败。可能没有空闲空间了
            }
            else
            {
            /* 当写入位置超出文件大小，即对当前文件进行扩充写入，就需要分配额外的磁盘块，
             * 并为之建立逻辑块号与物理盘块号之间的映射*/
                Kernel::instance().getBufferManager().Bdwrite(first_index_buf);
                this->i_addr[lbn] = phy_blk_id;
                this->i_flag |= Inode::IUPD;
            }
            /*
             * 因为后面很可能马上还要用到此处新分配的数据块，所以不急于立刻输出到
             * 磁盘上；而是将缓存标记为延迟写方式，这样可以减少系统的I/O操作。
             */

            

        }

        return phy_blk_id;
    }
    else {
        /* lbn >= 6 大型、巨型文件 */
        Buf *first_index_buf,*second_index_buf;
        /* 计算逻辑块号lbn对应i_addr[]中的索引 */
        if (lbn < Inode::LARGE_FILE_BLOCK){ 
          // 大型文件
            index = (lbn - Inode::SMALL_FILE_BLOCK) / Inode::ADDRESS_PER_INDEX_BLOCK + 6;
        }
        else{ 
          /* 巨型文件: 长度介于263 - (128 * 128 * 2 + 128 * 2 + 6)个盘块之间 */
            index = (lbn - Inode::LARGE_FILE_BLOCK) / (Inode::ADDRESS_PER_INDEX_BLOCK * Inode::ADDRESS_PER_INDEX_BLOCK) + 8;
        }

        phy_blk_id = this->i_addr[index];
        /* 若该项为零，则表示不存在相应的间接索引表块 */
        if (0 == phy_blk_id){
            this->i_flag |= Inode::IUPD;
            int new_free_blk_id = Kernel::instance().getSuperBlock().balloc();
            /* 分配一空闲盘块存放间接索引表 */
            if (new_free_blk_id<0){
              /* 分配失败 */
              return ERROR_OUTOF_BLOCK; 
            }
            /* i_addr[index]中记录间接索引表的物理盘块号 */
            this->i_addr[index] = new_free_blk_id;
            first_index_buf=Kernel::instance().getBufferManager().GetBlk(new_free_blk_id);
            Kernel::instance().getBufferManager().Bclear(first_index_buf);
            

        }
        else{
            /* 读出存储间接索引表的字符块 */
            first_index_buf = Kernel::instance().getBufferManager().Bread(phy_blk_id);

        }
        /* 获取缓冲区首址 */
        inode_link_table = (int *)first_index_buf->b_addr;

        if (index >= 8){ /* ASSERT: 8 <= index <= 9 */
          /*对于巨型文件的情况，经由二次间接索引表找到一次间接索引表*/
            index = ((lbn - Inode::LARGE_FILE_BLOCK) / Inode::ADDRESS_PER_INDEX_BLOCK) % Inode::ADDRESS_PER_INDEX_BLOCK;

            /* iTable指向缓存中的二次间接索引表。该项为零，不存在一次间接索引表 */
            phy_blk_id = inode_link_table[index];
            if (0 == phy_blk_id){
                BlkId new_free_blk_id = Kernel::instance().getSuperBlock().balloc();
                if (new_free_blk_id < 0){
                    /* 分配一次间接索引表磁盘块失败，释放缓存中的二次间接索引表，然后返回 */
                    Kernel::instance().getSuperBlock().bfree(new_free_blk_id);
                    //bufMgr.Brelse(first_index_buf);
                    return ERROR_OUTOF_BLOCK;
                }
                /* 将新分配的一次间接索引表磁盘块号，记入二次间接索引表相应项 */
                inode_link_table[index] = new_free_blk_id;
                second_index_buf = Kernel::instance().getBufferManager().GetBlk(new_free_blk_id);
                Kernel::instance().getBufferManager().Bclear(second_index_buf);

                /* 将更改后的二次间接索引表延迟写方式输出到磁盘 */
                Kernel::instance().getBufferManager().Bdwrite(first_index_buf);
            }
            else{
                /* 释放1次间接索引表占用的缓存，并读入2次间接索引表 */
                Kernel::instance().getBufferManager().Brelse(first_index_buf);
                second_index_buf = Kernel::instance().getBufferManager().Bread(phy_blk_id);
            }

            first_index_buf = second_index_buf;
            /* 令iTable指向一次间接索引表 */
            inode_link_table = (int *)second_index_buf->b_addr;
        }


        /* 计算逻辑块号lbn最终位于一次间接索引表中的表项序号index */
        if (lbn < Inode::LARGE_FILE_BLOCK){
            index = (lbn - Inode::SMALL_FILE_BLOCK) % Inode::ADDRESS_PER_INDEX_BLOCK;
        }
        else{
            index = (lbn - Inode::LARGE_FILE_BLOCK) % Inode::ADDRESS_PER_INDEX_BLOCK;
        }


        int new_blk_large = Kernel::instance().getSuperBlock().balloc();
        phy_blk_id = inode_link_table[index];
        if (phy_blk_id  == 0 && new_blk_large >= 0){
            /* 第一块没有写满，所以分配到的文件数据盘块号登记在一次间接索引表中 */
            phy_blk_id = new_blk_large;
            inode_link_table[index] = phy_blk_id;
            /* 将数据盘块、更改后的一次间接索引表用延迟写方式输出到磁盘 */
            second_index_buf = Kernel::instance().getBufferManager().GetBlk(new_blk_large);
            Kernel::instance().getBufferManager().Bclear(second_index_buf);//清空！

            Kernel::instance().getBufferManager().Bdwrite(second_index_buf);
            Kernel::instance().getBufferManager().Bdwrite(first_index_buf);
        }
        else{
            /* 释放一次间接索引表占用缓存 */
            Kernel::instance().getBufferManager().Brelse(first_index_buf);
        }
        return phy_blk_id;
    }
} //根据逻辑块号查混合索引表，得到物理块号。

void InodeCache::init(){
  /**
   * @brief 很简单，不需要把InodeCache区覆盖什么的，只需要清空bitmap
   */
  inodeCacheBitmap.clear();
}

Inode *InodeCache::getInodeByID(int inode_id){
  /**
   * @brief 遍历查找InodeCache区域看有没有inodeID吻合的inode，若有则取出;
   *        若没有，先向磁盘取这个inode，放入inodeCacheArea，（可能会出现替换）
   */
  for (int i = 0; i < INODE_CACHE_SIZE; i++){
    if (inodeCacheBitmap.isAvai(i) && inode_cache_area[i].i_number == inode_id){
      //首先需要这个inodeCache是有效的
      return &inode_cache_area[i];
    }
  }

  //没有在inodeCache中找到，需要从ext要，写入inodeCache
  return &inode_cache_area[addInodeCache((Kernel::instance().getFileSystem().getDiskInodeByNum(inode_id)), inode_id)];

} 
//返回inodeCache块的缓存

int InodeCache::addInodeCache(DiskInode inode, InodeId inode_id){
  /**
   * @brief 将磁盘Inode拷贝到InodeCache
   *        若InodeCache未满，则直接放入;否则,发生替换.
   * @return 放入位置的下标 
   */
  int cur_free_map_pos = inodeCacheBitmap.getFreeBitId();
  //注意DiskInode和内存Inode数据结构的区别
  if (cur_free_map_pos < 0){
    //空间不够，需要替换某个inode,随机替换
    srand((unsigned)time(NULL));
    //确定替换的下标
    int replace_index = (rand() % (INODE_CACHE_SIZE - 10)) + 10; // 保留前几个inode不替换，因为比较常用
    if ((inode_cache_area[replace_index].i_flag & (Inode::IUPD | Inode::IACC)) != 0){
      //脏inode写回
      Kernel::instance().getFileSystem().writeBackDiskInode(inode_cache_area[replace_index].i_number, inode_cache_area[replace_index]);
    }
    
    inode_cache_area[cur_free_map_pos] = Inode(inode, inode_id);
    inodeCacheBitmap.setBit(cur_free_map_pos);
    //用新的inode覆盖掉
    cur_free_map_pos = replace_index;
  }
  else{
    inode_cache_area[cur_free_map_pos] = Inode(inode, inode_id);
    inodeCacheBitmap.setBit(cur_free_map_pos);
  }
  return cur_free_map_pos;
}





int InodeCache::writeBackInode(){
  /*
   * @brief 写回所有的Inode缓存回磁盘
   */
  //遍历inodeCache，查找存在的并且是脏的inode
  for (int i = 0; i < inodeCacheBitmap.getMapSize(); i++){
    if (inodeCacheBitmap.isAvai(i)){ //该inode缓存有意义
      if (inode_cache_area[i].i_flag & (Inode::IUPD | Inode::IACC)){ // 确实被分配了内容

         Kernel::instance().getFileSystem().writeBackDiskInode(inode_cache_area[i].i_number, inode_cache_area[i]);
      }
    }
  }
  return OK;
}

/* 系统全局打开文件表对象实例的定义 */
OpenFileTable g_OpenFileTable;

OpenFileTable::OpenFileTable(){
}

OpenFileTable::~OpenFileTable(){
}


File *OpenFileTable::FAlloc(){
  /* @brief 进程打开文件描述符表中找的空闲项*/
    for (int i = 0; i < OpenFileTable::NFILE; i++){
        /* f_count==0表示该项空闲 */
        if (this->m_File[i].f_count == 0){
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

void OpenFileTable::CloseF(File *pFile){
    Inode *pNode;
    if (pFile->f_count <= 1){
        /* 如果当前进程是最后一个引用该文件的进程，对特殊块设备、字符设备文件调用相应的关闭函数 */
        // pFile->f_inode->CloseI(pFile->f_flag & File::FWRITE);
        // g_InodeTable.IPut(pFile->f_inode);
        pFile->f_inode_id = 0;
        pFile->f_offset = 0;
    }

    /* 引用当前File的进程数减1 */
    pFile->f_count--;
}


