#include "Kernel.h"
#include "User.h"
#include "VirtualProcess.h"
#include "Path.h"
VirtualProcess VirtualProcess::instance;

VirtualProcess::VirtualProcess()
{
    //TODO 需要初始化VirtualProcess
    //strcpy(defaultUser.u_curdir, "/");
    //defaultUser.u_curDir = Path("/");
    defaultUser.curDirInodeId = 1;
}

VirtualProcess::~VirtualProcess()
{
    //TODO 需要初始化VirtualProcess
}

VirtualProcess *VirtualProcess::Instance()
{
    return &VirtualProcess::instance;
}

User &VirtualProcess::getUser()
{
    return defaultUser;
}

/* 获取用户ID，低16比特为真实用户ID(u_ruid)，高16比特为有效用户ID(u_uid) */
short VirtualProcess::Getuid()
{
    return defaultUser.u_uid;
}

/* 获取组ID, 低16比特为真实组ID(u_rgid)，高16比特为有效组ID(u_gid) */
short VirtualProcess::Getgid()
{
    return defaultUser.u_gid;
}
Kernel Kernel::kernelInstance;

Kernel::Kernel()
{
#ifdef IS_DEBUG
    std::cout << "Construct Kernel" << std::endl;
#endif
    vfs.registerExt2(&ext2);
    vfs.bindDirectoryInodeCache(&directoryCache);
    vfs.bindInodeCache(&inodeCache);
    vfs.bindSuperBlockCache(&superBlockCache);
    ext2.setBufferCache(&bufferCache);
    bufferCache.setDiskDriver(&diskDriver);
}
Kernel *Kernel::instance()
{

    return &Kernel::kernelInstance;
}
VFS &Kernel::getVFS()
{
    return vfs;
}
Ext2 &Kernel::getExt2()
{
    return ext2;
}
DiskDriver &Kernel::getDiskDriver()
{
    return diskDriver;
}
BufferCache &Kernel::getBufferCache()
{
    return bufferCache;
}

SuperBlockCache &Kernel::getSuperBlockCache()
{
    return superBlockCache;
}

InodeCache &Kernel::getInodeCache()
{
    return inodeCache;
}
