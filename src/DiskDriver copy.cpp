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
         std::cout << "[ERROR]镜像映射失败" << std::endl;
      #endif
      exit(-1);
      ret_status = VFS_UNINITIALIZED;
   }
   else{
      #ifdef IS_DEBUG
         std::cout << "[INFO]镜像映射成功" << std::endl;
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
         std::cout << "[ERROR]镜像映射解除失败" << std::endl;
      #endif
      return;
   }
   else{
      #ifdef IS_DEBUG
         std::cout << "[INFO]镜像映射解除成功" << std::endl;
      #endif
      close(disk_img_fd);
   }
}

DiskBlock *DiskDriver::getBlk(int block_num){
   return disk_mem_addr + block_num;
}

void DiskDriver::readBlk(int block_num, DiskBlock *dst){
   memcpy(dst, disk_mem_addr + block_num, DISK_BLOCK_SIZE);
}

void DiskDriver::writeBlk(int block_num, const DiskBlock &blk){
   
   memcpy(disk_mem_addr + block_num, &blk, DISK_BLOCK_SIZE);
}


DiskBlock *DiskDriver::getDiskMemAddr(){
   return disk_mem_addr;
}


DirectoryEntry::DirectoryEntry(){
	m_ino = 0;
  memset(m_name, 0, DIRSIZ);
  
}

DirectoryEntry::~DirectoryEntry(){
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

OpenFiles::OpenFiles(){
}

OpenFiles::~OpenFiles(){
}

int OpenFiles::AllocFreeSlot(){
   /** 
	 * @brief 进程请求打开文件时，在打开文件描述符表中分配一个空闲表项
	 */
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
	/**
	 * @brief 根据用户系统调用提供的文件描述符参数fd，找到对应的打开文件控制块File结构
	 */
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
   /** 
	 * @brief 为已分配到的空闲描述符fd和已分配的打开文件表中,空闲File对象建立勾连关系
	 */
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
    this->i_number = i_number; 
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
/* 将包含外存Inode字符块中信息拷贝到内存Inode中 */
void Inode::copyInode(Buf* pb, int inode_id){
    DiskInode& d_inode = *(DiskInode*)(pb->b_addr + (inode_id % INODE_NUMBER_PER_SECTOR )*sizeof(DiskInode));
    this->i_mode = d_inode.d_mode;
    this->i_nlink = d_inode.d_nlink;
    this->i_uid = d_inode.d_uid;
    this->i_gid = d_inode.d_gid;
    this->i_size = d_inode.d_size;
    memcpy(this->i_addr, d_inode.d_addr, sizeof(d_inode.d_addr));
}

static int cnt = 0;

void watch_buffer(DiskBlock* data){
  int* index = (int*) data;
  for(int i = 0 ; i < 128; i ++ ){
    std::cout << i << " : " << index[i] <<std::endl;
  }
  if(cnt++ > 5){
  exit(0);

  }
}
int Inode::Bmap(int lbn){
  /**
   * @param lbn 给定的盘块号
   * @brief 根据混合索引表，用逻辑块号，查出物理盘块号
   *        或查不到，就自己申请新的物理盘块
   *        need to config cache
   */

    /* 转换后的物理盘块号 */
    int phy_blk_id, f_level_index, s_level_index, t_level_index; 
    /* 用于访问索引盘块中一次间接、两次间接索引表 */
    int *inode_link_table;  
    int index, index_in_fst_tab, index_in_sec_tab;

    /* 超出支持的最大文件块数 */
    if (lbn >= Inode::HUGE_FILE_BLOCK){
        //u.u_error = User::EFBIG;
        return ERROR_LBN_OVERFLOW;
    }

    if (lbn < 6){ 
        /* 如果是小型文件，从基本索引表i_addr[0-5]中获得物理盘块号即可 */
        phy_blk_id = this->i_addr[lbn];
        #ifdef IS_DEBUG
          std::cout << "Bmap: " << phy_blk_id << "  this->" <<  this->i_number << "\n";
        #endif
        /*如果该逻辑块号还没有相应的物理盘块号与之对应，则分配一个物理块*/
        if (phy_blk_id == 0){
          
            phy_blk_id = Kernel::instance().getSuperBlock().balloc();
            
            Buf* first_index_buf=Kernel::instance().getBufferManager().GetBlk(phy_blk_id);
                        std::cout << "phy_blk_idnew_free_blk_id: " << phy_blk_id << std::endl;

            Kernel::instance().getBufferManager().Bclear(first_index_buf);
            if (phy_blk_id == -1){
                //分配失败。可能没有空闲空间了
                std::cout << "分配失败。可能没有空闲空间了" << std::endl;
            }
            else{
            /* 当写入位置超出文件大小，进行额外申请，设置延迟写标志就可以初始化该数据块*/
                Kernel::instance().getBufferManager().Bdwrite(first_index_buf);
                this->i_addr[lbn] = phy_blk_id;
                this->i_flag |= Inode::IUPD;
            }
        }
        return phy_blk_id;
    }
    else {
        /* lbn >= 6 大型、巨型文件 */
        Buf *first_index_buf,*second_index_buf;
        DiskBlock f_diskBlock, s_diskBlock;

        /* 计算逻辑块号lbn对应i_addr[]中的索引 */
        if (lbn < Inode::LARGE_FILE_BLOCK){ // 大型文件
            index = (lbn - Inode::SMALL_FILE_BLOCK) / Inode::ADDRESS_PER_INDEX_BLOCK + 6;
        }
        else{ /* 巨型文件: 长度介于263 - (128 * 128 * 2 + 128 * 2 + 6)个盘块之间 */
            index = (lbn - Inode::LARGE_FILE_BLOCK) / (Inode::ADDRESS_PER_INDEX_BLOCK * Inode::ADDRESS_PER_INDEX_BLOCK) + 8;
        }

        phy_blk_id = this->i_addr[index];
        

        //#ifdef IS_DEBUG
          std::cout << "Bmap-L2: " << f_level_index << "\n";
        //#endif

        /* 若该项为零，则表示不存在相应的间接索引表块 */
        if (0 == f_level_index){
            this->i_flag |= Inode::IUPD;
            int new_free_blk_id = Kernel::instance().getSuperBlock().balloc();
            /* 分配一空闲盘块存放间接索引表 */
            if (new_free_blk_id<0){
              /* 分配失败 */
              return ERROR_OUTOF_BLOCK; 
            }
            std::cout << "new_free_blk_id: " << new_free_blk_id << std::endl;
    
            /* i_addr[index]中记录间接索引表的物理盘块号 */
            this->i_addr[index] = new_free_blk_id;
            f_level_index = new_free_blk_id;

            first_index_buf=Kernel::instance().getBufferManager().GetBlk(new_free_blk_id);
            std::cout << "====" << first_index_buf->b_blkno << std::endl;
        }
        else{
            /* 读出存储间接索引表的字符块 */
            f_level_index = phy_blk_id;

            first_index_buf = Kernel::instance().getBufferManager().Bread(phy_blk_id);

            // f_diskBlock = *(first_index_buf->b_addr);
            // Kernel::instance().getBufferManager().Brelse(first_index_buf);
        }
        /* 获取缓冲区首址 */
        
        f_diskBlock = *(first_index_buf->b_addr);
        inode_link_table = (int *)&f_diskBlock;// first_index_buf->b_addr; // 
        Kernel::instance().getBufferManager().Brelse(first_index_buf);


        index_in_sec_tab = index;

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
                s_level_index = new_free_blk_id;

                second_index_buf = Kernel::instance().getBufferManager().GetBlk(new_free_blk_id);
                // Kernel::instance().getBufferManager().Bclear(second_index_buf);
                // /* 将更改后的二次间接索引表延迟写方式输出到磁盘 */
                // Kernel::instance().getBufferManager().Bdwrite(first_index_buf);
            }
            else{
                /* 释放1次间接索引表占用的缓存，并读入2次间接索引表 */
                // Kernel::instance().getBufferManager().Brelse(first_index_buf);
                s_level_index = phy_blk_id;
                second_index_buf = Kernel::instance().getBufferManager().Bread(phy_blk_id);
            }

            //first_index_buf = second_index_buf;
            /* 令iTable指向一次间接索引表 */
            //inode_link_table = (int *)second_index_buf->b_addr;


            f_diskBlock = *(second_index_buf->b_addr);
            inode_link_table = (int *)&f_diskBlock;// first_index_buf->b_addr; // 
            Kernel::instance().getBufferManager().Brelse(second_index_buf);

        }


        /* 计算逻辑块号lbn最终位于一次间接索引表中的表项序号index */
        if (lbn < Inode::LARGE_FILE_BLOCK){
            index = (lbn - Inode::SMALL_FILE_BLOCK) % Inode::ADDRESS_PER_INDEX_BLOCK;
        }
        else{
            index = (lbn - Inode::LARGE_FILE_BLOCK) % Inode::ADDRESS_PER_INDEX_BLOCK;
        }
         //std::cout << "   hhhh: " << index << "\n";
        std::cout << "!!!!" << first_index_buf->b_blkno << std::endl;
        //

        int new_blk_large ;

        std::cout << "   index: " << index << "\n";

        // no matter which is lack of content
        int item_phy_blk_id = inode_link_table[index];
        phy_blk_id = inode_link_table[index];

        int* tmp = (int*)first_index_buf->b_addr;

        //#ifdef IS_DEBUG
          std::cout << "Bmap-L2-item: " << item_phy_blk_id << " " << tmp[index]  << "\n";
        //#endif

        if (item_phy_blk_id  == 0 && (new_blk_large = Kernel::instance().getSuperBlock().balloc()) >= 0){
            /* 第一块没有写满，所以分配到的文件数据盘块号登记在一次间接索引表中 */
            std::cout << "new_blk_large: " << new_blk_large << std::endl;
            int new_item_phy_blk_id = new_blk_large;
            phy_blk_id = new_blk_large;
            inode_link_table[index] = new_item_phy_blk_id; // new alloc block reg in first table


            if (0){
              /* 将数据盘块、更改后的一次间接索引表用延迟写方式输出到磁盘 */
              // second_index_buf = Kernel::instance().getBufferManager().GetBlk(new_blk_large);
              // Kernel::instance().getBufferManager().Bclear(second_index_buf);//清空！
              // Kernel::instance().getBufferManager().Bwrite(second_index_buf);
              second_index_buf = Kernel::instance().getBufferManager().Bread(s_level_index);
              memcpy(second_index_buf->b_addr, inode_link_table, DISK_BLOCK_SIZE);
              Kernel::instance().getBufferManager().Bwrite(second_index_buf);

            }
            else{
              first_index_buf = Kernel::instance().getBufferManager().Bread(f_level_index);
              memcpy(first_index_buf->b_addr, inode_link_table, DISK_BLOCK_SIZE);
              Kernel::instance().getBufferManager().Bwrite(first_index_buf);


            }
            std::cout << "!!!!?????" << first_index_buf->b_blkno << std::endl;
        }
        else{
            /* 释放一次间接索引表占用缓存 */
            
        }

        //std::cout << "Bmap-L2-item: " << phy_blk_id << "\n";
        return phy_blk_id;
        
    }
}


/*
 * 检查编号为inumber的外存INode是否有内存拷贝，
 * 如果有则返回该内存INode在内存INode表中的索引
 */
int InodeCache::IsLoaded(int inumber) {
    
    for (int i = 0; i < NINODE; ++i) {
      //std::cout << "inode_cache_area[i].i_number = " << inode_cache_area[i].i_number << "  " << inumber << std::endl;

        if (inode_cache_area[i].i_number == inumber) { // &&inode_cache_area[i].i_count
            return i;
        }
    }
    return -1;
}

/* 在内存INode表中寻找一个空闲的内存INode */
int InodeCache::getFreeINode() {
    for (int i = 0; i < InodeCache::NINODE; i++) {
        if (this->inode_cache_area[i].i_count == 0) {
            return  i;
        }
    }
    return -1;
}

void InodeCache::init(){
  /**
   * @brief 清空bitmap
   */
  memset(inode_cache_area, 0, sizeof(inode_cache_area));
  
  //inodeCacheBitmap.clear();
}

Inode *InodeCache::getInodeByID(int inode_id){
  /**
   * @brief 遍历查找InodeCache区域看有没有inodeID吻合的inode，若有则取出;
   *        若没有，向磁盘申请新的inode，并放入内存
   */
  // for (int i = 0; i < INODE_CACHE_SIZE; i++){
  //   if (IsLoaded(i) > 0){
  //     //首先需要这个inodeCache是有效的
  //     return &inode_cache_area[i];
  //   }
  // }

    Inode* pInode;
    int index = IsLoaded(inode_id);

    //std::cout << "IsLoaded: "<< index << "  inodeID :" << inode_id << std::endl;

    if (index >= 0) {
        pInode = inode_cache_area + index;
        ++pInode->i_count;
        return pInode;
    }



    // index = getFreeINode();

    // if (-1 == index) {
    //     std::cout << "[ERROR] inodeTable 满了！" << std::endl;
    //     return NULL;
    // }

    
    // pInode = inode_cache_area + index;
    // pInode->i_number = inode_id;
    // pInode->i_count++;

    

    // BufferCache* cur_buffer_cache = &Kernel::instance().getBufferManager();
    // Buf* pBuffer = cur_buffer_cache->Bread(INODE_ZONE_START_SECTOR + inode_id / INODE_NUMBER_PER_SECTOR);
    
    // std::cout << "index : " << pInode->i_number << std::endl;

    // pInode = 
    // pInode->copyInode(pBuffer, inode_id);
    // cur_buffer_cache->Brelse(pBuffer);


    // std::cout << "index : " << pInode->i_number << std::endl;

    // return pInode;

  //没有在inodeCache中找到，新申请一个
  return &inode_cache_area[addInodeCache((Kernel::instance().getFileSystem().getDiskInodeById(inode_id)), inode_id)];

} 

int InodeCache::addInodeCache(DiskInode inode, InodeId inode_id){
  /**
   * @brief 将磁盘Inode拷贝到InodeCache
   *        若InodeCache未满，则直接放入;否则,发生替换.
   * @return 放入位置的下标 
   */
  int cur_free_map_pos = getFreeINode();

  #ifdef IS_DEBUG
    std::cout << "cur_free_map_pos: " << cur_free_map_pos <<std::endl;
  #endif

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
    //inodeCacheBitmap.setBit(cur_free_map_pos);
    //用新的inode覆盖掉
    cur_free_map_pos = replace_index;
  }
  else{
    inode_cache_area[cur_free_map_pos] = Inode(inode, inode_id);
    //inodeCacheBitmap.setBit(cur_free_map_pos);
  }

  inode_cache_area[cur_free_map_pos].i_count = 1;
  inode_cache_area[cur_free_map_pos].i_flag = (Inode::IUPD | Inode::IACC);

  #ifdef IS_DEBUG
    std::cout << "cur_free_map_pos: " << cur_free_map_pos <<std::endl;
  #endif

  return cur_free_map_pos;
}


int InodeCache::writeBackInode(){
  /*
   * @brief 写回所有的Inode缓存回磁盘
   */
  //遍历inodeCache，查找存在的并且是脏的inode
  for (int i = 0; i < InodeCache::NINODE; i++){
    if (inode_cache_area[i].i_count != 0){ //该inode缓存有意义
          //std::cout << "fuck" << std::endl;

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
  /** 
   * @brief 进程打开文件描述符表中找的空闲项
   */
    for (int i = 0; i < OpenFileTable::NFILE; i++){
        /* f_count==0表示该项空闲 */
        if (this->m_File[i].f_count == 0){
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
   /** 
	 * @brief 对打开文件控制块File结构的引用计数f_count减1，若引用计数f_count为0，则释放File结构。
	 */
    Inode *pNode;
    if (pFile->f_count <= 1){
        /* 如果当前进程是最后一个引用该文件的进程，对特殊块设备、字符设备文件调用相应的关闭函数 */
        pFile->f_inode_id = 0;
        pFile->f_offset = 0;
    }

    /* 引用当前File的进程数减1 */
    pFile->f_count--;
}


