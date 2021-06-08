#include "FileSystem.h"
#include "Kernel.h"
#include "Tools.h"
#include "DiskDriver.h"

SuperBlock::SuperBlock(){ //:  disk_block_bitmap(DISK_SIZE / DISK_BLOCK_SIZE)
    this->total_inode_num = INODE_ZONE_SIZE;
    this->total_block_num = DISK_SIZE / sizeof(DiskBlock);
    
    this->free_block_bum = 0;
    this->s_free[0] = -1;

    this->free_inode_num = 0;

    this->s_flock = 0;
    this->s_ilock = 0;
    this->s_fmod = 0;
    this->s_ronly = 0;



    //总inode数  -1是因为0#inode不可用
    // total_inode_num = MAX_INODE_NUM - 1; 
    // //空闲inode
    // free_inode_num = total_inode_num;    
    // //初始化空闲inode栈
    // for (int i = 0; i < total_inode_num; i++){
    //     s_inode[i] = total_inode_num - i;
    // }

        //空闲盘块初始化
    // char freeBlock[DISK_BLOCK_SIZE], freeBlock1[DISK_BLOCK_SIZE];
    // memset(freeBlock, 0, DISK_BLOCK_SIZE);
    // memset(freeBlock1, 0, DISK_BLOCK_SIZE);

    // for (int i = 0; i < DATA_ZONE_SIZE; ++i) {
    //     // 超过最大值，把整块信息连同
    //     if (free_inode_num >= MAX_INODE_NUM) {
    //         memcpy(freeBlock1, &s_nfree, sizeof(int) + sizeof(s_free));
    //         deviceDriver->write(&freeBlock1, BLOCK_SIZE);
    //         superBlock->s_nfree = 0;
    //     }
    //     else {
    //         deviceDriver->write(freeBlock, BLOCK_SIZE);
    //     }
    //     s_free[free_inode_num++] = i + DATA_ZONE_START_SECTOR;
    // }
}

void SuperBlock::writeBackSuper(){
    /** 
     * @brief 回写superblock
     */
    BufferCache* my_buffer_cache = &Kernel::instance().getBufferManager();
	for (int j = 0; j < 2; j++) {
		int* p = (int *)this + j * 128;
		Buf *p_buf = my_buffer_cache->GetBlk(SUPERBLOCK_START_SECTOR + j);
		memcpy(p_buf->b_addr, p, DISK_BLOCK_SIZE);
		Kernel::instance().getBufferManager().Bwrite(p_buf);
	}
    
    // for (int j = 0; j < 2; j++) { // superBlock大小为2！
    //     Buf *p_buf = Kernel::instance().getBufferManager().GetBlk(j);
    //     SuperBlock *p_superBlock = (SuperBlock *)p_buf->b_addr;
    //     *p_superBlock = *this;
    //     Kernel::instance().getBufferManager().Bdwrite(p_buf);
	// }



}

void SuperBlock::init(){

}

BlkId SuperBlock::balloc() {
    /** 
     * @brief 在存储设备上分配空闲磁盘块！ 
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
	* 栈已空，新分配到空闲磁盘块中记录了下一组空闲磁盘块的编号
	* 将下一组空闲磁盘块的编号读入SuperBlock的空闲磁盘块索引表s_free[100]中。
	*/
    if (free_block_bum <= 0) {
        #ifdef IS_DEBUG
            std::cout << "***我在这里***" << std::endl;
        #endif
		pBuffer = my_buffer_cache->Bread(blkno);
		int* p = (int *)pBuffer->b_addr;

		free_block_bum = *p; // 成组连接法
        p ++ ;

        //#ifdef IS_DEBUG
            std::cout << "成组连接法: " << free_block_bum << std::endl;
        //#endif

		memcpy(s_free, p, sizeof(s_free));
		my_buffer_cache->Brelse(pBuffer);
	}
    
	pBuffer = my_buffer_cache->GetBlk(blkno);

	if (pBuffer) {
        /* need clear immed!*/
		my_buffer_cache->Bclear(pBuffer);
        std::cout << "why here?" << std::endl;
        my_buffer_cache->Bwrite(pBuffer);
	}
    #ifdef IS_DEBUG
        std::cout << "[superblock] balloc blkno: " << blkno << "  pBuffer: " << pBuffer << "\n";
	#endif
    return blkno;
}

/* 释放存储设备dev上编号为blkno的磁盘块 */
void SuperBlock::bfree(BlkId blknum) {
	Buf* pBuffer;
    BufferCache* my_buffer_cache = &Kernel::instance().getBufferManager();

	if (free_block_bum >= MAX_INODE_NUM ) {
		pBuffer = my_buffer_cache->GetBlk(blknum);
		int *p = (int*)pBuffer->b_addr;
		*p++ = free_block_bum;

		memcpy(p, s_free, sizeof(int)*MAX_INODE_NUM);
        
		free_block_bum = 0;
		my_buffer_cache->Bwrite(pBuffer);
	}

    
	s_free[free_block_bum++] = blknum;
    
}



// BlkId SuperBlock::balloc(){
//     /**
//      * @brief 分配一个空闲盘块号
//      */
//     int ret = disk_block_bitmap.getFreeBitId();
//     disk_block_bitmap.setBit(ret);
//     free_block_bum--;
//     return ret;
// }


// void SuperBlock::bfree(BlkId blknum){
//     /**
//      * @brief 回收一个盘块
//      */
//     free_block_bum++;
//     disk_block_bitmap.unsetBit(blknum);
// }

// void SuperBlock::ballocCeratin(BlkId blk_num){
//     /**
//      * @brief 分配指定的盘块
//      */
//     if (disk_block_bitmap.isAvai(blk_num)){

//     }
//     else{
//         disk_block_bitmap.setBit(blk_num);
//         free_block_bum--;
//     }
// }


InodeId SuperBlock::ialloc(){
    /**
     * @brief 分配空闲inode
     */
    // if (free_inode_num != 0){
        
    //     return s_inode[--free_inode_num];
    // }
    // else{
    //     return ERROR_OUTOF_INODE;
    // }
    Inode* pInode;

    int ino;
    /* SuperBlock直接管理的空闲Inode索引表已空，必须到磁盘上搜索空闲Inode。*/

    BufferCache* my_buffer_cache = &Kernel::instance().getBufferManager();
    InodeCache* my_inode_cache = &Kernel::instance().getInodeCache();

    if (free_inode_num <= 0) {
        ino = -1;
        for (int i = 0; i < total_inode_num; ++i) {
            Buf* pBuffer = my_buffer_cache->Bread(INODE_ZONE_START_SECTOR + i);
            int* p = (int*)pBuffer->b_addr;
            for (int j = 0; j < sizeof(DiskBlock) / sizeof(DiskInode); ++j) {
                ++ino;
                int mode = *(p + j * sizeof(DiskInode) / sizeof(int));
                if (mode) {
                    continue;
                }

                /*
                * 如果外存inode的i_mode==0，此时并不能确定该inode是空闲的，
                * 因为有可能是内存inode没有写到磁盘上,所以要继续搜索内存inode中是否有相应的项
                */
                if (Kernel::instance().getInodeCache().IsLoaded(ino) == -1) {
                    s_inode[free_inode_num++] = ino;
                    if (free_inode_num >= MAX_INODE_NUM) {
                        break;
                    }
                }
            }

            my_buffer_cache->Brelse(pBuffer);
            if (free_inode_num >= MAX_INODE_NUM) {
                break;
            }
        }
        if (free_inode_num <= 0) {
            return -1;
        }

    }

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


// InodeBlock::InodeBlock() : inode_bitmap(MAX_INODE_NUM){
// }


// int InodeBlock::ialloc(){
//     /**
//     * @brief 分配一个空闲inode号
//     */
//     int ret = inode_bitmap.getFreeBitId();
//     inode_bitmap.setBit(ret);
//     return ret;
// }


// void InodeBlock::ifree(InodeId InodeID){
//     /**
//     * @brief 回收一个inode
//     */
//     inode_bitmap.unsetBit(InodeID);
// }


// DiskInode *InodeBlock::getInode(InodeId InodeID){
//     /**
//     * @brief 根据InodeID获取一个Inode结构
//     */
//     return &inode_block[InodeID];
// }


// void InodeBlock::iupdate(InodeId inode_id,DiskInode disk_inode){
//     /**
//      * @brief 将某个制定inode号的inode更新为diskInode的内容
//      */
//     inode_bitmap.setBit(inode_id);   
//     // 置位inode_id为disk_inode
//     inode_block[inode_id]=disk_inode;
// }

DiskInode::DiskInode(){
	this->d_mode = 0;
	this->d_nlink = 0;
	this->d_size = 0;
	for (int i = 0; i < 10; i++)
	{
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
     * 
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

    /* 清空block */
    DiskBlock * head = Kernel::instance().getDiskDriver().getDiskMemAddr();

    DiskBlock *diskMemAddr = Kernel::instance().getDiskDriver().getDiskMemAddr();
    memset(diskMemAddr, 0, DISK_SIZE);

    /* 初始化superblock  */
    SuperBlock superBlock;
    //空文件，先写入superblock占据空间，未设置文件大小
    SuperBlock *p_superBlock = (SuperBlock *)diskMemAddr;
    *p_superBlock = superBlock; 
    p_superBlock++;
    diskMemAddr = (DiskBlock *)p_superBlock;

    #ifdef IS_DEBUG
        std::cout << "super: " << (char*)diskMemAddr - (char*)head << std::endl;
    #endif

    DiskInode emptyDINode;

    //根目录DiskNode
    int tempAddr[10] = {1024, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // 指向他的数据块
    DiskInode rootDINode = DiskInode(Inode::IFDIR, 1, 1, 1, 0, tempAddr);// 6 * sizeof(DirectoryEntry)
    rootDINode.d_nlink = 1;
    InodeId root_id = 1;
    
    //Kernel::instance().getInodeCache().addInodeCache(rootDINode, root_id);

    /* 写入根目录DiskNode */
    DiskInode *p_DiskInode = (DiskInode *)diskMemAddr;
    // *p_DiskInode = rootDINode;
    // p_DiskInode++;


    // deviceDriver->write(&rootDINode, sizeof(rootDINode));
    // /* 初始化inodeCache */
    // InodeBlock tempInodePool;
    // int tempAddr[10] = {4, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    // DiskInode tempDiskInode = DiskInode(Inode::IFDIR, 1, 1, 1, 0, tempAddr);// 6 * sizeof(DirectoryEntry)
    // tempInodePool.iupdate(1, tempDiskInode);
    

    // diskMemAddr = (DiskBlock *)p_InodePool;
    // printf("sizeof(rootDINode)%d\n", sizeof(rootDINode));
    // printf("sizeof(int)%d\n", sizeof(int));

    //从第1个DiskINode初始化，第0个固定用于根目录"/"，不可改变
    
    for (int i = 0; i < INODE_NUMBERS; ++i) {
        //std::cout << i << std::endl;
        if (superBlock.total_inode_num < MAX_INODE_NUM) {
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
        /* 写入DiskNode */
        
        p_DiskInode++;
    }
    #ifdef IS_DEBUG
        std::cout << "all_indoe: " << INODE_NUMBERS << " " << ((char*)p_DiskInode - (char*)head) / 512 << std::endl;
    #endif

    //空闲盘块初始化
    
    DiskBlock freeBlock, freeBlock1;
    // memset(freeBlock, 0, DISK_BLOCK_SIZE);
    // memset(freeBlock1, 0, DISK_BLOCK_SIZE);


    
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


    DiskBlock *p_DiskBlock = (DiskBlock *)p_DiskInode;

    for (int i = 0; i < DATA_ZONE_SIZE; ++i) {
        // 超过最大值，把整块信息连同
        //std::cout <<DATA_ZONE_SIZE<<" " <<  i << std::endl;
        if (superBlock.free_block_bum >= MAX_INODE_NUM) {
            memcpy(freeBlock1.content, &superBlock.free_block_bum, sizeof(int) + sizeof(superBlock.s_free));
            #ifdef IS_DEBUG
                std::cout << (int)*freeBlock1.content << std::endl;
            #endif
            if(i == 0){
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
        superBlock.s_free[superBlock.free_block_bum++] = i + DATA_ZONE_START_SECTOR;
    }
    #ifdef IS_DEBUG
        std::cout << "DiskBlock: " << 1024 << "~" << 1024 +((char*)p_DiskBlock - (char*)p_DiskInode) / 512 << std::endl;
        std::cout << "superBlock.free_block_bum: " << 1024 << "~" << 1024 + superBlock.free_block_bum << "\n";
    #endif


    /* 0  superblock  */
    // tempSuperBlock.ballocCeratin(0); 
    // tempSuperBlock.ballocCeratin(1);
    // tempSuperBlock.ballocCeratin(2);
    // tempSuperBlock.ballocCeratin(3); 

    // /* 4 数据 */ 
    // tempSuperBlock.ballocCeratin(4); 
    // tempSuperBlock.free_inode_num -= 1;

    diskMemAddr = Kernel::instance().getDiskDriver().getDiskMemAddr();
    p_superBlock = (SuperBlock *)diskMemAddr;
    *p_superBlock = superBlock; 

    //Kernel::instance().getInodeCache().writeBackInode();

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


    // Buf *p_buf;
    // p_buf = p_bufferCache->Bread(0);
    memcpy(&superBlock, diskMemAddr, DISK_BLOCK_SIZE * 2);
    //p_bufferCache->Brelse(p_buf);
}

int VFS::setBufferCache(BufferCache *bufferCache){
    this->p_bufferCache = bufferCache;
    return OK;
}


void VFS::writeBackDiskInode(int inode_id, Inode cur_inode){ 
    /**
     * @brief 将diskInode重新写回磁盘中
     * @notice 要先读后写!
     */
    // 定位到第二个inode pool
    #ifdef IS_DEBUG
        std::cout << "[writeBack]: " << inode_id << std::endl;
    #endif
    if (1) { // this->i_flag&(INode::IUPD | INode::IACC)
        Buf *p_buf = p_bufferCache->Bread(INODE_ZONE_START_SECTOR + cur_inode.i_number / INODE_NUMBER_PER_SECTOR);
		DiskInode dINode(cur_inode);

		unsigned char* p = (unsigned char*) p_buf->b_addr + (cur_inode.i_number % INODE_NUMBER_PER_SECTOR) * sizeof(DiskInode);
		DiskInode* pNode = &dINode;
        
		memcpy(p, pNode, sizeof(DiskInode));
        
		p_bufferCache->Bdwrite(p_buf); 
	}
    // int blk_num = 2 + inode_id / (DISK_BLOCK_SIZE / DISKINODE_SIZE);

    // std::cout << "!!writeBack!!: " << blk_num << std::endl;

    
    // DiskInode *cur_diskInode = (DiskInode *)p_buf->b_addr;
    // cur_diskInode = cur_diskInode + inode_id % (DISK_BLOCK_SIZE / DISKINODE_SIZE);
    // //定位到需要写diskInode的位置
    // *cur_diskInode = disk_inode;     
    //p_bufferCache->Bdwrite(p_buf); 




}

DiskInode VFS::getDiskInodeById(int inode_id){
    //inode分布在两个盘块上，首先根据inodeID计算在哪个盘块上

    int blk_num = 2 + inode_id / (DISK_BLOCK_SIZE / DISKINODE_SIZE);

    #ifdef IS_DEBUG
        std::cout << "getDiskInode : " << blk_num << std::endl;
    #endif

    
    // Buf *p_buf = p_bufferCache->Bread(blk_num);
    // DiskInode *cur_diskInode = (DiskInode *)p_buf->b_addr;
    // DiskInode tempDiskInode = *(cur_diskInode + inode_id % (DISK_BLOCK_SIZE / DISKINODE_SIZE));
    // p_bufferCache->Brelse(p_buf);

    DiskBlock cur_diskBlock;
    Kernel::instance().getDiskDriver().readBlk(blk_num, &cur_diskBlock);
    const DiskInode *cur_diskInode = (DiskInode *)&cur_diskBlock;
    DiskInode tempDiskInode = *(cur_diskInode + inode_id % (DISK_BLOCK_SIZE / DISKINODE_SIZE));
    // std::cout << "ok " << std::endl;

    // inode_id = inode_id;

    // Buf *p_buf = p_bufferCache->Bread(INODE_ZONE_START_SECTOR + inode_id / INODE_NUMBER_PER_SECTOR);
	
	// unsigned char* p = (unsigned char*) p_buf->b_addr + (inode_id % INODE_NUMBER_PER_SECTOR) * sizeof(DiskInode);
	// DiskInode dInode;
    // memcpy(p, &dInode, sizeof(DiskInode));

    // std::cout << "GetDiskInode: " << INODE_ZONE_START_SECTOR + inode_id / INODE_NUMBER_PER_SECTOR << \
    //              " " <<(inode_id % INODE_NUMBER_PER_SECTOR) * sizeof(DiskInode) << "\n";
	// p_bufferCache->Brelse(p_buf); 

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
            //Kernel::instance().getBufferManager().Brelse(p_buf);
            break;
        } //ino==0表示该文件被删除
    }

    //Kernel::instance().getBufferManager().Brelse(p_buf);
    return ret;
}



VFS_Status VFS::getExt2Status(){
    return ext2_status;
}

VFS_Status VFS::setExt2Status(VFS_Status ext2_status){
    this->ext2_status = ext2_status;
}

