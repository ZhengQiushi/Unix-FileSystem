#include "define.h"
#include "Shell.h"
#include "VFS.h"
#include "FileSystem.h"
#include "DiskDriver.h"
#include "BufferCache.h"
#include "Kernel.h"

int main()
{
   int i;
   Shell shell(Kernel::instance());

   //这些模块是依赖关系，不是组合关系。



   printf("%d %d\n", sizeof(SuperBlock), sizeof(SuperBlock));

   shell.readUserInput();

   return 0;
}