#include "Tools.h"
#include "Tools.h"
#include <cmath>
#include "Tools.h"
#include <ctime>

int TimeHelper::getCurTime()
{
    time_t now = time(0);
    return int(now);
} //获取当前时间

std::string gengerString(int n){
	srand((unsigned)time(NULL));                        
   //产生随机化种子
	printf("生成%d个字符的字符串\n",n);                                      
	std::string str = "";
	for(int i=1;i <= n;i++){
		int flag;                        
		flag = rand()%2;                     
      //随机使flag为1或0，为1就是大写，为0就是小写 
		if(flag == 1)                        
      //如果flag=1 
			str += rand()%('Z'-'A'+1)+'A';       
         //追加大写字母的ascii码 
		else 
			str += rand()%('z'-'a'+1)+'a';       
         //如果flag=0，追加为小写字母的ascii码 
		
	}
	return str;
} 

bool genTestsFile(std::string res_dir, const int test_str_len){
   FILE *fd_des = fopen(res_dir.data(), "wb");
   if (fd_des == NULL){
      Logcat::log("[ERROR]测试文件创建失败！");
      return -1;
   }
   std::string test_str = gengerString(test_str_len);
   //printf("%s\n", test_str.c_str());

   fwrite(test_str.data(), 1, test_str_len, fd_des);
   fclose(fd_des);
   return 1;
}

Bitmap::~Bitmap()
{
    // if(bitmap!=nullptr){
    //     delete bitmap;
    // }
}

Bitmap::Bitmap(int elemNum)
{
    //bitmap=new uint8_t[elemNum/BITMAP_PERBLOCK_SIZE+1]{0};
    bitmapSize = elemNum;
    //   memcpy(bitmap,0,DISK_SIZE/DISK_BLOCK_SIZE/8);
}

//NOTE 注意，盘块号是从0开始的。但是盘块数量是从1开始的
int Bitmap::setBit(int blockID)
{

    if (blockID > bitmapSize - 1 || blockID < 0)
        return ERROR_OFR; //范围非法

    int elemPos = blockID / BITMAP_PERBLOCK_SIZE;
    int innerPos = blockID % BITMAP_PERBLOCK_SIZE;
    //NOTE 显然这里也可以通过循环位移得到对应的掩码，但是不比下面的switch case快
    switch (innerPos)
    {
    case 0:
        bitmap[elemPos] |= 0x01;
        break;
    case 1:
        bitmap[elemPos] |= 0x02;
        break;
    case 2:
        bitmap[elemPos] |= 0x04;
        break;
    case 3:
        bitmap[elemPos] |= 0x08;
        break;
    case 4:
        bitmap[elemPos] |= 0x10;
        break;
    case 5:
        bitmap[elemPos] |= 0x20;
        break;
    case 6:
        bitmap[elemPos] |= 0x40;
        break;
    case 7:
        bitmap[elemPos] |= 0x80;
        break;
    default:
        //cannot be reached!
        break;
    }

    return OK;
}

int Bitmap::unsetBit(int blockID)
{
    if (blockID > bitmapSize - 1 || blockID < 0)
        return ERROR_OFR; //范围非法
    int elemPos = blockID / BITMAP_PERBLOCK_SIZE;
    int innerPos = blockID % BITMAP_PERBLOCK_SIZE;
    switch (innerPos)
    {
    case 0:
        bitmap[elemPos] &= 0xfe;
        break;
    case 1:
        bitmap[elemPos] &= 0xfd;
        break;
    case 2:
        bitmap[elemPos] &= 0xfb;
        break;
    case 3:
        bitmap[elemPos] &= 0xf7;
        break;
    case 4:
        bitmap[elemPos] &= 0xef;
        break;
    case 5:
        bitmap[elemPos] &= 0xdf;
        break;
    case 6:
        bitmap[elemPos] &= 0xbf;
        break;
    case 7:
        bitmap[elemPos] &= 0x7f;
        break;
    default:
        //cannot be reached!
        break;
    }
    return OK;
}
bool Bitmap::getBitStat(int blockID)
{
    if (blockID > bitmapSize - 1 || blockID < 0)
        return false; //范围非法
    int elemPos = blockID / BITMAP_PERBLOCK_SIZE;
    int innerPos = blockID % BITMAP_PERBLOCK_SIZE;
    bool ret = false;
    switch (innerPos)
    {
    case 0:
        ret = (bitmap[elemPos] & 0x01) ;
        break;
    case 1:
        ret = (bitmap[elemPos] & 0x02) ;
        break;
    case 2:
        ret = (bitmap[elemPos] & 0x04) ;
        break;
    case 3:
        ret = (bitmap[elemPos] & 0x08) ;
        break;
    case 4:
        ret = (bitmap[elemPos] & 0x10) ;
        break;
    case 5:
        ret = (bitmap[elemPos] & 0x20);
        break;
    case 6:
        ret = (bitmap[elemPos] & 0x40);
        break;
    case 7:
        ret = (bitmap[elemPos] & 0x80);
        break;
    default:
        //cannot be reached!
        break;
    }
    return ret;
}

/**
 * NOTE 特别注意，这里getAFreeBitNum之后并不会将这个比特位set1.
 * 如果需要获用的话，在getAFreeBitNum之后，要手动setBit
 * 
 */
int Bitmap::getAFreeBitNum()
{
    int ret = -1;
    for (int i = 0; i < ceil(bitmapSize / BITMAP_PERBLOCK_SIZE); i++)
    {
        if (bitmap[i] != 0xFF)
        { //存在空位
            if (!(bitmap[i] & 0x01)) 
            {
                ret = i * BITMAP_PERBLOCK_SIZE + 0;
                break;
            }
            else if (!(bitmap[i] & 0x02))
            {
                ret = i * BITMAP_PERBLOCK_SIZE + 1;
                break;
            }
            else if (!(bitmap[i] & 0x04))
            {
                ret = i * BITMAP_PERBLOCK_SIZE + 2;
                break;
            }
            else if (!(bitmap[i] & 0x08))
            {
                ret = i * BITMAP_PERBLOCK_SIZE + 3;
                break;
            }
            else if (!(bitmap[i] & 0x10))
            {
                ret = i * BITMAP_PERBLOCK_SIZE + 4;
                break;
            }
            else if (!(bitmap[i] & 0x20))
            {
                ret = i * BITMAP_PERBLOCK_SIZE + 5;
                break;
            }
            else if (!(bitmap[i] & 0x40))
            {
                ret = i * BITMAP_PERBLOCK_SIZE + 6;
                break;
            }
            else if (!(bitmap[i] & 0x80))
            {
                ret = i * BITMAP_PERBLOCK_SIZE + 7;
                break;
            }
            // else if (~(bitmap[i] & 0x80))
            // {
            //     ret = i * BITMAP_PERBLOCK_SIZE + 8;
            //     break;
            // }
        }
    }
    return ret;
}

void Bitmap::clear()
{
    memset(bitmap, 0, bitmapSize); //TODO 请确认这句话的正确性。
}
/**
 * 有多少个元素
 */
int Bitmap::getElemNum()
{
    return this->bitmapSize;
}
void Logcat::log(const char *str)
{
    std::cout << str << std::endl;
}

void Logcat::log(const char *tag, const char *str)
{
    std::cout << tag << ":" << str << std::endl;
}

void Logcat::devlog(const char *tag, const char *str)
{
#ifdef IS_DEBUG
    std::cout << tag << ":" << str << std::endl;
#endif
}
void Logcat::devlog(const char *str)
{
#ifdef IS_DEBUG
    std::cout << str << std::endl;
#endif
}
