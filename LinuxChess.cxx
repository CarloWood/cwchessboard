#include "sys.h"
#include "debug.h"
#include "LinuxChessApplication.h"

int main(int argc, char* argv[])
{
  Debug(NAMESPACE_DEBUG::init());
  Dout(dc::notice, "Entered main()");
  LinuxChessApplication::create()->run(argc, argv);
  Dout(dc::notice, "Leaving main()");
}
