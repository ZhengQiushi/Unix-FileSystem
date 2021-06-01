#include "BufferCache.h"
#include "Kernel.h"
#include "Tools.h"


void BufferCache::setDiskDriver(DiskDriver *my_diskDriver){
    this->diskDriver = my_diskDriver;
}

void BufferCache::unmount(){
    Bflush();
}


void BufferCache::init(){
    int i;
    Buf *bp;

    this->bFreeList.b_forw = this->bFreeList.b_back = &(this->bFreeList);
    this->bFreeList.av_forw = this->bFreeList.av_back = &(this->bFreeList);

    for (i = 0; i < BUFFER_CACHE_NUM; i++){
        bp = &(this->m_Buf[i]);
        bp->b_blkno = -1;
        bp->b_dev = -1;
        bp->b_addr = &Buffer[i];

        bp->b_flags = Buf::B_BUSY;
        Brelse(bp); //放入自由缓存队列
    }
}


Buf *BufferCache::Bread(int blk_num){
    /**
     * @brief 将指定物理块号的数据块BUF从缓存队列中读出
     */
    Buf *bp;
    /* 根据块号申请缓存 */
    bp = this->GetBlk(blk_num);

    /* 如果在设备队列中找到所需缓存，即B_DONE已设置，就不需进行I/O操作 */
    if (bp->b_flags & Buf::B_DONE){
        //说明太好了，有现成的缓存块（副本和磁盘一致）
        return bp; 
    }

    /* 没有找到相应缓存，构成I/O读请求块 */
    bp->b_flags |= Buf::B_READ;
    bp->b_wcount = DISK_BLOCK_SIZE;

    diskDriver->readBlk(blk_num, bp->b_addr);
    
    bp->b_flags |= Buf::B_DONE;

    return bp;
}
void BufferCache::Bclear(Buf *bp) {
    memset(bp->b_addr, 0, DISK_BLOCK_SIZE);
	return;
}

void BufferCache::Bwrite(Buf *bp){
    /**
     * @brief 将一个缓存块中的东西写回磁盘
     */
    unsigned int flags;

    flags = bp->b_flags;
    bp->b_flags &= ~(Buf::B_READ | Buf::B_DONE | Buf::B_ERROR | Buf::B_DELWRI);
    bp->b_wcount = DISK_BLOCK_SIZE;

    if ((flags & Buf::B_DELWRI) == 0){
        std::cout << "[ERROR]bwrite出错" << std::endl;
    }
    else{
        diskDriver->writeBlk(bp->b_blkno, *bp->b_addr);
        bp->b_flags |= Buf::B_DONE;
    }

    return;
}


void BufferCache::Bdwrite(Buf *bp){
    /**
     * @brief 延迟写磁盘块
     */
    /* 置上B_DONE允许其它进程使用该磁盘块内容 */
    bp->b_flags |= (Buf::B_DELWRI | Buf::B_DONE);
    this->Brelse(bp);
    return;
}


void BufferCache::Bflush(){
    /**
     * @brief 将dev指定设备队列中延迟写的缓存全部输出到磁盘.需要全部刷回
     */
    Buf *bp;

    for (bp = this->bFreeList.b_forw; bp != &(this->bFreeList); bp = bp->b_forw){
        /* 找出自由队列中所有延迟写的块 */
        if ((bp->b_flags & Buf::B_DELWRI)){
            this->Bwrite(bp);
        }
    }
    return;
}


Buf *BufferCache::GetBlk(int blk_num){
    /**
     * @brief 申请一块缓存，用于读写设备dev上的字符块blkno
     */
    Buf *bp;

    User &u = Kernel::instance().getUser();

    /* 首先在该设备队列中搜索是否有相应的缓存 */
    for (bp = bFreeList.b_forw; bp != &bFreeList; bp = bp->b_forw){
        /* 不是要找的缓存，则继续 */
        if (bp->b_blkno != blk_num)
            continue;

        /* 从自由队列中抽取出来 */
        this->getFetched(bp);
        return bp; //可能会有延迟写标志
    }

    /* 取自由队列第一个空闲块 */
    bp = this->bFreeList.av_forw;
    if(bp == &this->bFreeList){
        
    }
    this->getFetched(bp);


    /* 如果该字符块是延迟写，将其异步写到磁盘上 */
    if (bp->b_flags & Buf::B_DELWRI){
        this->Bwrite(bp);
    }

    /* 注意: 这里清除了所有其他位，只设了B_BUSY */
    bp->b_flags = Buf::B_BUSY; //若有延迟写bit，也一并消除了
    bp->b_blkno = blk_num;
    memset(bp->b_addr, 0, DISK_BLOCK_SIZE);
    
    if (bp->b_dev != devno){
        //加入设备缓存队列
        bp->b_back = &(this->bFreeList);
        bp->b_forw = this->bFreeList.b_forw;
        this->bFreeList.b_forw->b_back = bp;
        this->bFreeList.b_forw = bp;
        bp->b_dev = devno;
    }
    
    return bp;
}



void BufferCache::getFetched(Buf *bp){
    /**
     * @brief 从队列中取出
     */
    /* 从自由队列中取出 */
    bp->av_back->av_forw = bp->av_forw;
    bp->av_forw->av_back = bp->av_back;
    /* 设置B_BUSY标志 */
    bp->b_flags |= Buf::B_BUSY;
    return;
}


void BufferCache::Brelse(Buf *bp){
    bp->b_flags &= ~(Buf::B_WANTED | Buf::B_BUSY | Buf::B_ASYNC); //消除这些位的符号（这里其实没用到）
    (this->bFreeList.av_back)->av_forw = bp;
    bp->av_back = this->bFreeList.av_back;
    bp->av_forw = &(this->bFreeList);
    this->bFreeList.av_back = bp;
    return;
}
