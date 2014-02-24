// daemon.cpp : Defines the entry point for the console application.
//

#include "daemon_frame.h"
#include "core_main.h"

int main(int argc, char* argv[])
{
	CREATE_DAEMON_FRAME();

	DAEMON_FRAME()->init();
	DAEMON_FRAME()->start();
	
	core_main();

	DAEMON_FRAME()->destroy();

	DESTROY_DAEMON_FRAME();

	return 0;
}

