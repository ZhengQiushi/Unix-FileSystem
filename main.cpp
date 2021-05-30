#include "define.h"
#include "Shell.h"
#include "FileSystem.h"
#include "DiskDriver.h"
#include "BufferCache.h"
#include "Kernel.h"

int main()
{
   int i;
   Shell shell(Kernel::instance());

   shell.readUserInput();

   return 0;
}