#include "define.h"
#include "Shell.h"
#include "VFS.h"
#include "Ext2.h"
#include "DiskDriver.h"
#include "BufferCache.h"
#include "Kernel.h"

int main()
{
   int i;
   Shell shell;

   //这些模块是依赖关系，不是组合关系。

   shell.setVFS(&Kernel::instance()->getVFS());

   shell.readUserInput();

   return 0;
}