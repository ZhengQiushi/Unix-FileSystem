#include "DiskDriver.h"
#include "Tools.h"
#include "DiskDriver.h"

DiskDriver::DiskDriver()
{
   TAG = strdup("DiskDriver");
}
DiskDriver::~DiskDriver()
{
   if (TAG != nullptr)
      delete TAG;
}

/**
 * NOTE DiskDriver的mount和VFS的mount不一样,VFS还要做更多。
 * 磁盘安装的过程，superblock的刷入，在ext中执行。
 * 这里做的是磁盘镜像文件的打开和内存映射。感谢神奇的mmap
 * 打开img文件,如果没有就尝试创建
 * @return: 如果返回-1表示mount失败，返回0表示有现成的img文件，
 * 返回1表示img文件新生成
 *  */
Ext2_Status DiskDriver::mountImg()
{ 
  

   Ext2_Status retVal = Ext2_UNINITIALIZED;
   //打开img文件,如果没有就尝试创建
   DiskFd = open(DISK_IMG_FILEPATH, O_RDWR | O_CREAT, DEF_MODE);
   if (DiskFd == -1){
      retVal = Ext2_UNINITIALIZED;
      exit(-1);
   }

   /**
    * 新创建的文件，需要做扩容操作，扩大到指定虚拟磁盘大小
    */
   if (lseek(DiskFd, 0, SEEK_END) < DISK_SIZE){
      //说明是新创建的文件，需要改变文件的大小
      lseek(DiskFd, 0, SEEK_SET);
      ftruncate(DiskFd, DISK_SIZE);
      retVal = Ext2_NOFORM;
   }
   else{
      retVal = Ext2_READY;
   }

   /**
    * mmap这是一个非常神奇的系统调用，将一个文件或者其它对象映射进内存。
    * 当使用mmap映射文件到进程后,就可以直接操作这段虚拟地址进行文件的读写等操作,不必再调用read,write等系统调用。
    * 但需注意,直接对该段内存写时不会写入超过当前文件大小的内容.
    */
   DiskMemAddr = (DiskBlock *)mmap(nullptr, DISK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, DiskFd, 0);
   if (DiskMemAddr == MAP_FAILED){
      #ifdef IS_DEBUG
         Logcat::log("[ERROR]镜像映射失败");
      #endif
      exit(-1);
      retVal = Ext2_UNINITIALIZED;
   }
   else{
      #ifdef IS_DEBUG
         Logcat::log("[INFO]镜像映射成功");
      #endif
   }

   return retVal;
}

/**
 * DiskDriver的unmount所做的工作为：
 * 结束文件的内存映射，关闭磁盘镜像文件。
 * NOTE 注意！磁盘的卸载需要脏缓存的刷回、脏inode的写回和SuperBlock的写回。
 * 应当在本unmount之前完成。
 * 
 */
void DiskDriver::unmount()
{
   if (munmap(DiskMemAddr, DISK_SIZE) == -1)
   {
      Logcat::log(TAG, "Munmap失败！");
      return;
   }
   else
   {
      Logcat::log(TAG, "Munmap成功！");
      close(DiskFd);
      isMounted = false;
   }
}
DiskBlock *DiskDriver::getBlk(int blockNum)
{
   return DiskMemAddr + blockNum;
}
void DiskDriver::readBlk(int blockNum, DiskBlock *dst)
{
   memcpy(dst, DiskMemAddr + blockNum, DISK_BLOCK_SIZE);
}
void DiskDriver::writeBlk(int blockNum, const DiskBlock &blk)
{
   memcpy(DiskMemAddr + blockNum, &blk, DISK_BLOCK_SIZE);
}
bool DiskDriver::isDiskMounted()
{
   return isMounted;
}

DiskBlock *DiskDriver::getDiskMemAddr()
{
   return DiskMemAddr;
}