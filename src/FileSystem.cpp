#include "FileSystem.h"
#include "Kernel.h"
#include "Tools.h"
#include "DiskDriver.h"

SuperBlock::SuperBlock(): disk_block_bitmap(DISK_SIZE / DISK_BLOCK_SIZE){
    //总inode数  -1是因为0#inode不可用
    total_inode_num = MAX_INODE_NUM - 1; 
    //空闲inode
    free_inode_num = total_inode_num;    
    //初始化空闲inode栈
    for (int i = 0; i < total_inode_num; i++){
        s_inode[i] = total_inode_num - i;
    }
}

void SuperBlock::writeBackSuper(){
    /** 
     * @brief 回写superblock
     */
    Buf *p_buf = Kernel::instance().getBufferManager().GetBlk(0);
    SuperBlock *p_superBlock = (SuperBlock *)p_buf->b_addr;
    *p_superBlock = *this;
    Kernel::instance().getBufferManager().Bdwrite(p_buf);
}


BlkId SuperBlock::balloc(){
    /**
     * @brief 分配一个空闲盘块号
     */
    int ret = disk_block_bitmap.getFreeBitId();
    disk_block_bitmap.setBit(ret);
    free_block_bum--;
    return ret;
}


void SuperBlock::bfree(BlkId blknum){
    /**
     * @brief 回收一个盘块
     */
    free_block_bum++;
    disk_block_bitmap.unsetBit(blknum);
}

void SuperBlock::ballocCeratin(BlkId blk_num){
    /**
     * @brief 分配指定的盘块
     */
    if (disk_block_bitmap.isAvai(blk_num)){

    }
    else{
        disk_block_bitmap.setBit(blk_num);
        free_block_bum--;
    }
}


InodeId SuperBlock::ialloc(){
    /**
     * @brief 分配空闲inode
     */
    if (free_inode_num != 0){
        
        return s_inode[--free_inode_num];
    }
    else{
        return ERROR_OUTOF_INODE;
    }
}


void SuperBlock::ifree(InodeId inode_id){
    /**
     * @brief 回收inode
     */
    s_inode[free_inode_num++]=inode_id;
}


InodeBlock::InodeBlock() : inode_bitmap(MAX_INODE_NUM){
}


int InodeBlock::ialloc(){
    /**
    * @brief 分配一个空闲inode号
    */
    int ret = inode_bitmap.getFreeBitId();
    inode_bitmap.setBit(ret);
    return ret;
}


void InodeBlock::ifree(InodeId InodeID){
    /**
    * @brief 回收一个inode
    */
    inode_bitmap.unsetBit(InodeID);
}


DiskInode *InodeBlock::getInode(InodeId InodeID){
    /**
    * @brief 根据InodeID获取一个Inode结构
    */
    return &inode_block[InodeID];
}


void InodeBlock::iupdate(InodeId inode_id,DiskInode disk_inode){
    /**
     * @brief 将某个制定inode号的inode更新为diskInode的内容
     */
    inode_bitmap.setBit(inode_id);   
    // 置位inode_id为disk_inode
    inode_block[inode_id]=disk_inode;
}

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

    /* 磁盘分配情况
     * 0# superblock
     * 1,2,3# inodePool
     * 4~DISK_BLOCK_NUM-1# 放数据
     */

    /* 清空block */
    DiskBlock *diskMemAddr = Kernel::instance().getDiskDriver().getDiskMemAddr();
    memset(diskMemAddr, 0, DISK_SIZE);

    /* 初始化superblock  */
    SuperBlock tempSuperBlock;
    tempSuperBlock.total_block_num = DISK_BLOCK_NUM;
    tempSuperBlock.free_block_bum = DISK_BLOCK_NUM;

    /* 0  superblock  */
    tempSuperBlock.ballocCeratin(0); 

    /* 1~3 inodeCache */
    tempSuperBlock.ballocCeratin(1);
    tempSuperBlock.ballocCeratin(2);
    tempSuperBlock.ballocCeratin(3); 

    /* 4 数据 */ 
    tempSuperBlock.ballocCeratin(4); 
    tempSuperBlock.free_inode_num -= 1;

    /* 写回superblock */
    SuperBlock *p_superBlock = (SuperBlock *)diskMemAddr;
    *p_superBlock = tempSuperBlock; 
    p_superBlock++;
    diskMemAddr = (DiskBlock *)p_superBlock;
    /* 同步到kernel */
    Kernel::instance().getSuperBlock() = tempSuperBlock;

    /* 初始化inodeCache */
    InodeBlock tempInodePool;
    int tempAddr[10] = {4, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    DiskInode tempDiskInode = DiskInode(Inode::IFDIR, 1, 1, 1, 0, tempAddr);// 6 * sizeof(DirectoryEntry)
    tempInodePool.iupdate(1, tempDiskInode);
    
    /* 写入inodeCache */
    InodeBlock *p_InodePool = (InodeBlock *)diskMemAddr;
    *p_InodePool = tempInodePool;
    p_InodePool++;
    diskMemAddr = (DiskBlock *)p_InodePool;

    /* 写入数据 */
    DirectoryEntry *p_directoryEntry = (DirectoryEntry *)diskMemAddr;
    DirectoryEntry tempDirctoryEntry;
    strcpy(tempDirctoryEntry.m_name, ".");
    tempDirctoryEntry.m_ino = 1;
    *p_directoryEntry = tempDirctoryEntry;
    p_directoryEntry++;
    strcpy(tempDirctoryEntry.m_name, "..");
    tempDirctoryEntry.m_ino = 1;
    *p_directoryEntry = tempDirctoryEntry;
    p_directoryEntry++;

    /* 初始化当前路径 */
    Kernel::instance().getUser().cur_path = "/";
    ext2_status = VFS_READY;
}


void VFS::loadSuperBlock(SuperBlock &superBlock){
    /**
     * @brief 重新读出superBlock
     */
    Buf *p_buf;
    p_buf = p_bufferCache->Bread(0);
    memcpy(&superBlock, p_buf->b_addr, DISK_BLOCK_SIZE);
    p_bufferCache->Brelse(p_buf);
}

int VFS::setBufferCache(BufferCache *bufferCache){
    this->p_bufferCache = bufferCache;
    return OK;
}


void VFS::writeBackDiskInode(int inode_id, DiskInode disk_inode){ 
    /**
     * @brief 将diskInode重新写回磁盘中
     * @notice 要先读后写!
     */
    // 定位到第二个inode pool
    int blk_num = 2 + inode_id / (DISK_BLOCK_SIZE / DISKINODE_SIZE);

    Buf *p_buf = p_bufferCache->Bread(blk_num);
    DiskInode *cur_diskInode = (DiskInode *)p_buf->b_addr;
    cur_diskInode = cur_diskInode + inode_id % (DISK_BLOCK_SIZE / DISKINODE_SIZE);
    //定位到需要写diskInode的位置
    *cur_diskInode = disk_inode;     
    p_bufferCache->Bdwrite(p_buf); 
}

DiskInode VFS::getDiskInodeById(int inode_id){
    //inode分布在两个盘块上，首先根据inodeID计算在哪个盘块上
    int blk_num = 2 + inode_id / (DISK_BLOCK_SIZE / DISKINODE_SIZE);

    Buf *p_buf = p_bufferCache->Bread(blk_num);
    DiskInode *cur_diskInode = (DiskInode *)p_buf->b_addr;
    DiskInode tempDiskInode = *(cur_diskInode + inode_id % (DISK_BLOCK_SIZE / DISKINODE_SIZE));
    p_bufferCache->Brelse(p_buf);
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
        dirInode = getInodeIdInDir(dirInode, path.path[i].c_str());
        if (dirInode < 0){
            return ERROR_PATH_NFOUND; //没有找到
        }
    }
    return dirInode;
}

InodeId VFS::getInodeIdInDir(InodeId par_inode_id, FileName fileName){ 
    /**
     * @brief 在当前目录下， 找指定文件名的文件
     * @return 找到了就返回该文件的Inode，不然就返回-1 
     */

    //先根据目录inode号dirInodeId获得目录inode对象
    Inode *p_dir_inode = Kernel::instance().getInodeCache().getInodeByID(par_inode_id);

    //读取该inode指示的数据块
    int blk_num = p_dir_inode->Bmap(0); //Bmap查物理块号

    Buf *p_buf = Kernel::instance().getBufferManager().Bread(blk_num);
    DirectoryEntry *p_directoryEntry = (DirectoryEntry *)p_buf->b_addr;

    //访问这个目录文件中的entry
    for (int i = 0; i < DISK_BLOCK_SIZE / sizeof(DirectoryEntry); i++){
        if ((p_directoryEntry->m_ino != 0) && (!strcmp(p_directoryEntry->m_name, fileName))){
            return p_directoryEntry->m_ino;
        } //ino==0表示该文件被删除
        p_directoryEntry++;
    }

    Kernel::instance().getBufferManager().Brelse(p_buf);
    return -1;
}



VFS_Status VFS::getExt2Status(){
    return ext2_status;
}

VFS_Status VFS::setExt2Status(VFS_Status ext2_status){
    this->ext2_status = ext2_status;
}

