#include <3ds.h>
#include <cstdio>

#include "sys.h"

namespace sys
{
	bool run = true;
	void init()
	{
		hidInit();
		amInit();
	}

	void exit()
	{
		hidExit();
		amExit();
	}
}
