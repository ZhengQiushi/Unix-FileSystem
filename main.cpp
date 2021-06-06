#include "define.h"
#include "Shell.h"
#include "FileSystem.h"
#include "DiskDriver.h"
#include "BufferCache.h"
#include "Kernel.h"

/* 是否需要产生测试字符串 */
// #define GEN_TEST_FILE 

int main(){
   Shell shell(Kernel::instance());

   std::cout << sizeof(SuperBlock) << std::endl;
   std::cout << sizeof(DiskInode) << std::endl;
#ifdef GEN_TEST_FILE
   if(0 > genTestsFile("../assets/tests"))
      return -1;
#endif

   shell.run();

   return 0;
}