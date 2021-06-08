#include "FileSystem.h"
#include "Kernel.h"
#include "Tools.h"
#include "DiskDriver.h"

SuperBlock::SuperBlock(){ 
    this->total_inode_num = INODE_ZONE_SIZE;
    this->total_block_num = DISK_SIZE / sizeof(DiskBlock);
    
    this->free_block_bum = 0;
    this->s_free[0] = -1;

    this->free_inode_num = 0;

    this->s_flock = 0;
    this->s_ilock = 0;
    this->s_fmod = 0;
    this->s_ronly = 0;
}

void SuperBlock::writeBackSuper(){
    /** 
     * @brief 回写superblock
     */
    BufferCache* my_buffer_cache = &Kernel::instance().getBufferManager();
	for (int j = 0; j < 2; j++) { 
        // 就前两个盘块
		int* p = (int *)this + j * 128;
        // （int） 4 * 128 ！
		Buf *p_buf = my_buffer_cache->GetBlk(SUPERBLOCK_START_SECTOR + j);
		memcpy(p_buf->b_addr, p, DISK_BLOCK_SIZE);
		Kernel::instance().getBufferManager().Bwrite(p_buf);
	}
}

void SuperBlock::init(){

}

BlkId SuperBlock::balloc() {
    /** 
     * @brief 分配空闲磁盘块！ 
     * */
	int blkno;
	Buf* pBuffer;
    BufferCache* my_buffer_cache = &Kernel::instance().getBufferManager();

    /* 从索引表“栈顶”获取空闲磁盘块编号 */
	blkno = s_free[--free_block_bum];

    #ifdef IS_DEBUG
        std::cout <<"blkno : " <<  blkno << " free_block_bum: " <<free_block_bum << std::endl;
    #endif

    /* 若获取磁盘块编号为零，则表示已分配尽所有的空闲磁盘块。*/
	if (blkno < 0) {
		free_block_bum = 0;
        std::cout << "[ERROR]所有盘块都被分配完了" << std::endl;
		return -1;
	}

	/*
	* 栈已空，新分配到空闲磁盘块中记录了下一组空闲磁盘块的编号，将下一组空闲磁盘块的编号读入SuperBlock的空闲磁盘块索引表s_free[100]中。
	*/
    if (free_block_bum <= 0) {
        #ifdef IS_DEBUG
            std::cout << "***我在这里***" << std::endl;
        #endif
		pBuffer = my_buffer_cache->Bread(blkno);
		int* p = (int *)pBuffer->b_addr;
        // 成组连接法， 重置空闲数
		free_block_bum = *p; 
        p ++ ;

        #ifdef IS_DEBUG
            std::cout << "成组连接法: " << free_block_bum << std::endl;
        #endif
        /* 重置空闲项 */
		memcpy(s_free, p, sizeof(s_free));
		my_buffer_cache->Brelse(pBuffer);
	}
    
	pBuffer = my_buffer_cache->GetBlk(blkno);

	if (pBuffer) {
        /* 如果申请到了，马上清空内容！ */
		my_buffer_cache->Bclear(pBuffer);
        #ifdef IS_DEBUG
            std::cout << "why here?" << std::endl;
        #endif
        my_buffer_cache->Bwrite(pBuffer);
	}
    #ifdef IS_DEBUG
        std::cout << "[superblock] balloc blkno: " << blkno << "  pBuffer: " << pBuffer << "\n";
	#endif
    return blkno;
}


void SuperBlock::bfree(BlkId blknum) {
    /** 
     * @brief 释放为blkno的磁盘块
     * 
     *  */
	Buf* pBuffer;
    BufferCache* my_buffer_cache = &Kernel::instance().getBufferManager();

	if (free_block_bum >= MAX_INODE_NUM ) {
        /* 超出栈的上限啦 */
		pBuffer = my_buffer_cache->GetBlk(blknum);
		int *p = (int*)pBuffer->b_addr;
		/* 重新压回去啦， 先压数，再压内容*/
        *p++ = free_block_bum;
		memcpy(p, s_free, sizeof(int)*MAX_INODE_NUM);
        
		free_block_bum = 0;
		my_buffer_cache->Bwrite(pBuffer);
	}
	s_free[free_block_bum++] = blknum;
}

InodeId SuperBlock::ialloc(){
    /**
     * @brief 分配空闲inode
     */
    Inode* pInode;

    int ino;
    BufferCache* my_buffer_cache = &Kernel::instance().getBufferManager();
    InodeCache* my_inode_cache = &Kernel::instance().getInodeCache();

    if (free_inode_num <= 0) {
        /* SuperBlock直接管理的空闲Inode索引表已空，必须到磁盘上搜索空闲Inode。*/
        ino = -1;
        for (int i = 0; i < total_inode_num; ++i) {
            /* 遍历就完事了！*/
            Buf* pBuffer = my_buffer_cache->Bread(INODE_ZONE_START_SECTOR + i);
            int* p = (int*)pBuffer->b_addr;
            for (int j = 0; j < sizeof(DiskBlock) / sizeof(DiskInode); ++j) {
                /* 暴力遍历每一个block的每一个inode项 */
                ++ino;
                int mode = *(p + j * sizeof(DiskInode) / sizeof(int));
                if (mode) {
                    continue;
                }

                /*
                * i_mode==0，且不在内存内，说明是空闲的！
                */
                if (Kernel::instance().getInodeCache().isLoaded(ino) == -1) {
                    s_inode[free_inode_num++] = ino;
                    if (free_inode_num >= MAX_INODE_NUM) {
                        break;
                    }
                }
            }
            /* 这个盘块搞完了 */
            my_buffer_cache->Brelse(pBuffer);
            /* 搞完检查满没满，满了就直接结束 */
            if (free_inode_num >= MAX_INODE_NUM) {
                break;
            }
        }
        if (free_inode_num <= 0) {
            return -1;
        }

    }
    /* 拿出来！*/
    ino = s_inode[--free_inode_num];
    pInode = my_inode_cache->getInodeByID(ino);
    if (NULL == pInode) {
        return -1;
    }
    return pInode->i_number;
}


void SuperBlock::ifree(InodeId inode_id){
    /**
     * @brief 回收inode
     */
	if (free_inode_num >= MAX_INODE_NUM) {
		return ;
	}
	s_inode[free_inode_num++] = inode_id;
	
}

DiskInode::DiskInode(){
    /**
     * @brief 初始化
     */
	this->d_mode = 0;
	this->d_nlink = 0;
	this->d_size = 0;
	for (int i = 0; i < 10; i++){
		this->d_addr[i] = 0;
	}
}

DiskInode::DiskInode(unsigned int d_mode, int d_nlink, short d_uid, short d_gid, int d_size, int d_addr[10]){
	this->d_mode = d_mode;
	this->d_nlink = d_nlink;
	this->d_uid = d_uid;
	this->d_gid = d_gid;
	this->d_size = d_size;
	memcpy(this->d_addr, d_addr, MIXED_ADDR_TABLE_SIZE);
}


DiskInode::DiskInode(Inode inode){
    /**
     * @brief inode拷贝成diskinode
     */
	d_mode = inode.i_mode;
	d_nlink = inode.i_nlink;
	d_uid = inode.i_uid;
	d_gid = inode.i_gid;
	d_size = inode.i_size;
	memcpy(d_addr, inode.i_addr, MIXED_ADDR_TABLE_SIZE);
} 

void VFS::format(){
    /**
     * @brief 清空镜像文件，并进行初始化
     */
    Kernel::instance().getInodeCache().init();
    Kernel::instance().getBufferManager().init();
    Kernel::instance().getUser().init();
    
    /* 磁盘分配情况
     * 0-1# superblock
     * 2-1024# InodeBlock
     * 1025~DISK_BLOCK_NUM-1# 放数据
     */

    DiskBlock * head = Kernel::instance().getDiskDriver().getDiskMemAddr();
    DiskBlock *diskMemAddr = Kernel::instance().getDiskDriver().getDiskMemAddr();
    /* 清空block */
    memset(diskMemAddr, 0, DISK_SIZE);

    /* 初始化superblock  */
    SuperBlock superBlock;
    //空文件，先写入superblock占据空间，未设置文件大小
    SuperBlock *p_superBlock = (SuperBlock *)diskMemAddr;
    *p_superBlock = superBlock; 
    p_superBlock++;

    // 更新指针
    diskMemAddr = (DiskBlock *)p_superBlock;
    #ifdef IS_DEBUG
        std::cout << "super: " << (char*)diskMemAddr - (char*)head << std::endl;
    #endif

    DiskInode emptyDINode;

    //根目录DiskNode
    int tempAddr[10] = {1024, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // 指向他的数据块
    DiskInode rootDINode = DiskInode(Inode::IFDIR, 1, 1, 1, 0, tempAddr);// 6 * sizeof(DirectoryEntry)
    rootDINode.d_nlink = 1;
    InodeId root_id = 1; // 默认是1 
    

    DiskInode *p_DiskInode = (DiskInode *)diskMemAddr;

    //从第1个DiskINode初始化，第0个固定用于根目录"/"，不可改变
    for (int i = 0; i < INODE_NUMBERS; ++i) {
        if (superBlock.total_inode_num < MAX_INODE_NUM) {
            /* 初始化inode的同时也进行inode的压入，压入一百个就行了 */
            superBlock.s_inode[superBlock.total_inode_num++] = i;
        }
        if(i == root_id){
            *p_DiskInode = rootDINode;
                #ifdef IS_DEBUG
                    std::cout << "root: " << (char*)p_DiskInode - (char*)p_superBlock << std::endl;
                #endif
        }
        else{
            *p_DiskInode = emptyDINode;
        }
        /* 写入DiskNode，并后移 */
        p_DiskInode++;
    }
    #ifdef IS_DEBUG
        std::cout << "all_indoe: " << INODE_NUMBERS << " " << ((char*)p_DiskInode - (char*)head) / 512 << std::endl;
    #endif

    //根目录的块
    DiskBlock rootBlock;
    /* 写入到rootInode的下 */
    DirectoryEntry *p_directoryEntry = (DirectoryEntry *)rootBlock.content;
    DirectoryEntry tempDirctoryEntry;
    strcpy(tempDirctoryEntry.m_name, ".");
    tempDirctoryEntry.m_ino = root_id;
    *p_directoryEntry = tempDirctoryEntry;
    p_directoryEntry++;
    strcpy(tempDirctoryEntry.m_name, "..");
    tempDirctoryEntry.m_ino = root_id;
    *p_directoryEntry = tempDirctoryEntry;
    p_directoryEntry++;

    /* 两个空块 */
    DiskBlock freeBlock, freeBlock1;
    DiskBlock *p_DiskBlock = (DiskBlock *)p_DiskInode;

    for (int i = 0; i < DATA_ZONE_SIZE; ++i) {
        // 超过最大值，把整块信息连同
        if (superBlock.free_block_bum >= MAX_INODE_NUM) {
            /* 成组连接法 */
            memcpy(freeBlock1.content, &superBlock.free_block_bum, sizeof(int) + sizeof(superBlock.s_free));
            #ifdef IS_DEBUG
                std::cout << (int)*freeBlock1.content << std::endl;
            #endif
            if(i == 0){
                // 指定块（根目录）进行初始化！
                *p_DiskBlock = rootBlock;
            }
            else{
                *p_DiskBlock = freeBlock1;
            }
            
            p_DiskBlock++;

            superBlock.free_block_bum = 0;
        }
        else {
            if(i == 0){
                #ifdef IS_DEBUG
                    std::cout << "Head: " << ((char*)p_DiskBlock - (char*)head) / 512 << std::endl;
                #endif
                *p_DiskBlock = rootBlock;
            }
            else{
                *p_DiskBlock = freeBlock;
            }
            p_DiskBlock++;
        }
        /** 成组链接法写入啦 */
        superBlock.s_free[superBlock.free_block_bum++] = i + DATA_ZONE_START_SECTOR;
    }
    #ifdef IS_DEBUG
        std::cout << "DiskBlock: " << 1024 << "~" << 1024 +((char*)p_DiskBlock - (char*)p_DiskInode) / 512 << std::endl;
        std::cout << "superBlock.free_block_bum: " << 1024 << "~" << 1024 + superBlock.free_block_bum << "\n";
    #endif

    /* 再重新初始化 superblock */
    diskMemAddr = Kernel::instance().getDiskDriver().getDiskMemAddr();
    p_superBlock = (SuperBlock *)diskMemAddr;
    *p_superBlock = superBlock; 

    Kernel::instance().getSuperBlock() = superBlock;
    /* 初始化当前路径 */
    Kernel::instance().getUser().cur_path = "/";
    ext2_status = VFS_READY;
}


void VFS::loadSuperBlock(SuperBlock &superBlock){
    /**
     * @brief 重新读出superBlock
     */
    DiskBlock *diskMemAddr = Kernel::instance().getDiskDriver().getDiskMemAddr();
    memcpy(&superBlock, diskMemAddr, DISK_BLOCK_SIZE * 2);
}

int VFS::setBufferCache(BufferCache *bufferCache){
    this->p_bufferCache = bufferCache;
    return OK;
}


void VFS::writeBackDiskInode(int inode_id, Inode cur_inode){ 
    /**
     * @brief 将diskInode重新写回磁盘中
     * @note  要先读后写!
     */
    #ifdef IS_DEBUG
        std::cout << "[writeBack]: " << inode_id << std::endl;
    #endif

    Buf *p_buf = p_bufferCache->Bread(INODE_ZONE_START_SECTOR + cur_inode.i_number / INODE_NUMBER_PER_SECTOR);
	DiskInode dINode(cur_inode);
    /* 定位diskblock上的inode位置 */
	unsigned char* p = (unsigned char*) p_buf->b_addr + (cur_inode.i_number % INODE_NUMBER_PER_SECTOR) * sizeof(DiskInode);
	DiskInode* pNode = &dINode;
    /* 拷贝啦 */
	memcpy(p, pNode, sizeof(DiskInode));
	p_bufferCache->Bdwrite(p_buf); 
}

DiskInode VFS::getDiskInodeById(int inode_id){
    /** 
     * @brief 根据inode_id 找diskInode
     */
    /* 跳过一开始的两个superblock啦 */
    int blk_num = 2 + inode_id / (DISK_BLOCK_SIZE / DISKINODE_SIZE);

    #ifdef IS_DEBUG
        std::cout << "getDiskInode : " << blk_num << std::endl;
    #endif

    DiskBlock cur_diskBlock;
    Kernel::instance().getDiskDriver().readBlk(blk_num, &cur_diskBlock);
    const DiskInode *cur_diskInode = (DiskInode *)&cur_diskBlock;
    DiskInode tempDiskInode = *(cur_diskInode + inode_id % (DISK_BLOCK_SIZE / DISKINODE_SIZE));

    return tempDiskInode;
}


InodeId VFS::locateInode(const myPath& path){
    /**
     * @brief 根据路径，做线性目录搜索
     * @return 该文件的InodeId
     * 
     */
    //先确定其父目录的inode号
    InodeId par_inode_id = locateParDir(path); 
    if (path.getLevel() < 0){
        // cd / (此时getLevel == -1)
        return ROOT_INODE_ID;
    }
    else{
        /* 不然就继续进入下一步 */
        return getInodeIdInDir(par_inode_id, path.getInodeName().c_str());
    }
}


InodeId VFS::locateParDir(const myPath& path){
    /**
     * @brief 找到当前路径的父路径，从头开始遍历...
     * @return 返回父路径
     */
    //目录文件的inode号
    InodeId dirInode; 

    if (path.from_root){ 
        //如果是绝对路径,从根inode开始搜索
        dirInode = ROOT_INODE_ID;
    }
    else{ 
        //如果是相对路径，从当前inode号开始搜索
        dirInode = Kernel::instance().getUser().curDirInodeId;
    }

    for (int i = 0; i < path.getLevel(); i++){
        //遍历全部路径，如果有一个没有找到就报错
        #ifdef IS_DEBUG
            std::cout << "  i :" << i << std::endl;
        #endif
        dirInode = getInodeIdInDir(dirInode, path.path[i].c_str());
        if (dirInode < 0){
            return ERROR_PATH_NFOUND; //没有找到
        }
    }
    return dirInode;
}

InodeId VFS::getInodeIdInDir(InodeId par_inode_id, FileName fileName){ 
    /**  内部调用
     * @brief 在当前目录下， 找指定文件名的文件
     * @return 找到了就返回该文件的Inode，不然就返回-1 
     */

    //先根据目录inode号dirInodeId获得目录inode对象
    int ret = -1;

    Inode *p_dir_inode = Kernel::instance().getInodeCache().getInodeByID(par_inode_id);

    #ifdef IS_DEBUG
        std::cout <<"getInodeIdInDir(inode) : " <<  p_dir_inode->i_number << std::endl;
    #endif

    //读取该inode指示的数据块
    int blk_num = p_dir_inode->Bmap(0); //Bmap查物理块号

    #ifdef IS_DEBUG
        std::cout << "getInodeIdInDir(blk_num) : " << blk_num << std::endl;
    #endif

    Buf *p_buf = Kernel::instance().getBufferManager().Bread(blk_num);

    Buf real_buf = *p_buf;

    DirectoryEntry *p_directoryEntry = (DirectoryEntry *)real_buf.b_addr;

    //访问这个目录文件中的entry
    for (int i = 0; i < DISK_BLOCK_SIZE / sizeof(DirectoryEntry); i++){
        DirectoryEntry * cur_entry = p_directoryEntry + i;
        if ((cur_entry->m_ino != 0) && (!strcmp(cur_entry->m_name, fileName))){
            ret = cur_entry->m_ino;
            #ifdef IS_DEBUG
                std::cout << "getInodeIdInDir: " << ret << "  " << cur_entry->m_name << std::endl;
            #endif
            break;
        } //ino==0表示该文件被删除
    }
    return ret;
}



VFS_Status VFS::getExt2Status(){
    return ext2_status;
}

VFS_Status VFS::setExt2Status(VFS_Status ext2_status){
    this->ext2_status = ext2_status;
}

