#include "Shell.h"

#include "Kernel.h"

void Shell::help()
{
    system("cat help");
}

int Shell::readUserInput()
{
    mount();
    Logcat::log("建议先输入help指令，查看使用说明");

    static int auto_test = 0;
    
    char test[50][100] = { " ",
                         "touch a",
                         "touch b", 
                         "ls",
                         "fopen a -wr", 
                         "fopen b -wr",
                         "fwrite 0 12345678901234567890123456789012345678901234567890",
                         "fclose 0", 
                         "fopen a -wr",

                        /** 测试读取到终端  **/
                        //  "touch a",
                        //  "fopen a -wr",
                        //  "fwrite 0 12345678901234567890123456789012345678901234567890",
                        //  "fclose 0",
                        //  "fopen a -wr",
                        //  "fread 0 11", // 测试读出效果
                        //  "fread 0 3",
                        //  "fread 0 -f",

                        /** 测试读取到短文件  **/
                        //  "touch a",
                        //  //"fopen a -wr",
                        //  "fwrite 0 12345678901234567890123456789012345678901234567890",
                        //  "fclose 0",
                        //  "fopen a -wr",
                        //  "fread 0 -o a1.txt 11",    // read fd -o path size/-f 输出到外面
                        //  "fread 0 -o a2.txt 3",
                        //  "fread 0 -o a3.txt -f",

                        // 读取到长文件
                        //  "fwrite 1 -d README.md -f",
                        //  "fclose 1",
                        //  "fopen b -wr",
                        //  "fread 1 8000",
                        //  "fread 1 -o b2.txt 8000",
                         
                        //  "fread 1 8000",
                        //  "fread 1 -o b3.txt -f",

                         // "fread 1 -o b1.txt -f"


                        /** 测试seek  **/
                        //  "touch a",
                        //  //"fopen a -wr",
                        //  "fwrite 0 12345678901234567890123456789012345678901234567890",
                        //  "fclose 0",
                        //  "fopen a -wr",
                          "fread 0 4",    // read fd -o path size/-f 输出到外面
                          "fseek 0 -4",
                          "fwrite 0 12312321",
                          "fseek 0 -123",
                          "fread 0 -f",
                        //  "fread 0 -o a3.txt -f",


                         };

    // strcat(tty_buffer, test);
    
    while (true)
    {
        //Step0:
        //显示命令提示符

        putchar('$');
        putchar(' ');
        //TODO

        //Step1:获取用户输入放到缓冲区
        if(auto_test ++ < 0){
            strcpy(tty_buffer, test[auto_test]);
            //printf("%s\n", tty_buffer);
        }
        else{
            std::cin.getline(tty_buffer, MAX_CMD_LEN, '\n');
        }
        

        //Step2:先将tab转换为space
        for (char *checker = strrchr(tty_buffer, '\t'); checker != NULL; checker = strrchr(checker, '\t'))
        {
            *checker = ' ';
        }

        //Step3:以空格、tab为界，分解命令参数，存到Shell::split_cmd中
        char *dupl_tty_buffer = strdup(tty_buffer);
        /**
         * NOTE strdup创建的字符串是在堆上的，需要自己delete释放
         *@comment:这里拷贝一份tty_buffer的副本，因为后面用strtok函数的时候，会改变参数的字符串
         *当然也不是非要调用strtok，但是方便啊
         * 
         */

        //splitCmd先清空一下
        memset(split_cmd, 0x0, sizeof(split_cmd));
        int cmd_param_seq = 0;
        for (char *p = strtok(dupl_tty_buffer, " "); p != nullptr; p = strtok(NULL, " "), cmd_param_seq++)
        {
            strcpy(split_cmd[cmd_param_seq], p);
        }
        param_num = cmd_param_seq;
#ifdef IS_DEBUG
        for (int i = 0; i < param_num; i++)
        {
            std::cout << "看一下刚输入的参数：" << split_cmd[i] << ' ';
        }
        std::cout << std::endl;
#endif
        //TODO

        //Step4:解析执行指令
        parseCmd();
        delete dupl_tty_buffer;
        fflush(stdin);
    }
}

void Shell::parseCmd()
{
    switch (getInstType())
    {
    case MOUNT:
        mount(); //OK
        break;
    case UNMOUNT:
        unmount(); //OK
        break;
    case FORMAT:
        format(); //OK
        break;
    case CD:
        cd(); //OK
        break;
    case LS:
        ls(); //OK
        break;
    case RM:
        rm(); //OK
        break;
    case RMDIR:
        rmdir(); //OK
        break;
    case MKDIR:
        mkdir(); //OK
        break;
    case TOUCH:
        touch(); //OK
        break;
    case CLEAR:
        clear(); //OK
        break;
    case HELP:
        help(); //OK
        break;
    case EXIT:
        mexit(); //OK
        break;
    case VERSION:
        version(); //OK
        break;
    case STORE:
        store(); //OK
        break;
    case WITHDRAW:
        withdraw(); //OKKK
        break;
    case FOPEN: 
        open();
        break;
    case FCREAT: 
        creat();
        break;
    case FREAD:
        read();
        break;
    case FWRITE:
        write();
        break;
    case FCLOSE:
        close();
        break;
    case FSEEK:
        lseek();
        break;
    
    default:
        Logcat::log("CMD NOT SUPPORTED!\n");
        break;
    }
}

/**
 * @comment:实际上是做字符串到枚举类型的转化，为了switch case
 */
INSTRUCT Shell::getInstType()
{
    char *instStr = getInstStr();
#ifdef IS_DEBUG
    Logcat::log(TAG, "命令行命令字为:");
    Logcat::log(TAG, instStr);

#endif
    //为什么从1开始
    for (int i = 1; i < INST_NUM; i++)
    {
        //这里要加感叹号，注意strcmp在相等时返回的是0
        if (!strcmp(instructStr[i], instStr))
        {

#ifdef IS_DEBUG
            //std::cout<<INSTRUCT(i)<<std::endl;
#endif
            return INSTRUCT(i - 1);
        }
    }
    return ERROR_INST;
}

int Shell::FileMode(std::string mode) {
    int md = 0;
    if (mode.find("-r") != std::string::npos) {
        md |= File::FREAD;
    }
    if (mode.find("-w") != std::string::npos) {
        md |= File::FWRITE;
    }
    if (mode.find("-rw") != std::string::npos) {
        md |= (File::FREAD | File::FWRITE);
    }
    return md;
}

/**
 * @comment:命令缓冲区→命令参数字符数组→第一个参数得到命令字符串
 * 此函数的功能就是读出第一个字符串，亦即InstStr
 */
char *Shell::getInstStr()
{
    return split_cmd[0];
    //很简单，数组首个就是命令关键字
}

/**
 * @comment:这个是getInstStr更通用的情况
 */
char *Shell::getParam(int i)
{
    return split_cmd[i];
}

/**
 * 获得参数的个数
 */
int Shell::getParamAmount()
{
    for (int i = 0; i < MAX_PARAM_NUM; i++)
    {
        if (!strcmp(split_cmd[i], ""))
        {
            return i;
        }
    }
    return MAX_PARAM_NUM;
}

void Shell::mount()
{
    Logcat::devlog(TAG, "MOUNT EXEC");
    /**
     * 装载磁盘的最上层命令调用函数：
     * 硬盘装载的步骤：
     * ①内存inodeCache初始化
     * ②DiskDriver打开虚拟磁盘img，mmap，进入就绪状态
     * ③装载SuperBlock到VFS的SuperBlock缓存
     * 
     *  */
    my_kernel.mount();
}

void Shell::unmount()
{
    my_kernel.unmount();
    Logcat::devlog(TAG, "unmount EXEC");
}

/**
 * 对装载的磁盘镜像做格式化
 */
void Shell::format()
{

    if (1)
    {
        my_kernel.format();
        Logcat::devlog(TAG, "format EXEC");
    }
    else
    {
        Logcat::log(TAG, "ERROR,DISK NOT MOUNTED!");
    }
}
void Shell::mkdir()
{
    if (getParamAmount() == 2){

        switch (my_kernel.mkDir(getParam(1))){
            case ERROR_FILENAME_EXSIST:
                Logcat::log("[ERROR]创建失败,存在同名目录");
                break;
            case ERROR_NO_FOLDER_EXSIT:
                Logcat::log("[ERROR]创建失败,请先创建文件夹");
                break;
            default:
                Logcat::devlog(TAG, "创建成功");
                break;
        }
    }
    else
    {
        Logcat::log("ERROR！MKDIR参数个数错误！");
    }
    Logcat::devlog(TAG, "mkdir EXEC");
}
void Shell::cat()
{
    Logcat::devlog(TAG, "cat EXEC");
    Logcat::log("cat 暂不支持");
}
void Shell::touch()
{
    if (getParamAmount() != 2)
    {
        Logcat::log("ERROR!参数个数错误！");
        return;
    }
    else
    {
        switch (my_kernel.createFile(getParam(1))){
            case ERROR_FILENAME_EXSIST:
                Logcat::log("[ERROR]创建失败,存在同名文件");
                break;
            case ERROR_NO_FOLDER_EXSIT:
                Logcat::log("[ERROR]创建失败,请先创建文件夹");
                break;
            default:
                Logcat::devlog(TAG, "创建成功");
                break;
        }
    }
}

/**
 * 删除文件
 */
void Shell::rm()
{
    if (getParamAmount() != 2)
    {
        Logcat::log("ERROR!参数个数错误！");
        return;
    }
    else
    {
        if (0 > my_kernel.deleteFile(getParam(1)))
        {
            Logcat::log("删除文件失败！");
        }
    }

    Logcat::devlog(TAG, "rm EXEC");
}

/**
 * 删除目录以及其下的所有文件
 */
void Shell::rmdir()
{
    if (getParamAmount() != 2)
    {
        Logcat::log("ERROR!参数个数错误！");
        return;
    }
    else
    {
        if (0 > my_kernel.deleteFolder(getParam(1)))
        {
            Logcat::log("删除，目录失败！");
        }
    }

    Logcat::devlog(TAG, "rmdir EXEC");
}

void Shell::version()
{
    system("cat version");
    Logcat::devlog(TAG, "version EXEC");
}
void Shell::man()
{
    Logcat::log(TAG, "欢迎求助那个男人");

    Logcat::devlog(TAG, "man EXEC");
}
void Shell::mexit()
{
    if (1)
    {
        my_kernel.unmount();
    }
    Logcat::devlog(TAG, "exit EXEC");
    Logcat::log("程序结束！");
    exit(OK);
}

/**
 * 用户指令：更改当前目录
 */
void Shell::cd()
{

    //cd必须带参数
    if (getParamAmount() != 2)
    {
        Logcat::log("Error!cd命令参数个数错误！");
    }
    else
    {
        my_kernel.cd(getParam(1));
    }
}

/**
 * ls函数可以带参数，也可以不带（curDir）
 */
void Shell::ls()
{
    if (!strcmp(getParam(1), ""))
    {
        //不带参数的ls，以curDir为默认参数
        my_kernel.ls(Kernel::instance().getUser().curDirInodeId);
    }
    else
    {
        my_kernel.ls(getParam(1)); //getParam(1)获得的是ls后面跟的目录名（可能是相对的也可能是绝对的）
    }
}

/**
 * 将外部文件考入虚拟磁盘.带两个命令参数
 * Usage: store [src path] [des filename]
 */
void Shell::store()
{
    if (getParamAmount() == 3)
    {
        InodeId desInodeId;
        //STORE的步骤
        //Step1：创建文件（如果有同名的返回失败）
        desInodeId = my_kernel.createFile(getParam(2));
        if (desInodeId < 0)
        {
            Logcat::log("ERROR!目标文件名已存在！");
            return;
        }
        //Step2：打开文件
        Path desPath(getParam(2));
        FileFd fd_des = my_kernel.open(desPath, File::FWRITE);
        //Step3：写入文件
        FILE *fd_src = fopen(getParam(1), "rb");
        if (fd_src == NULL)
        {
            Logcat::log("源文件打开失败！");
            return;
        }
        DiskBlock tempBuf;
        int file_size = 0;
        while (!feof(fd_src))
        {
            //int blkCount = 0;
            int readsize = fread(&tempBuf, 1, DISK_BLOCK_SIZE, fd_src);
            file_size += readsize;
            my_kernel.write(fd_des, (uint8_t *)&tempBuf, readsize);
        }
        Inode *p_desInode = Kernel::instance().getInodeCache().getInodeByID(desInodeId);
        p_desInode->i_size = file_size; //TODO这一块不太好，封装性差了点

        //Step4：关闭文件
        fclose(fd_src);
        my_kernel.close(fd_des);
    }
    else
    {
        Logcat::log("ERROR!store命令参数个数错误");
    }
}

/**
 * 将文件从虚拟磁盘中拷出
 * Usage: withdraw [src filename] [des outer_path]
 */
void Shell::withdraw()
{
    if (getParamAmount() == 3)
    {
        InodeId desInodeId;
        //WITHDRAW的步骤
        //Step1：创建文件（如果有同名的返回失败）
        FILE *fd_des = fopen(getParam(2), "wb");
        if (fd_des == NULL)
        {
            Logcat::log("目的文件创建失败！");
            return;
        }

        //Step2：打开文件
        Path srcPath(getParam(1));
        FileFd fd_src = my_kernel.open(srcPath, File::FREAD);
        if (fd_src < 0)
        {
            Logcat::log("源文件打开失败！");
            return;
        }
        //Step3：写入文件
        DiskBlock tempBuf;
        while (!my_kernel.eof(fd_src))
        {
            //int blkCount = 0;
            int writesize = my_kernel.read(fd_src, (uint8_t *)&tempBuf, DISK_BLOCK_SIZE);
            //? 为什么最后一个是 \00
            if(writesize < DISK_BLOCK_SIZE)
                writesize -= 1;
            fwrite(&tempBuf, 1, writesize, fd_des);
        }
        //Step4：关闭文件
        fclose(fd_des);
        my_kernel.close(fd_src);
    }
    else
    {
        Logcat::log("ERROR!store命令参数个数错误");
    }
}

void Shell::clear()
{
    system("clear");
}

Shell::Shell(Kernel& kernel):my_kernel(kernel){
    TAG = strdup("Shell");

}


Shell::~Shell()
{
    delete TAG;
}
void Shell::setKernel(Kernel& kernel){
    my_kernel = kernel;
}



//隐式调用

void Shell::creat()
{
    /*
     * brief@ fcreat filename -w/r
     */
    if (getParamAmount() != 3)
    {
        Logcat::log("[ERROR]参数个数错误！");
        return;
    }
    else
    {
        int md = FileMode(getParam(2));
        if (md == 0) {
            Logcat::log(TAG, "this mode is undefined !");
            return;
        }

        if (0 > my_kernel.createFile(getParam(1)))
        {
            Logcat::log("[ERROR]存在同名文件，创建失败！");
            return;
        }
        //Step1：打开内部文件
        Path srcPath(getParam(1));
        FileFd fd_des = my_kernel.open(srcPath, md);

        if (fd_des < 0)
        {
            Logcat::log("[ERROR]文件打开失败！");
            return;
        }
        printf("[INFO]成功创建并打开文件，fd = %d\n", fd_des);
    }
}

/**
 * 临时的，不应该是一个用户接口
 */
void Shell::open()
{
    /*
     * brief@ fopen filename rights...
     */

    if (getParamAmount() == 3)
    {
        int md = FileMode(getParam(2));
        if (md == 0) {
            Logcat::log(TAG, "this mode is undefined !");
            return;
        }

        //Step1：打开内部文件
        Path srcPath(getParam(1));

        InodeId targetInodeId = my_kernel.getExt2().locateInode(srcPath);
        if (targetInodeId <= 0)
        {
            Logcat::log("[ERROR]无法打开不存在的文件");
            return;
        }
        else if ((my_kernel.getInodeCache().getInodeByID(targetInodeId)->i_mode & Inode::IFMT) == Inode::IFDIR)
        {
            Logcat::log("[ERROR]无法打开一个目录");
            return;
        }

        FileFd fd_des = my_kernel.open(srcPath, md);

        if (fd_des < 0)
        {
            Logcat::log("文件打开失败！");
            return;
        }
        printf("成功打开文件，fd = %d\n", fd_des);
    }
    else
    {
        Logcat::log("ERROR!store命令参数个数错误");
    }

}
/**
 * 临时的，不应该是一个用户接口
 */
void Shell::close()
{
    /*
     * brief@ fclose filename rights...
     */
    if (getParamAmount() == 2)
    {
        User& u = Kernel::instance().getUser();
        //Step1
        FileFd fd_src = std::stoi(getParam(1));
        /* 获取打开文件控制块File结构 */
        File* pFile = u.u_ofiles.GetF(fd_src);
        if (NULL == pFile) {
            Logcat::log("不存在这个fd");
            return;
        }
        printf("成功关闭fd = %d\n", fd_src);
        my_kernel.close(fd_src);
    }
    else
    {
        Logcat::log("ERROR!fclose命令参数个数错误");
    }
}

/**
 * 临时的，不应该是一个用户接口
 */
void Shell::read(){
    /*
     * brief@ read fd size/-f
     *        read fd -o path size/-f 输出到外面
     */
    if (getParamAmount() == 3 || getParamAmount() == 5)
    {
        InodeId desInodeId;
        User& u = Kernel::instance().getUser();

        if(!isdigit(std::string(getParam(1)).front())){
            Logcat::log("[ERROR]fd为非法输入");
            return;
        }
        FileFd fd_src = std::stoi(getParam(1));
        File* pFile = u.u_ofiles.GetF(fd_src);

        if (NULL == pFile) {
            Logcat::log("[ERROR]不存在这个fd");
            return;
        }

        Inode *p_inode = my_kernel.getInodeCache().getInodeByID(pFile->f_inode_id); //TODO错误处理?

        int& file_size = p_inode->i_size;
        int& file_offset = pFile->f_offset;
        int cur_read_num = 0;
        DiskBlock tempBuf;
        int read_limit = INT32_MAX;

        if(getParamAmount() == 5){
            

            if(strcmp(getParam(2), "-o") != 0){
                //
                Logcat::log("[ERROR]第二个参数应该为-o");
                return;
            }
            //Step1：创建文件（如果有同名的返回失败）
            FILE *fd_des = fopen(getParam(3), "wb");
            if (fd_des == NULL){
                Logcat::log("[ERROR]目的文件创建失败！");
                return;
            }

            if(strcmp(getParam(4), "-f") == 0){
                //Step3：写入文件
                DiskBlock tempBuf;
                while (!my_kernel.eof(fd_src))
                {
                    //int blkCount = 0;
                    int writesize = my_kernel.read(fd_src, (uint8_t *)&tempBuf, DISK_BLOCK_SIZE);
                    
                    //? 为什么最后一个是 \00
                    if(writesize < DISK_BLOCK_SIZE)
                        writesize -= 1;
                    cur_read_num += writesize;
                    fwrite(&tempBuf, 1, writesize, fd_des);
                }
            }
            else{
                if(!isdigit(std::string(getParam(4)).front())){
                    Logcat::log("[ERROR]size为非法输入");
                    return;
                }
                read_limit = std::stoi(getParam(4));

                while (!my_kernel.eof(fd_src)){

                    int readable_size = std::min(std::min(file_size - file_offset, read_limit - cur_read_num), DISK_BLOCK_SIZE);//
                    // 读不到了
                    //printf("readable_size %d %d\n",readable_size, file_offset );
                    if(readable_size <= 0){
                        break;
                    }

                    int real_read_num = my_kernel.read(fd_src, (uint8_t *)&tempBuf, readable_size);
                    tempBuf.content[real_read_num] = (uint8_t)'\0';

                    fwrite(&tempBuf, 1, readable_size, fd_des);
                    cur_read_num += readable_size;
                }
                
 
            }
            printf("[INFO] 成功读出%d bytes,内容存在 '%s' 中\n", cur_read_num, getParam(3));
            fclose(fd_des);

        }
        
        else if(getParamAmount() == 3){
            if(strcmp(getParam(2), "-f") != 0){
                if(!isdigit(std::string(getParam(2)).front())){
                    Logcat::log("[ERROR]size为非法输入");
                    return;
                }
                read_limit = std::stoi(getParam(2));
            }

            std::string res_read = "";
            while (!my_kernel.eof(fd_src)){
                
                int readable_size = std::min(std::min(file_size - file_offset, read_limit - cur_read_num), DISK_BLOCK_SIZE);//
                // 读不到了
                if(readable_size <= 0){
                    break;
                }

                int real_read_num = my_kernel.read(fd_src, (uint8_t *)&tempBuf, readable_size);

                tempBuf.content[real_read_num] = (uint8_t)'\0';

                res_read += std::string((char *)&tempBuf);
                cur_read_num += readable_size;
            }
            char s[1];
            printf("[INFO] 成功读出%d bytes,内容为: %s\n", cur_read_num, res_read.data()); // s);//
        }
        //WITHDRAW的步骤
        else{
            Logcat::log("ERROR!store命令参数个数错误");
        }
    }
    else
    {
        Logcat::log("ERROR!store命令参数个数错误");
    }

    Logcat::log(TAG, "read EXEC");
}

/**
 * 临时的，不应该是一个用户接口
 */
void Shell::write()
{
    /*
     * brief@ fwrite fd string....
     *        fwrite fd -d pathname size/-f  外部文件写入！
     * 
     */

    int cur_arg_num = getParamAmount();

    if (cur_arg_num == 3 || cur_arg_num == 5)
    {
        if(!isdigit(std::string(getParam(1)).front())){
            Logcat::log("[ERROR]fd为非法输入");
            return;
        }
        User& u = Kernel::instance().getUser();
        //Step1

        FileFd fd_des = std::stoi(getParam(1));
        /* 获取打开文件控制块File结构 */
        File* pFile = u.u_ofiles.GetF(fd_des);

        Inode *p_inode = my_kernel.getInodeCache().getInodeByID(pFile->f_inode_id); //TODO错误处理?

        int ori_file_size = p_inode->i_size;
        int file_offset = pFile->f_offset;

        if (NULL == pFile) {
            Logcat::log("[ERROR]不存在这个fd");
            return;
        }

        if(pFile->f_flag < File::FWRITE){
            Logcat::log("[ERROR]没有写的权利");
            return;
        }
        //
        InodeId desInodeId = pFile->f_inode_id;

        DiskBlock tempBuf;
        int file_size = 0;

        if(cur_arg_num == 3){
            // 从参数中写入
            std::string input =  getParam(2);
            int i;
            for(i = 0 ; i + DISK_BLOCK_SIZE < input.length(); i += DISK_BLOCK_SIZE){
                memcpy(&tempBuf, input.data() + i, DISK_BLOCK_SIZE);
                my_kernel.write(fd_des, (uint8_t *)&tempBuf, DISK_BLOCK_SIZE);
                file_size += DISK_BLOCK_SIZE;
            }
            int last_readsize = input.length() - i;
            memcpy(&tempBuf, input.data() + i, last_readsize);

            my_kernel.write(fd_des, (uint8_t *)&tempBuf, last_readsize);
            file_size += last_readsize;
        }
        else if(cur_arg_num == 5 && strcmp(getParam(2), "-d") == 0){

            FILE *fd_src = fopen(getParam(3), "rb");
            if (fd_src == NULL){
                Logcat::log("[ERROR]不存在的外部文件");
                return;
            }
            // 从文件中写入
            // 默认是全部写入
            int file_size_limit = INT32_MAX;

            //printf("hhhh %s/n", getParam(3));

            if(strcmp(getParam(4),"-f") != 0){
                //给定长度
                file_size_limit = std::stoi(getParam(4));
            }

            while (!feof(fd_src)){
                //每次可以读入的量
                int readsize = fread(&tempBuf, 1, DISK_BLOCK_SIZE, fd_src);
                tempBuf.content[readsize] = '\0';
                file_size += readsize;

                if(file_size >= file_size_limit){
                    int cur_readsize = file_size_limit - (file_size - readsize);
                    file_size = file_size_limit;
                    my_kernel.write(fd_des, (uint8_t *)&tempBuf, cur_readsize);
                    break;
                }
                else{
                    my_kernel.write(fd_des, (uint8_t *)&tempBuf, readsize);
                }
            }
        }
        
        else{
            Logcat::log("[ERROR]不符合输入规则");
            return;
        }
        Inode *p_desInode = Kernel::instance().getInodeCache().getInodeByID(desInodeId);
        p_desInode->i_size = std::max(file_offset + file_size, ori_file_size); //TODO这一块不太好，封装性差了点

        printf("[INFO]成功写入 fd = %d %d bytes\n", fd_des, file_size);
        //printf("%d %d\n", file_offset + file_size, ori_file_size);

    }
    else
    {
        Logcat::log("[ERROR]参数个数错误");
    }
}

/**
 * 临时的，不应该是一个用户接口
 */
void Shell::lseek()
{
    /*
     * brief@ fseek fd offset
     */

    if(getParamAmount() == 3){


        if(!isdigit(std::string(getParam(1)).front())){
            Logcat::log("[ERROR]fd为非法输入");
            return;
        }
        char front = std::string(getParam(2)).front();

        if(!isdigit(front) && front != '-' ){
            Logcat::log("[ERROR]offset为非法输入");
            return;
        }

        User& u = Kernel::instance().getUser();

        FileFd fd_src = std::stoi(getParam(1));
        int offset = std::stoi(getParam(2));

        File* pFile = u.u_ofiles.GetF(fd_src);

        if (NULL == pFile) {
            Logcat::log("[ERROR]不存在这个fd");
            return;
        }
        Inode *p_inode = my_kernel.getInodeCache().getInodeByID(pFile->f_inode_id); //TODO错误处理?


        int& file_size = p_inode->i_size;
        int file_offset = pFile->f_offset;

        if(0 <=file_offset + offset && file_offset + offset <= file_size){
            pFile->f_offset = file_offset + offset;
        }
        else if(file_offset + offset < 0){
            pFile->f_offset = 0;
        }
        else{
            pFile->f_offset = file_size;
        }
        int real_offset = pFile->f_offset - file_offset; // (+往后，-往前)

        printf("[INFO]成功seek %d bytes, 当前位置： %d \n", real_offset, pFile->f_offset);


    }
    
}