#include "Kernel.h"
#include "DiskDriver.h"
#include "Tools.h"

Kernel Kernel::kernelInstance;

Kernel::Kernel()
{
#ifdef IS_DEBUG
    std::cout << "Construct Kernel" << std::endl;
#endif
    bufferCache.setDiskDriver(&this->diskDriver);
    fileSystem.setBufferCache(&this->bufferCache);
}
Kernel& Kernel::instance(){
    return Kernel::kernelInstance;
}

VFS &Kernel::getFileSystem(){
    return fileSystem;
}

DiskDriver &Kernel::getDiskDriver(){
    return diskDriver;
}

BufferCache &Kernel::getBufferManager(){
    return bufferCache;
}

SuperBlock &Kernel::getSuperBlock(){
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
    cur_path = "/";
}

void Kernel::initKernel(){

    /* 内存inodeCache初始化 */
    inodeCache.init();
    bufferCache.init();

    /* DiskDriver打开虚拟磁盘img，mmap，进入就绪状态 */
    VFS_Status mount_img = diskDriver.mountImg();
     
    /* 设置当前的加载状态 */
    fileSystem.setExt2Status(mount_img);
    if(mount_img == VFS_READY){
        std::cout << "[INFO]镜像加载成功" << std::endl;
        /* 如果成功加载，就读入superblock，否则就是默认 */
        fileSystem.loadSuperBlock(Kernel::instance().getSuperBlock());
    }
    else if(mount_img == VFS_NOFORM){
        std::cout << "[INFO]您刚刚创建了一块新的镜像！" << std::endl;
        fileSystem.format();
        std::cout << "[INFO]成功初始化" << std::endl;
    }
    else{
        std::cout << "[ERROR]镜像加载失败" << std::endl;
    }
}


void Kernel::relsKernel(){
    /* 刷回InodeCache,SuperBlock */
    superBlock.writeBackSuper();

    inodeCache.writeBackInode();
    bufferCache.Bflush();


    diskDriver.unmount();
}

void Kernel::my_man(std::string cur_inst) {
    auto it = my_man_map.find(cur_inst);
    if (it == my_man_map.end()) {
        std::cout << "[ERROR]似乎并不存在 '" << cur_inst << "' 这条指令... \n";
        return;
    }
    std::cout << *it->second << std::endl;
}

int Kernel::format(){
        switch (fileSystem.getExt2Status()){
        case VFS_UNINITIALIZED:
            std::cout << ("[ERROR]磁盘装载错误")<< std::endl;
            exit(-1);
            break;
        case VFS_NOFORM:
            fileSystem.format();
            break;
        case VFS_READY:
            std::cout << ("[WARNING] 磁盘可能已有数据，确定要格式化吗？\n")<< std::endl;
            std::cout << ("Press \"y\" for 'format', \"n\" for 'cancel':")<< std::endl;
            char temp_cmd;
            while (temp_cmd = getchar()){
                if (temp_cmd == 'y'){
                    fileSystem.format();
                    break;
                }
                else if (temp_cmd == 'n'){
                    return ERROR_CANCEL;
                    break;
                }
                else{
                    std::cout << ("Press \"y\" for 'format', \"n\" for 'cancel':") << std::endl;
                }
            }
            break;
        default:
            break;
        }
    return OK;
}
InodeId Kernel::kernelTouch(const char *fileName){
    /**
     * @brief 在当前目录下创建文件(不管是文件还是文件夹)，文件名为fileName,
     * @return 返回新创建文件的inodeId
     */

    InodeId new_file_node = -1;

    /* 记录到该文件目录 */
    myPath new_file_dir(fileName);

    /* 尚未存在路径 */
    InodeId file_folder_inode = fileSystem.locateParDir(new_file_dir);
    if(file_folder_inode < 0){
        return ERROR_NO_FOLDER_EXSIT;
    }
    /* 存在路径，但是不是目录 */
    if((inodeCache.getInodeByID(file_folder_inode)->i_mode & Inode::IFMT) != Inode::IFDIR){
        return ERROR_NO_FOLDER_EXSIT;
    }

    /* 不重名 */
    InodeId file_inode = fileSystem.locateInode(new_file_dir);

    if (file_inode > 0){
        return ERROR_FILENAME_EXSIST;
    }
    new_file_node = createFile(file_inode, file_folder_inode, new_file_dir.getInodeName());

    return new_file_node;
}

InodeId Kernel::createFile(const InodeId &cur_Inode, const InodeId &par_inode, const std::string &node_name){
    /**
     * @brief 在当前目录下创建文件(不管是文件还是文件夹)，文件名为fileName,
     * @return 返回新创建文件的inodeId
     */
    InodeId new_file_node = -1;
    /* 为新文件分配新inode */
    new_file_node = superBlock.ialloc(); //得到inode号
    if (new_file_node <= 0){
        return -1;
    }
    Inode *p_inode = inodeCache.getInodeByID(new_file_node); //并将这个inode写入inodeCache
    p_inode->newInode(Inode::IUPD | Inode::IACC, new_file_node);

    /* 在当前目录文件中写入新的目录项 */
    Inode *p_dir_inode = inodeCache.getInodeByID(par_inode); // 不是当前文件中
    int blk_num = p_dir_inode->Bmap(0); //Bmap查物理块号
    Buf *p_buf = bufferCache.Bread(blk_num);

    DirectoryEntry *p_directoryEntry = (DirectoryEntry *)p_buf->b_addr;

    int i;
    for (i = 0; i < DISK_BLOCK_SIZE / sizeof(DirectoryEntry); i++){
        if ((p_directoryEntry->m_ino == 0)){
            //ino==0表示该文件被删除
            p_directoryEntry->m_ino = new_file_node;
            strcpy(p_directoryEntry->m_name, node_name.c_str());
            break;
        } 
        p_directoryEntry++;
    }
    if (i == DISK_BLOCK_SIZE / sizeof(DirectoryEntry)){
        return ERROR_NOTSPEC;
    }
    Kernel::instance().getBufferManager().Bdwrite(p_buf);

    return new_file_node;
}


int Kernel::mkdir(const char *dirName){

    /* 记录到该文件目录 */
    myPath new_file_dir(dirName);

    // 相当于先建立文件...
    int newDirInodeId = kernelTouch(dirName);

    if (newDirInodeId < 0){
        return newDirInodeId;
    }

    
    /* 然后把它改为文件夹 */
    Inode *p_inode = inodeCache.getInodeByID(newDirInodeId);
    p_inode->i_mode = Inode::IFDIR;

    
    BlkId blk_num = p_inode->Bmap(0);

    


    #ifdef IS_DEBUG
        std::cout << "[mkdir]blk_num: " << blk_num << "\n";
    #endif

    Buf *p_buf = bufferCache.Bread(blk_num);
    DirectoryEntry *p_directoryEntry = (DirectoryEntry *)p_buf->b_addr;
    /* 同时添加. 和 .. */
    DirectoryEntry tempDirectoryEntry;
    strcpy(tempDirectoryEntry.m_name, ".");
    tempDirectoryEntry.m_ino = newDirInodeId;
    *p_directoryEntry = tempDirectoryEntry;
    p_directoryEntry++;

    InodeId file_folder_inode = fileSystem.locateParDir(new_file_dir);
    strcpy(tempDirectoryEntry.m_name, "..");
    tempDirectoryEntry.m_ino = file_folder_inode; 
    *p_directoryEntry = tempDirectoryEntry;
   
    #ifdef IS_DEBUG
        std::cout << "[mkdir] newDirInodeId: " << newDirInodeId << "\n";
        std::cout << "[mkdir] file_folder_inode: " << file_folder_inode << "\n";
    #endif


    Kernel::instance().getBufferManager().Bdwrite(p_buf);

    return OK;
}



InodeId Kernel::deleteFolder(const char *dirName){
    /**
     * @brief 递归删除文件夹
     *        首先，删除其父目录的项目（该步骤包含在deleteFile中）
     *        然后，删除该文件，如果是文件夹则需要递归进入
     * @return 
     */

    myPath path(dirName);
    InodeId file_InodeID = fileSystem.locateInode(path);
    if (file_InodeID < 0){
        return file_InodeID;
    }
    InodeId dir_InodeID = fileSystem.locateParDir(path);

    /* 定位当前删除节点以及其父目录节点 */
    Inode &p_delete_inode = *inodeCache.getInodeByID(file_InodeID);
    Inode &p_dir_inode = *inodeCache.getInodeByID(dir_InodeID);

    if (p_delete_inode.i_mode & Inode::IFMT != Inode::IFDIR){
        printf("[ERROR]非法的参数\n");
        return -1;
    }

    p_delete_inode.i_flag |= Inode::IACC;

    /* 如果要删除的是目录 */
    if ((p_delete_inode.i_mode & Inode::IFMT) == Inode::IFDIR){
        /** 读出其数据帧，查看其所有的目录项*/
        int blk_num = p_delete_inode.Bmap(0); 
        Buf *p_buf = Kernel::instance().getBufferManager().Bread(blk_num);
        DirectoryEntry *p_directoryEntry = (DirectoryEntry *)p_buf->b_addr;
        
        for (int i = 0; i < DISK_BLOCK_SIZE / sizeof(DirectoryEntry); i++){
            /* 遍历目录项，进行逐个删除*/
            const InodeId entry_inode_Id = p_directoryEntry->m_ino;
            if ((entry_inode_Id != 0)){
                if (!strcmp(p_directoryEntry->m_name, ".") || !strcmp(p_directoryEntry->m_name, "..")){
                    p_directoryEntry++; // 别忘记啦
                    continue;
                }
                else{
                    /* 该目录有意义 */
                    if ((inodeCache.getInodeByID(entry_inode_Id)->i_mode & Inode::IFMT) == Inode::IFDIR){
                        std::string absolut_path = path.toString() + "/" + std::string(p_directoryEntry->m_name);
                        deleteFolder(absolut_path.c_str());
                    }
                    else{
                        deleteFile(entry_inode_Id, file_InodeID);
                    }
                }
            } 

            p_directoryEntry++;
        }
        /* 删除该目录本身 */
        Kernel::instance().getBufferManager().Bdwrite(p_buf);
        deleteObject(dirName);
    }
    else{
        std::cout <<("[ERROR]不能删除文件");
        return ERROR_DELETE_FAIL;
    }
    return file_InodeID;
}


InodeId Kernel::deleteFile(const char *fileName){
    /**
     * @brief 直接通过名字，删除文件
     */
    myPath path(fileName);
    InodeId deleteFileInode = fileSystem.locateInode(path);
    if (deleteFileInode < 0){
        return deleteFileInode;
    }
    Inode *p_delete_inode = inodeCache.getInodeByID(deleteFileInode);
    Inode *p_dir_inode = inodeCache.getInodeByID(fileSystem.locateParDir(path));

    if ((p_delete_inode->i_mode & Inode::IFMT) != Inode::IFDIR){ //普通文件
        return deleteObject(deleteFileInode, fileSystem.locateParDir(path));//fileName);
    }
    else{
        std::cout <<("[ERROR]不能删除文件夹");
        return ERROR_DELETE_FAIL;
    }
}


InodeId Kernel::deleteFile(const InodeId &cur_Inode, const InodeId &par_Inode){
    //目录文件和普通文件要分别处理！

    if (cur_Inode < 0)
    {
        return cur_Inode;
    }
    Inode *p_delete_inode = inodeCache.getInodeByID(cur_Inode);

    
    if ((p_delete_inode->i_mode & Inode::IFMT) != Inode::IFDIR){ //普通文件
        return deleteObject(cur_Inode, par_Inode);
    }
    else
    {
        std::cout <<("[ERROR]不能删除文件夹");
        return ERROR_DELETE_FAIL;
    }
}



/**
 * 直接删除
 */
InodeId Kernel::deleteObject(const char *fileName){
    myPath path(fileName);
    InodeId deleteFileInode = fileSystem.locateInode(path);
    if (deleteFileInode < 0)
    {
        return ERROR_DELETE_FAIL;
    }

    Inode *p_delete_inode = inodeCache.getInodeByID(deleteFileInode);
    Inode *p_dir_inode = inodeCache.getInodeByID(fileSystem.locateParDir(path));

    BlkId phyno;
    //Step1 释放盘块
    relseBlock(p_delete_inode);
    //Step2 删除目录项
    int dirblkno = p_dir_inode->Bmap(0); //Bmap查物理块号
    Buf *p_buf;
    p_buf = Kernel::instance().getBufferManager().Bread(dirblkno);
    DirectoryEntry *p_directoryEntry = (DirectoryEntry *)p_buf->b_addr;

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
    Kernel::instance().getBufferManager().Bdwrite(p_buf);
    //Step3 释放inode
    p_delete_inode->i_flag = 0; //这里是为了不再把删除的inode刷回，只用在superblock中标记inode删除即可
    superBlock.ifree(deleteFileInode);


    return deleteFileInode;
}

void Kernel::relseBlock(Inode *delete_inode){
    for (int i = 9; i >= 0; --i) {
		if (delete_inode->i_addr[i]) {
			if (i >= 6) {
                #ifdef IS_DEBUG
                    std::cout << i << std::endl;
                #endif
				Buf* pFirstBuffer = getBufferManager().Bread(delete_inode->i_addr[i]);
				int *pFirst = (int*)pFirstBuffer->b_addr;
				for (int j = DISK_BLOCK_SIZE / sizeof(int) - 1; j >= 0; --j) {
					if (pFirst[j]) {
						if (i >= 8) {
                            #ifdef IS_DEBUG
                                std::cout << i << std::endl;
							#endif
                            Buf* pSecondBuffer = getBufferManager().Bread(pFirst[j]);
                            

							int* pSecond = (int*)pSecondBuffer->b_addr;
							for (int k = DISK_BLOCK_SIZE / sizeof(int) - 1; k >= 0; --k) {
								if (pSecond[k]) {
                                    superBlock.bfree(pSecond[k]);
								}
							}
                            #ifdef IS_DEBUG
                                std::cout << "笨蛋您又来了" << std::endl;
                            #endif
							getBufferManager().Brelse(pSecondBuffer);
						}
                        #ifdef IS_DEBUG
                            std::cout << "free_block: " << superBlock.free_block_bum  \
                                    << "free_inode: " << superBlock.free_inode_num <<"\n";
                        #endif

                        superBlock.bfree(pFirst[j]);
					}
				}
                
				getBufferManager().Brelse(pFirstBuffer);
			}
            superBlock.bfree(delete_inode->i_addr[i]);
			delete_inode->i_addr[i] = 0;
		}
	}

}

InodeId Kernel::deleteObject(const InodeId &cur_Inode, const InodeId &par_Inode){

    if (cur_Inode < 0)
    {
        return ERROR_DELETE_FAIL;
    }

    Inode *p_delete_inode = inodeCache.getInodeByID(cur_Inode);
    Inode *p_dir_inode = inodeCache.getInodeByID(par_Inode);

    BlkId phyno;
    //Step1 释放盘块
    relseBlock(p_delete_inode);

    //Step2 删除目录项
    int dirblkno = p_dir_inode->Bmap(0); //Bmap查物理块号
    Buf *p_buf;
    p_buf = Kernel::instance().getBufferManager().Bread(dirblkno);
    DirectoryEntry *p_directoryEntry = (DirectoryEntry *)p_buf->b_addr;

    int de_i;
    int delete_inode_id = cur_Inode; // 防止指针引用导致的修改
    for (de_i = 0; de_i < DISK_BLOCK_SIZE / sizeof(DirectoryEntry); de_i++){
        if ((p_directoryEntry->m_ino == p_delete_inode->i_number)){ 
            p_directoryEntry->m_ino = 0;
            break;
        } //ino==0表示该文件被删除
        p_directoryEntry++;
    }

    if (de_i == DISK_BLOCK_SIZE / sizeof(DirectoryEntry))
    {
        return ERROR_DELETE_FAIL;
    }
    Kernel::instance().getBufferManager().Bdwrite(p_buf);
    //Step3 释放inode
    p_delete_inode->i_flag = 0; //这里是为了不再把删除的inode刷回，只用在superblock中标记inode删除即可
    superBlock.ifree(delete_inode_id);




    return delete_inode_id;
}



int Kernel::cd(const char *dirName)
{
    //Path path(dirName);
    myPath inst_path(dirName);
    

    //printf("dirName %s Path %s\n", dirName, path.toString().c_str());
    InodeId targetInodeId = fileSystem.locateInode(inst_path);
    if (targetInodeId <= 0)
    {
        std::cout <<("[ERROR]目录查找失败") << std::endl;
    }
    else if ((inodeCache.getInodeByID(targetInodeId)->i_mode & Inode::IFMT) != Inode::IFDIR)
    {
        std::cout <<("[ERROR]cd 命令的参数必须是目录") << std::endl;
    }
    else{

        Kernel::instance().getUser().curDirInodeId = targetInodeId;

        // 实时修改当前路径
        myPath my_cur_path(Kernel::instance().getUser().cur_path);

        if(inst_path.from_root){
            // 全部路径修改
            Kernel::instance().getUser().cur_path = "/" + inst_path.toString();
        }
        else{
            for(int i = 0 ; i < inst_path.path.size() &&  my_cur_path.path.size() >= 0 ; i ++ ){
                //printf("%d: |%s| |%s| \n",i, path.path[i], cur_path.path[cur_len - 1]);
                if(inst_path.path[i] == "."){
                    continue;
                }
                else if(inst_path.path[i] == ".."){
                    my_cur_path.my_pop();
                }
                else{
                    my_cur_path.path.push_back(inst_path.path[i]);

                }
            }

            // 部分路径添加
            Kernel::instance().getUser().cur_path = "/" + my_cur_path.toString();

        }

        //printf("cur: %s\n ", Kernel::instance().getUser().cur_path.c_str());
        

    }

    return targetInodeId;
}

void Kernel::ls(InodeId dirInodeID)
{
    //首先要获得这个inode->访问这个目录文件
    //step1: 检查inodeCache中有没有，有则直接用，没有则向Ext2模块要

    Inode &inode = *inodeCache.getInodeByID(dirInodeID);
    if (inode.i_mode & Inode::IFMT != Inode::IFDIR){
        std::cout << ("[ERROR]非法的参数") << std::endl;;
        return;
    }

    inode.i_flag |= Inode::IACC;

    #ifdef IS_DEBUG
        std::cout << "[ls] inodeID   " << inode.i_number << std::endl;
    #endif

    //Step2：读这个目录文件到缓存块中（可能已经存在于缓存块中,规定目录文件不能超过4096B）
    int blk_num = inode.Bmap(0); //Bmap查物理块号
    
    #ifdef IS_DEBUG
        std::cout << "[ls] blk_num: "<< blk_num << std::endl;
    #endif

    Buf *p_buf;
    p_buf = Kernel::instance().getBufferManager().Bread(blk_num);
    DirectoryEntry *p_directoryEntry = (DirectoryEntry *)p_buf->b_addr;
    //Step3：访问这个目录文件中的entry，打印出来（同时缓存到dentryCache中）

    for (int i = 0; i < DISK_BLOCK_SIZE / sizeof(DirectoryEntry); i++){
        if ((p_directoryEntry->m_ino != 0)){
            #ifdef IS_DEBUG
                std::cout << "[ls] entry: " << i << std::endl;
            #endif
            Inode &per_inode = *inodeCache.getInodeByID(p_directoryEntry->m_ino);
            std::string p_mode = "";
            if((per_inode.i_mode & Inode::IFMT) == Inode::IFDIR){
                p_mode = "D";
            }
            else{
                p_mode = "F";
            }
            std::cout << p_mode << " " << std::setw(10) << p_directoryEntry->m_name << "   " << std::setw(3) << p_directoryEntry->m_ino <<  " " <<per_inode.i_size << std::endl;
        } //ino==0表示该文件被删除

        p_directoryEntry++;
    }

    Kernel::instance().getBufferManager().Brelse(p_buf);

}

void Kernel::ls(const char *dirName){

    InodeId par_inode_id;

    myPath path(dirName);

    par_inode_id = fileSystem.locateInode(path);
    #ifdef IS_DEBUG
        std::cout << par_inode_id << std::endl;
    #endif
    if ((inodeCache.getInodeByID(par_inode_id)->i_mode & Inode::IFMT) == Inode::IFDIR){
        ls(par_inode_id);
    }
    else
    {
        std::cout <<("[ERROR]操作对象不可为文件") << std::endl;
    }
}


FileFd Kernel::open(const myPath& path, int mode){
    FileFd fd;
   /*先找inode*/
    InodeId openFileInodeId = fileSystem.locateInode(path);

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
int Kernel::read(int fd, uint8_t *content, int length){
    //分析：length可能大于、小于、等于盘块的整数倍
    int readByteCount = 0;

    User &u = Kernel::instance().getUser();
    File *p_file = u.u_ofiles.GetF(fd);
    Inode *p_inode = inodeCache.getInodeByID(p_file->f_inode_id);
    p_inode->i_flag |= Inode::IUPD;
    Buf *p_buf;

    if (length > p_inode->i_size - p_file->f_offset + 1)
    {
        length = p_inode->i_size - p_file->f_offset + 1;
    }

    while (readByteCount < length && p_file->f_offset <= p_inode->i_size) //NOTE 这里是<还是<=再考虑一下
    {
        BlkId logicBlkno = p_file->f_offset / DISK_BLOCK_SIZE; //逻辑盘块号

        #ifdef IS_DEBUG
            std::cout << "==============================================" << std::endl;
            std::cout << "[read] logicBlkno: " << logicBlkno << "\n";
        #endif

        BlkId phy_blk_id = p_inode->Bmap(logicBlkno);            //物理盘块号


        int offsetInBlock = p_file->f_offset % DISK_BLOCK_SIZE; //块内偏移

        p_buf = Kernel::instance().getBufferManager().Bread(phy_blk_id);

        #ifdef IS_DEBUG
            std::cout << "[read] phy_blk_id: " << phy_blk_id << "\n";
        #endif

        uint8_t *p_buf_byte = (uint8_t *)p_buf->b_addr;
        p_buf_byte += offsetInBlock;

        int cur_left =  DISK_BLOCK_SIZE - offsetInBlock;// + 1;

        if (length - readByteCount <= cur_left)
        { //要读大小<=当前盘块剩下的,读需要的大小

            memcpy(content, p_buf_byte, length - readByteCount);
            p_file->f_offset += length - readByteCount;
            readByteCount = length;
            content += length - readByteCount;
            //修改offset
        }
        else
        { //把剩下的全部读出来
            memcpy(content, p_buf_byte, cur_left);
            p_file->f_offset += cur_left;
            readByteCount += cur_left;
            content += cur_left;
            //修改offset
        }
        Kernel::instance().getBufferManager().Brelse(p_buf);
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

    Buf *p_buf;
    while (writeByteCount < length) //NOTE 这里是<还是<=再考虑一下
    {
        

        BlkId logicBlkno = p_file->f_offset / DISK_BLOCK_SIZE; //逻辑盘块号
        // if (logicBlkno == 1030)
        // {
        //     printf("暂时停下");
        // }
        // 逻辑盘块 -> 转成相应的物理盘块
        BlkId phy_blk_id = p_inode->Bmap(logicBlkno);            //物理盘块号

        
        int offsetInBlock = p_file->f_offset % DISK_BLOCK_SIZE; //块内偏移
        //NOTE:可能要先读后写！！！
        //当写不满一个盘块的时候，就要先读后写

        if (offsetInBlock == 0 && length - writeByteCount >= DISK_BLOCK_SIZE)
        {
            //这种情况不需要先读后写

            p_buf = Kernel::instance().getBufferManager().GetBlk(phy_blk_id);

        }
        else
        {
            //先读后写
            p_buf = Kernel::instance().getBufferManager().Bread(phy_blk_id);
        }



        uint8_t *p_buf_byte = (uint8_t *)p_buf->b_addr;
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

        Kernel::instance().getBufferManager().Brelse(p_buf);

        Kernel::instance().getBufferManager().Bdwrite(p_buf);
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
