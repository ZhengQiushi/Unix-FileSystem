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

// SuperBlock::SuperBlock(const SuperBlock& superBlock): disk_block_bitmap(DISK_SIZE / DISK_BLOCK_SIZE){
//     this->disk_block_bitmap = superBlock.disk_block_bitmap;
//     this->free_block_bum = superBlock.free_block_bum;
//     this->free_inode_num = superBlock.free_inode_num;
//     this->total_block_num = superBlock.total_block_num;
//     this->total_inode_num = superBlock.total_inode_num;
//     this->superBlock_block_num = superBlock.superBlock_block_num;
//     memcpy(this->s_inode, superBlock.s_inode, sizeof(superBlock.s_inode));
// }

void SuperBlock::writeBackSuper(){
    /* 找到第一块 */
    Buf *pBuf = Kernel::instance().getBufferManager().GetBlk(0);
    SuperBlock *p_superBlock = (SuperBlock *)pBuf->b_addr;
    *p_superBlock = *this;
    Kernel::instance().getBufferManager().Bdwrite(pBuf);



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

/**
 * 将内存inode刷回磁盘的时候，需要用这个进行构造
 * NOTE: DiskInode的d_atime和d_mtime属性，不是从内存inode中转换的，而是根据系统当前时间。
 */
DiskInode::DiskInode(Inode inode)
{
	d_mode = inode.i_mode;
	d_nlink = inode.i_nlink;
	d_uid = inode.i_uid;
	d_gid = inode.i_gid;
	d_size = inode.i_size;
	memcpy(d_addr, inode.i_addr, MIXED_ADDR_TABLE_SIZE);

} //转换构造函数

void VFS::format()
{
    
    Kernel::instance().getInodeCache().init();
    Kernel::instance().getBufferManager().init();
    // 0# superblock
    // 1,2,3# inodePool
    // 4~DISK_BLOCK_NUM-1# 放数据
    DiskBlock *diskMemAddr = Kernel::instance().getDiskDriver().getDiskMemAddr();
    memset(diskMemAddr, 0, DISK_SIZE);

    //①构造一个superBlock结构，写入磁盘中
    SuperBlock tempSuperBlock;
    tempSuperBlock.total_block_num = DISK_BLOCK_NUM;
    tempSuperBlock.free_block_bum = DISK_BLOCK_NUM;

    tempSuperBlock.ballocCeratin(0); //0#盘块被superblock占据

    tempSuperBlock.ballocCeratin(1);
    tempSuperBlock.ballocCeratin(2);
    tempSuperBlock.ballocCeratin(3); //1~3#盘块被inodePool占据(即磁盘Inode区)

    tempSuperBlock.ballocCeratin(4); //4#盘块放根目录文件
    tempSuperBlock.free_inode_num -= 1;

    SuperBlock *p_superBlock = (SuperBlock *)diskMemAddr;
    *p_superBlock = tempSuperBlock; //没有动态申请，不用管深浅拷贝
    p_superBlock++;
    diskMemAddr = (DiskBlock *)p_superBlock;
    //还要送一份到VFS中
    Kernel::instance().getSuperBlock() = tempSuperBlock;

    //②构造DiskInode,修改InodePool,将InodePool写入磁盘img
    //1#inode，是根目录
    InodeBlock tempInodePool;
    int tempAddr[10] = {4, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    DiskInode tempDiskInode = DiskInode(Inode::IFDIR, 1, 1, 1, 0, tempAddr);// 6 * sizeof(DirectoryEntry)
    tempInodePool.iupdate(1, tempDiskInode);
    
    InodeBlock *p_InodePool = (InodeBlock *)diskMemAddr;
    *p_InodePool = tempInodePool;
    p_InodePool++;
    diskMemAddr = (DiskBlock *)p_InodePool;

    //③数据区写入目录文件
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

    Kernel::instance().getUser().cur_path = "/";
    ext2_status = VFS_READY;
}

int VFS::unregisterFs()
{
    
    ext2_status = VFS_UNINITIALIZED;
    return OK;
}

void VFS::loadSuperBlock(SuperBlock &superBlock){
    //User &u = Kernel::instance().getUser();
    Buf *pBuf;
    pBuf = p_bufferCache->Bread(0);
    memcpy(&superBlock, pBuf->b_addr, DISK_BLOCK_SIZE);
    p_bufferCache->Brelse(pBuf);
}

int VFS::setBufferCache(BufferCache *bufferCache){
    this->p_bufferCache = bufferCache;
    return OK;
}
int VFS::allocNewInode()
{
    return OK;
}

void VFS::writeBackDiskInode(int inode_id, DiskInode disk_inode){ 
    /*
     * @brief 将diskInode重新写回磁盘中
     * @notice 要先读后写!
     */
    // 定位到第二个inode pool
    int blk_num = 2 + inode_id / (DISK_BLOCK_SIZE / DISKINODE_SIZE);

    Buf *pBuf = p_bufferCache->Bread(blk_num);
    DiskInode *cur_diskInode = (DiskInode *)pBuf->b_addr;
    cur_diskInode = cur_diskInode + inode_id % (DISK_BLOCK_SIZE / DISKINODE_SIZE);
    //定位到需要写diskInode的位置
    *cur_diskInode = disk_inode;     //更新DiskInode
    p_bufferCache->Bdwrite(pBuf); 
}
//从disk中读取出一个制定inodeID的DiskInode.可能涉及io了（不确定在不在缓存中）
DiskInode VFS::getDiskInodeByNum(int inode_id)
{
    //inode分布在两个盘块上，首先根据inodeID计算在哪个盘块上
    int blk_num = 2 + inode_id / (DISK_BLOCK_SIZE / DISKINODE_SIZE);
    Buf *pBuf;
    pBuf = p_bufferCache->Bread(blk_num);
    DiskInode *cur_diskInode = (DiskInode *)pBuf->b_addr;
    DiskInode tempDiskInode;
    tempDiskInode = *(cur_diskInode + inode_id % (DISK_BLOCK_SIZE / DISKINODE_SIZE));
    p_bufferCache->Brelse(pBuf);
    return tempDiskInode; //外部可能会调用DiskInode的拷贝构造函数
}


InodeId VFS::locateInode(const myPath& path){
    /**
     * @brief 根据路径，做线性目录搜索
     * @return 该文件的InodeId
     * 
     */
    //先确定其父目录的inode号
    InodeId dirInodeId = locateParDir(path); 

    //std::cout <<path.getLevel() << " " <<  dirInodeId << "\n";

    if (path.getLevel() < 0){
        // cd / (此时getLevel == -1)
        return ROOT_INODE_ID;
    }
    else{
        return getInodeIdInDir(dirInodeId, path.getInodeName().c_str());
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
        //if(dirInode)
        dirInode = getInodeIdInDir(dirInode, path.path[i].c_str());
        if (dirInode < 0){
            return ERROR_PATH_NFOUND; //没有找到
        }
    }

    return dirInode;
}

/**
 * 线性目录搜索的步骤：根据文件名在目录文件（已知inode号）中查找inode号。
 * 可以知道目录搜索的起点一定是已知的inode号，要么是cur要么是ROOT
 */
InodeId VFS::getInodeIdInDir(InodeId dirInodeId, FileName fileName)
{ 
    /**
     * @brief 在当前目录下， 找指定文件名的文件
     * @return 找到了就返回该文件的Inode，不然就返回-1 
     */

    //Step1:先根据目录inode号dirInodeId获得目录inode对象
    Inode *p_dirInode = Kernel::instance().getInodeCache().getInodeByID(dirInodeId);
    //TODO 错误处理

    
    //Step2：读取该inode指示的数据块
    int blk_num = p_dirInode->Bmap(0); //Bmap查物理块号

    Buf *pBuf;

    pBuf = Kernel::instance().getBufferManager().Bread(blk_num);
    DirectoryEntry *p_directoryEntry = (DirectoryEntry *)pBuf->b_addr;
    //Step3：访问这个目录文件中的entry，搜索（同时缓存到dentryCache中）
    //TODO 缓存到dentryCache中
    for (int i = 0; i < DISK_BLOCK_SIZE / sizeof(DirectoryEntry); i++)
    {
        if ((p_directoryEntry->m_ino != 0) && (!strcmp(p_directoryEntry->m_name, fileName)))
        {
            return p_directoryEntry->m_ino;
        } //ino==0表示该文件被删除
        p_directoryEntry++;
    }

    Kernel::instance().getBufferManager().Brelse(pBuf);
    return -1;
}

int VFS::bmap(int inodeNum, int logicBlockNum)
{

    return OK;
} //文件中的地址映射。查混合索引表，确定物理块号。
  //逻辑块号bn=u_offset/512

VFS_Status VFS::getExt2Status()
{
    return ext2_status;
}

VFS_Status VFS::setExt2Status(VFS_Status ext2_status)
{
    this->ext2_status = ext2_status;
}

