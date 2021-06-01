#include "Shell.h"
#include "Kernel.h"






int Shell::run(){
    /**
     * @brief 解析输入，调用功能。
     * @notice 这是一个死循环
     */
    mount();

    std::cout << "[INFO]程序成功启动，祝您使用愉快" << std::endl;
    std::cout << "[INFO]man 指令随时为您效劳 :)" << std::endl;
    
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

    static int format_inst_num = 0;
    char format_inst[50][100] = {
        " ",
        "mkdir home",
        "mkdir dev",
        "mkdir etc",
        "mkdir bin",
        "cd home",
        "store ../assets/img.png photos",
        "store ../assets/readme.txt reports",
        "store ../assets/report.txt texts" ,
        "cd /"
    };

    int need_format = 0;

    while (true){
        //显示命令提示符与当前路径

        printf("[host-1851447] %s $", Kernel::instance().getUser().cur_path.c_str());
        putchar(' ');

        //是否执行初始化语句
        if(need_format == 1){
            if(format_inst_num ++ < 9){
                strcpy(tty_buffer, format_inst[format_inst_num]);
                std::cout << tty_buffer << std::endl;
            }
            else{
                need_format = 0;
                format_inst_num = 0;
            }
        }
        else if(auto_test ++ < 0){
            // 是否执行自动测试语句
            strcpy(tty_buffer, test[auto_test]);
            std::cout << tty_buffer << std::endl;
        }
        else{
            // 从命令行读语句
            std::cin.getline(tty_buffer, MAX_CMD_LEN, '\n');
        }
        //解析指令
        parseCmd();
        //执行指令
        executeCmd();
        //如果是初始化...
        if(getInstType() == FORMAT){
            need_format = 1;
        }
        
        fflush(stdin);
    }
}

void Shell::parseCmd(){
    //先将tab转换为space
    for (char *checker = strrchr(tty_buffer, '\t'); checker != NULL; checker = strrchr(checker, '\t')){
        *checker = ' ';
    }

    //Step3:以空格、tab为界，分解命令参数，存到Shell::split_cmd中
    char *dupl_tty_buffer = strdup(tty_buffer);
    //splitCmd先清空一下
    memset(split_cmd, 0x0, sizeof(split_cmd));
    int cmd_param_seq = 0;
    for (char *p = strtok(dupl_tty_buffer, " "); p != nullptr; p = strtok(NULL, " "), cmd_param_seq++){
        strcpy(split_cmd[cmd_param_seq], p);
    }
    param_num = cmd_param_seq;
    delete dupl_tty_buffer;
}

void Shell::executeCmd(){
    #ifdef IS_DEBUG
        std::cout << "getInstType(): " << getInstType() << "\n";
    #endif
    switch (getInstType()){
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
        case EXIT:
            mexit(); //OK
            break;
        case STORE:
            store(); //OK
            break;
        case LOAD:
            load(); //OKKK
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
        case MAN:
            man();
            break;
        case NOHUP:
            break;
        default:
            std::cout <<("[ERROR]你输入了非法命令") << std::endl;
            break;
    }
}


INSTRUCT Shell::getInstType(){
    /**
     * @brief 实际上是做字符串到枚举类型的转化，为了switch case
     */
    char *instStr = getInstStr();

#ifdef IS_DEBUG
    std::cout <<"命令行命令字为:" << instStr << std::endl;
#endif
    for (int i = 1; i < INST_NUM; i++){
        if (!strcmp(instructStr[i], instStr)){

#ifdef IS_DEBUG
            //std::cout<<INSTRUCT(i)<<std::endl;
#endif
            return INSTRUCT(i - 1);
        }
    }
    return ERROR_INST;
}

int Shell::getFileMode(std::string mode) {
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


char *Shell::getInstStr(){
    /**
     * 
     */
    return split_cmd[0];
    //很简单，数组首个就是命令关键字
}


char *Shell::getParam(int i){
    return split_cmd[i];
}


int Shell::getParamAmount(){
    /**
     * 获得参数的个数
     */
    for (int i = 0; i < MAX_PARAM_NUM; i++){
        if (!strcmp(split_cmd[i], "")){
            return i;
        }
    }
    return MAX_PARAM_NUM;
}

void Shell::mount(){
    my_kernel.initKernel();
}

void Shell::unmount(){
    my_kernel.relsKernel();
}

/**
 * 对装载的磁盘镜像做格式化
 */
void Shell::format(){
    my_kernel.format();
}

void Shell::mkdir(){
    if (getParamAmount() == 2){
        switch (my_kernel.mkdir(getParam(1))){
            case ERROR_FILENAME_EXSIST:
                std::cout <<("[ERROR]创建失败,存在同名目录") << std::endl;
                break;
            case ERROR_NO_FOLDER_EXSIT:
                std::cout <<("[ERROR]创建失败,请先创建文件夹") << std::endl;
                break;
            default:
                break;
        }
    }
    else{
        std::cout <<("[ERROR]非法的参数个数") << std::endl;
    }
}

void Shell::touch(){
    if (getParamAmount() != 2){
        std::cout <<("ERROR!参数个数错误！") << std::endl;
        return;
    }
    else
    {
        switch (my_kernel.kernelTouch(getParam(1))){
            case ERROR_FILENAME_EXSIST:
                std::cout <<("[ERROR]创建失败,存在同名文件") << std::endl;
                break;
            case ERROR_NO_FOLDER_EXSIT:
                std::cout <<("[ERROR]创建失败,请先创建文件夹") << std::endl;
                break;
            default:
                break;
        }
    }
}


void Shell::rm(){
    if (getParamAmount() != 2){
        std::cout <<("ERROR!参数个数错误！") << std::endl;
        return;
    }
    else{
        if (0 > my_kernel.deleteFile(getParam(1))){
            std::cout <<("删除文件失败！") << std::endl;
        }
    }
}


void Shell::rmdir(){
    if (getParamAmount() != 2){
        std::cout <<("ERROR!参数个数错误！") << std::endl;
        return;
    }
    else{
        if (0 > my_kernel.deleteFolder(getParam(1))){
            std::cout <<("删除，目录失败！") << std::endl;
        }
    }

}



void Shell::man(){
    if(getParamAmount() > 2){
        std::cout << "[ERROR]你给了那个男人太多参数了" << std::endl;
    }
    else{
        if(getParamAmount() == 1){
            my_kernel.my_man(getParam(0));
        }
        else{
            my_kernel.my_man(getParam(1));
        }
    }
}
void Shell::mexit(){
    my_kernel.relsKernel();
    std::cout <<("[INFO]程序结束,期望下次与您相会") << std::endl;
    exit(OK);
}

/**
 * 用户指令：更改当前目录
 */
void Shell::cd(){

    //cd必须带参数
    if (getParamAmount() != 2){
        std::cout <<("Error!cd命令参数个数错误！")  << std::endl;
    }
    else{
        my_kernel.cd(getParam(1));
    }
}

/**
 * ls函数可以带参数，也可以不带（curDir）
 */
void Shell::ls()
{
    if(getParamAmount() == 1){
        //不带参数的ls，以curDir为默认参数
        my_kernel.ls(Kernel::instance().getUser().curDirInodeId);
    }
    else if(getParamAmount() == 2){
        my_kernel.ls(getParam(1)); //getParam(1)获得的是ls后面跟的目录名（可能是相对的也可能是绝对的）

    }
    else{
        std::cout <<("[ERROR]非法的参数个数") << std::endl;
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
        desInodeId = my_kernel.kernelTouch(getParam(2));
        if (desInodeId < 0)
        {
            std::cout <<("ERROR!目标文件名已存在！") << std::endl;
            return;
        }
        //Step2：打开文件
        myPath desPath(getParam(2));

        FileFd fd_des = my_kernel.open(desPath, File::FWRITE);

        //Step3：写入文件
        FILE *fd_src = fopen(getParam(1), "rb");
        if (fd_src == NULL)
        {
            std::cout <<("[ERROR]源文件打开失败！") << std::endl;
            // 删除该文件...
            Kernel::instance().deleteObject(getParam(2));

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
        std::cout <<("[ERROR]非法的参数个数") << std::endl;
    }
}

/**
 * 将文件从虚拟磁盘中拷出
 * Usage: load [src filename] [des outer_path]
 */
void Shell::load(){
    if (getParamAmount() == 3)
    {
        InodeId desInodeId;
        //WITHDRAW的步骤
        //Step1：创建文件（如果有同名的返回失败）
        FILE *fd_des = fopen(getParam(2), "wb");
        if (fd_des == NULL)
        {
            std::cout <<("[ERROR]目的文件创建失败！") << std::endl;
            return;
        }

        //Step2：打开文件
        myPath srcPath(getParam(1));
        FileFd fd_src = my_kernel.open(srcPath, File::FREAD);
        if (fd_src < 0){
            std::cout <<("[ERROR]源文件打开失败！") << std::endl;
            unlink(getParam(2));
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
        std::cout <<("[ERROR]非法的参数个数") << std::endl;
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

//隐式调用
void Shell::creat()
{
    /*
     * brief@ fcreat filename -w/r
     */
    if (getParamAmount() != 3)
    {
        std::cout <<("[ERROR]参数个数错误！") << std::endl;
        return;
    }
    else
    {
        int md = getFileMode(getParam(2));
        if (md == 0) {
            std::cout <<("[ERROR]没有定义的操作类型") << std::endl;
            return;
        }

        if (0 > my_kernel.kernelTouch(getParam(1)))
        {
            std::cout <<("[ERROR]存在同名文件，创建失败") << std::endl;
            return;
        }
        //Step1：打开内部文件
        myPath srcPath(getParam(1));
        FileFd fd_des = my_kernel.open(srcPath, md);

        if (fd_des < 0)
        {
            std::cout <<("[ERROR]文件打开失败！") << std::endl;
            return;
        }
        std::cout << "[INFO]成功创建并打开文件，fd = " << fd_des  << std::endl;
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
        int md = getFileMode(getParam(2));
        if (md == 0) {
            std::cout <<("[ERROR]没有定义的操作类型") << std::endl;
            return;
        }

        //Step1：打开内部文件
        myPath srcPath(getParam(1));
         
        InodeId targetInodeId = my_kernel.getFileSystem().locateInode(srcPath);
        if (targetInodeId <= 0)
        {
            std::cout <<("[ERROR]无法打开不存在的文件") << std::endl;
            return;
        }
        else if ((my_kernel.getInodeCache().getInodeByID(targetInodeId)->i_mode & Inode::IFMT) == Inode::IFDIR)
        {
            std::cout <<("[ERROR]无法打开一个目录") << std::endl;
            return;
        }

        FileFd fd_des = my_kernel.open(srcPath, md);

        if (fd_des < 0)
        {
            std::cout <<("[ERROR]文件打开失败") << std::endl;
            return;
        }
        std::cout << "[INFO]成功打开文件，fd = " << fd_des << std::endl;
    }
    else
    {
        std::cout <<("[ERROR]非法的参数个数") << std::endl;
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
        if(!isdigit(std::string(getParam(1)).front())){
            std::cout <<("[ERROR]fd为非法输入") << std::endl;
            return;
        }
        FileFd fd_src = std::stoi(getParam(1));
        /* 获取打开文件控制块File结构 */
        File* pFile = u.u_ofiles.GetF(fd_src);
        if (NULL == pFile) {
            std::cout <<("[ERROR]不存在这个fd") << std::endl;
            return;
        }
        std::cout << "[INFO]成功关闭fd = " << fd_src << std::endl;
        my_kernel.close(fd_src);
    }
    else
    {
        std::cout <<("[ERROR]非法的参数个数") << std::endl;
    }
}

void Shell::read(){
    /**
     * @brief read fd size/-f
     *        read fd -o path size/-f 输出到外面
     */
    if (getParamAmount() == 3 || getParamAmount() == 5)
    {
        InodeId desInodeId;
        User& u = Kernel::instance().getUser();

        if(!isdigit(std::string(getParam(1)).front())){
            std::cout <<("[ERROR]fd为非法输入") << std::endl;
            return;
        }
        FileFd fd_src = std::stoi(getParam(1));
        File* pFile = u.u_ofiles.GetF(fd_src);

        if (NULL == pFile) {
            std::cout <<("[ERROR]不存在这个fd") << std::endl;
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
                std::cout <<("[ERROR]第二个参数应该为-o") << std::endl;
                return;
            }
            //Step1：创建文件（如果有同名的返回失败）
            FILE *fd_des = fopen(getParam(3), "wb");
            if (fd_des == NULL){
                std::cout <<("[ERROR]目的文件创建失败") << std::endl;
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
                    std::cout <<("[ERROR]size为非法输入") << std::endl;
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
                    std::cout <<("[ERROR]size为非法输入") << std::endl;
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
            std::cout << "[INFO] 成功读出" << cur_read_num << " bytes,内容为: \n" << res_read.data() << std::endl; // s);//
        }
        //WITHDRAW的步骤
        else{
            std::cout <<("[ERROR]非法的参数个数") << std::endl;
        }
    }
    else
    {
        std::cout <<("[ERROR]非法的参数个数") << std::endl;
    }

}


void Shell::write(){
    /**
     * @brief fwrite fd string....
     *        fwrite fd -d pathname size/-f  外部文件写入！
     * 
     */

    int cur_arg_num = getParamAmount();

    if (cur_arg_num == 3 || cur_arg_num == 5)
    {
        if(!isdigit(std::string(getParam(1)).front())){
            std::cout <<("[ERROR]fd为非法输入") << std::endl;
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
            std::cout <<("[ERROR]不存在这个fd") << std::endl;
            return;
        }

        if(pFile->f_flag < File::FWRITE){
            std::cout <<("[ERROR]没有写的权利") << std::endl;
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
                std::cout <<("[ERROR]不存在的外部文件") << std::endl;
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
            std::cout <<("[ERROR]不符合输入规则") << std::endl;
            return;
        }
        Inode *p_desInode = Kernel::instance().getInodeCache().getInodeByID(desInodeId);
        p_desInode->i_size = std::max(file_offset + file_size, ori_file_size); //TODO这一块不太好，封装性差了点

        std::cout << "[INFO]成功写入fd = " << fd_des << "  " << file_size << " bytes" << std::endl;
        //printf("%d %d\n", file_offset + file_size, ori_file_size);

    }
    else
    {
        std::cout <<("[ERROR]参数个数错误") << std::endl;
    }
}


void Shell::lseek(){
    /*
     * brief@ fseek fd offset
     */
    if(getParamAmount() == 3){


        if(!isdigit(std::string(getParam(1)).front())){
            std::cout <<("[ERROR]fd为非法输入") << std::endl;
            return;
        }
        char front = std::string(getParam(2)).front();

        if(!isdigit(front) && front != '-' ){
            std::cout <<("[ERROR]offset为非法输入") << std::endl;
            return;
        }

        User& u = Kernel::instance().getUser();

        FileFd fd_src = std::stoi(getParam(1));
        int offset = std::stoi(getParam(2));

        File* pFile = u.u_ofiles.GetF(fd_src);

        if (NULL == pFile) {
            std::cout <<("[ERROR]不存在这个fd") << std::endl;
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

        std::cout << "[INFO]成功seek " << real_offset << " bytes, 当前位置： " << pFile->f_offset << std::endl;


    }
    
}