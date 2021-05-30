#include "Kernel.h"
#include "DiskDriver.h"
#include "Tools.h"

Kernel Kernel::kernelInstance;

Kernel::Kernel()
{
#ifdef IS_DEBUG
    std::cout << "Construct Kernel!!!!!!!!!!!!!!!!!!" << std::endl;
#endif

    bufferCache.setDiskDriver(&this->diskDriver);

    ext2.setBufferCache(&this->bufferCache);
    
}
Kernel& Kernel::instance()
{

    return Kernel::kernelInstance;
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

SuperBlock &Kernel::getSuperBlockCache()
{
    return superBlock;
}

InodeCache &Kernel::getInodeCache()
{
    return inodeCache;
}

User &Kernel::getUser()
{
    return my_user;
}

User::User(){
	curDirInodeId = 1;
}




void Kernel::mount(){

    /* 内存inodeCache初始化 */
    inodeCache.init();
    bufferCache.init();

    /* DiskDriver打开虚拟磁盘img，mmap，进入就绪状态 */
    Ext2_Status mount_img = diskDriver.mountImg();
     
    /* 设置当前的加载状态 */
    ext2.setExt2Status(mount_img);
    if(mount_img == Ext2_READY){
        Logcat::log("[INFO]镜像加载成功");
        /* 如果成功加载，就读入superblock，否则就是默认 */
        ext2.loadSuperBlock(Kernel::instance().getSuperBlockCache());
    }
    else if(mount_img == Ext2_NOFORM){
        Logcat::log("[INFO]您刚刚创建了一块新的镜像！");
        ext2.format();
        Logcat::log("[INFO]成功初始化");
    }
    else{
        Logcat::log("[ERROR]镜像加载失败");
    }
}


void Kernel::unmount(){
        /* 刷回InodeCache,SuperBlock */
        inodeCache.flushAllCacheDirtyInode();
        superBlock.writeBack();

        ext2.unregisterFs();
}

int Kernel::format(){
        switch (ext2.getExt2Status()){
        case Ext2_UNINITIALIZED:
            printf("[ERROR]磁盘装载错误\n");
            exit(-1);
            break;
        case Ext2_NOFORM:
            ext2.format();
            break;
        case Ext2_READY:
            printf("[WARNING] 磁盘可能已有数据，确定要格式化吗？\n");
            printf("Press \"y\" for 'format', \"n\" for 'cancel':");
            char temp_cmd;
            while (temp_cmd = getchar()){
                if (temp_cmd == 'y'){
                    ext2.format();
                    break;
                }
                else if (temp_cmd == 'n'){
                    return ERROR_CANCEL;
                    break;
                }
                else{
                    printf("Press \"y\" for 'format', \"n\" for 'cancel':");
                }
            }
            break;
        default:
            break;
        }
    return OK;
}
InodeId Kernel::createFile(const char *fileName){
    /*
     * @biref 在当前目录下创建文件(不管是文件还是文件夹)，文件名为fileName,
     * @return 返回新创建文件的inodeId
     */

    InodeId newFileInode = -1;

    /* 记录到该文件目录 */
    Path new_file_dir(fileName);

    /* 尚未存在路径 */
    InodeId file_folder_inode = ext2.locateDir(new_file_dir);
    if(file_folder_inode < 0){
        return ERROR_NO_FOLDER_EXSIT;
    }
    if((inodeCache.getInodeByID(file_folder_inode)->i_mode & Inode::IFMT) != Inode::IFDIR){
        return ERROR_NO_FOLDER_EXSIT;
    }

    /* 不重名 */
    InodeId file_inode = ext2.locateInode(new_file_dir);
    if (file_inode > 0){
        return ERROR_FILENAME_EXSIST;
    }

    /* 看看应该在哪里创建 */
    InodeId where_to_write;
    if(new_file_dir.level <= 1){
        /* 深度为0 如 touch a */
        where_to_write = getUser().curDirInodeId;
    }
    else{
        where_to_write = file_folder_inode;
    }

    /* 为新文件分配新inode */
    newFileInode = superBlock.ialloc(); //得到inode号

    if (newFileInode <= 0){
        return -1;
    }
    Inode *p_inode = inodeCache.getInodeByID(newFileInode); //并将这个inode写入inodeCache
    p_inode->newInode(Inode::IUPD | Inode::IACC, newFileInode);

    /* 在当前目录文件中写入新的目录项 */
    Inode *p_dirInode = inodeCache.getInodeByID(where_to_write); // 不是当前文件中
    int blkno = p_dirInode->Bmap(0); //Bmap查物理块号
    Buf *pBuf = bufferCache.Bread(blkno);

    DirectoryEntry *p_directoryEntry = (DirectoryEntry *)pBuf->b_addr;

    int i;
    for (i = 0; i < DISK_BLOCK_SIZE / sizeof(DirectoryEntry); i++)
    {
        if ((p_directoryEntry->m_ino == 0)) //找到目录文件中可以见缝插针的地方，填入创建的inode信息
        {
            p_directoryEntry->m_ino = newFileInode;
            strcpy(p_directoryEntry->m_name, new_file_dir.getInodeName());
            break;
        } //ino==0表示该文件被删除

        p_directoryEntry++;
    }
    if (i == DISK_BLOCK_SIZE / sizeof(DirectoryEntry))
    {
        return ERROR_NOTSPEC;
    }
    Kernel::instance().getBufferCache().Bdwrite(pBuf);
    //Kernel::instance().getBufferCache().Brelse(pBuf);
    return newFileInode;
}

/**
 * 创建目录
 */
int Kernel::mkDir(const char *dirName){

    /* 记录到该文件目录 */
    Path new_file_dir(dirName);
    /* 记录到该文件的父亲目录 */
    Path new_file_folder_dir(new_file_dir);

    int newDirInodeId = createFile(dirName);// 相当于先建立文件...
    if (newDirInodeId < 0){
        return newDirInodeId;
    }

    /* 然后把它改为文件夹 */
    Inode *p_inode = inodeCache.getInodeByID(newDirInodeId);
    p_inode->i_mode = Inode::IFDIR;

    DirectoryEntry tempDirectoryEntry;
    

    BlkNum blkno = p_inode->Bmap(0);
    Buf *pBuf = bufferCache.Bread(blkno);

    DirectoryEntry *p_directoryEntry = (DirectoryEntry *)pBuf->b_addr;

    strcpy(tempDirectoryEntry.m_name, ".");
    tempDirectoryEntry.m_ino = newDirInodeId;
    *p_directoryEntry = tempDirectoryEntry;
    p_directoryEntry++;


    /* mkdir a/b/c */
    InodeId where_to_write;
    if(new_file_folder_dir.level == 0){
        /* 深度为0 如 touch a */
        where_to_write = Kernel::instance().getUser().curDirInodeId;
    }
    else{
        InodeId file_folder_inode = ext2.locateInode(new_file_folder_dir);
        where_to_write = file_folder_inode;
    }
    strcpy(tempDirectoryEntry.m_name, "..");
    tempDirectoryEntry.m_ino = where_to_write; //错啦
    *p_directoryEntry = tempDirectoryEntry;


    Kernel::instance().getBufferCache().Bdwrite(pBuf);

    return OK;
}



InodeId Kernel::deleteFolder(const char *dirName){
    /*
     * @brief 递归删除文件夹
     * @return 
     */
    Path path(dirName);
    InodeId deleteFileInode = ext2.locateInode(path);
    if (deleteFileInode < 0){
        return deleteFileInode;
    }

    Inode *p_delete_inode = inodeCache.getInodeByID(deleteFileInode);
    Inode *p_dirInode = inodeCache.getInodeByID(ext2.locateDir(path));

    /* 如果要删除的是目录 */
    if ((p_delete_inode->i_mode & Inode::IFMT) == Inode::IFDIR){
        //递归删除该目录下的所有文件
        int blkno = p_delete_inode->Bmap(0); //Bmap查物理块号
        Buf *pBuf;
        pBuf = Kernel::instance().getBufferCache().Bread(blkno);

        /* 遍历目录 */
        DirectoryEntry *p_directoryEntry = (DirectoryEntry *)pBuf->b_addr;
        int i;
        for (i = 0; i < DISK_BLOCK_SIZE / sizeof(DirectoryEntry); i++){
            if ((p_directoryEntry->m_ino != 0)){ //找到目录文件中可以见缝插针的地方，填入创建的inode信息
                if (!strcmp(p_directoryEntry->m_name, ".") || !strcmp(p_directoryEntry->m_name, "..")){
                    continue;
                }
                else{
                    if ((inodeCache.getInodeByID(p_directoryEntry->m_ino)->i_mode & Inode::IFMT) == Inode::IFDIR){
                        deleteFolder(p_directoryEntry->m_name);
                    }
                    else{
                        deleteFile(p_directoryEntry->m_name);
                    }
                }

            } //ino==0表示该文件被删除

            p_directoryEntry++;
        }
        Kernel::instance().getBufferCache().Bdwrite(pBuf);
        //删除该目录本身
        deleteObject(dirName);
    }
    else
    {
        Logcat::log("[ERROR]不能删除文件");
        return ERROR_DELETE_FAIL;
    }
    return deleteFileInode;
}

/**
 * 删除文件
 */
InodeId Kernel::deleteFile(const char *fileName){

    //目录文件和普通文件要分别处理！
    Path path(fileName);
    InodeId deleteFileInode = ext2.locateInode(path);
    if (deleteFileInode < 0)
    {
        return deleteFileInode;
    }
    Inode *p_delete_inode = inodeCache.getInodeByID(deleteFileInode);
    Inode *p_dirInode = inodeCache.getInodeByID(ext2.locateDir(path));
    if ((p_delete_inode->i_mode & Inode::IFMT) == 0){ //普通文件
        return deleteObject(fileName);
    }
    else
    {
        Logcat::log("[ERROR]不能删除文件夹");
        return ERROR_DELETE_FAIL;
    }
}

/**
 * 直接删除
 */
InodeId Kernel::deleteObject(const char *fileName){
    Path path(fileName);
    InodeId deleteFileInode = ext2.locateInode(path);
    if (deleteFileInode < 0)
    {
        return ERROR_DELETE_FAIL;
    }

    Inode *p_delete_inode = inodeCache.getInodeByID(deleteFileInode);
    Inode *p_dirInode = inodeCache.getInodeByID(ext2.locateDir(path));

    BlkNum phyno;
    //Step1 释放盘块
    for (int lbn = 0; (phyno = p_delete_inode->Bmap(lbn)) <= 0; lbn++)
    {
        superBlock.bfree(phyno);
    }
    //Step2 删除目录项
    int dirblkno = p_dirInode->Bmap(0); //Bmap查物理块号
    Buf *pBuf;
    pBuf = Kernel::instance().getBufferCache().Bread(dirblkno);
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
    Kernel::instance().getBufferCache().Bdwrite(pBuf);
    //Step3 释放inode
    p_delete_inode->i_flag = 0; //这里是为了不再把删除的inode刷回，只用在superblock中标记inode删除即可
    superBlock.ifree(deleteFileInode);
    return deleteFileInode;
}


int Kernel::cd(const char *dirName)
{
    Path path(dirName);
    InodeId targetInodeId = ext2.locateInode(path);
    if (targetInodeId <= 0)
    {
        Logcat::log("目录查找失败！");
    }
    else if ((inodeCache.getInodeByID(targetInodeId)->i_mode & Inode::IFMT) != Inode::IFDIR)
    {
        Logcat::log("ERROR! cd 命令的参数必须是目录！");
    }
    else{
        Kernel::instance().getUser().curDirInodeId = targetInodeId;
    }

    return targetInodeId;
}

void Kernel::ls(InodeId dirInodeID)
{
    //首先要获得这个inode->访问这个目录文件
    //step1: 检查inodeCache中有没有，有则直接用，没有则向Ext2模块要
    Inode &inode = *inodeCache.getInodeByID(dirInodeID);
    if (inode.i_mode & Inode::IFMT != Inode::IFDIR)
    {
        printf("[ERROR]非法的参数\n");
        return;
    }

    inode.i_flag |= Inode::IACC;
    //Step2：读这个目录文件到缓存块中（可能已经存在于缓存块中,规定目录文件不能超过4096B）
    int blkno = inode.Bmap(0); //Bmap查物理块号
    Buf *pBuf;
    pBuf = Kernel::instance().getBufferCache().Bread(blkno);
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
    Kernel::instance().getBufferCache().Brelse(pBuf);
}

void Kernel::ls(const char *dirName)
{
    //首先要根据目录名，确定inode号
    //step1 在DirectoryEntry中查找有没有现成的
    InodeId dirInodeId;

    Path path(dirName); //解析dirName转化为Path对象
    //先查一下directoryCache中有没有存dirName的目录项
    //TODO 先暂时不做VFS层的dentry缓存

    //没有，则向Ext模块要
    dirInodeId = ext2.locateInode(path);
    if ((inodeCache.getInodeByID(dirInodeId)->i_mode & Inode::IFMT) == Inode::IFDIR)
    {
        ls(dirInodeId);
    }
    else
    {
        Logcat::log("[ERROR]操作对象不可为文件");
    }
}

/**
 * 打开一个普通文件,返回文件的句柄
 */
FileFd Kernel::open(const Path& path, int mode)
{
    FileFd fd;
   /*先找inode*/
    InodeId openFileInodeId = ext2.locateInode(path);

    // 返回-1 说明打开失败
    if(openFileInodeId < 0){
        return -1;
    }
    //Step2. 检查打开合法性(省略了文件本身读写的限定)
    Inode *p_inodeOpenFile = inodeCache.getInodeByID(openFileInodeId);
    if (p_inodeOpenFile->i_mode & Inode::IFMT != 0)
    {
        return ERROR_OPEN_ILLEGAL; //在本程序中，只有普通文件可以open
    }
    p_inodeOpenFile->i_flag |= Inode::IACC;
    //Step3. 分配FILE结构
    File *pFile = Kernel::instance().m_OpenFileTable.FAlloc();
    if (pFile == NULL)
    {
        //分配失败
        return ERROR_OUTOF_OPENFILE;
    }
    //Step4. 建立钩连关系,u_ofile[]中的一项指向FILE
    User &u = Kernel::instance().getUser();
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


int Kernel::close(FileFd fd)
{

    User &u = Kernel::instance().getUser();

    /* 获取打开文件控制块File结构 */
    File *pFile = u.u_ofiles.GetF(fd);
    if (NULL == pFile)
    {
        return ERROR_CLOSE_FAIL;
    }

    /* 释放打开文件描述符fd，递减File结构引用计数 */
    u.u_ofiles.SetF(fd, NULL);
    Kernel::instance().m_OpenFileTable.CloseF(pFile);
    return OK;
}

/**
 * 从文件fd中读出length字节放到content缓冲区中。
 * 返回读出的字节数，如果fd剩下的字节小于length，则只把剩下的读出
 */
int Kernel::read(int fd, uint8_t *content, int length)
{
    //分析：length可能大于、小于、等于盘块的整数倍
    int readByteCount = 0;

    User &u = Kernel::instance().getUser();
    File *p_file = u.u_ofiles.GetF(fd);
    Inode *p_inode = inodeCache.getInodeByID(p_file->f_inode_id);
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
        pBuf = Kernel::instance().getBufferCache().Bread(phyBlkno);
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
        Kernel::instance().getBufferCache().Brelse(pBuf);
    }

    return readByteCount;
}
int Kernel::write(int fd, uint8_t *content, int length)
{
    //分析：length可能大于、小于、等于盘块的整数倍
    int writeByteCount = 0;

    User &u = Kernel::instance().getUser();

    File *p_file = u.u_ofiles.GetF(fd);

    Inode *p_inode = inodeCache.getInodeByID(p_file->f_inode_id);
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
            pBuf = Kernel::instance().getBufferCache().GetBlk(phyBlkno);
        }
        else
        {
            //先读后写
            pBuf = Kernel::instance().getBufferCache().Bread(phyBlkno);
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
        Kernel::instance().getBufferCache().Bdwrite(pBuf);
    }

    return writeByteCount;
}

/**
 * 判断是否到达文件尾部
 */
bool Kernel::eof(FileFd fd)
{
    User &u = Kernel::instance().getUser();
    File *p_file = u.u_ofiles.GetF(fd);
    Inode *p_inode = inodeCache.getInodeByID(p_file->f_inode_id); //TODO错误处理?
    if (p_file->f_offset == p_inode->i_size + 1)
        return true;
    else
        return false;
}
