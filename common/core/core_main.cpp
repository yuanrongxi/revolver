#include "revolver/base_os.h"
#include "core/core_main.h"

#include <iostream>
using namespace std;

#ifdef WIN32
void ignore_pipe()
{
}

int32_t core_main()
{
	char c = 'a';
	while((c = getchar()) != 'e')
	{
		usleep(10000);
	}

	return 0;
}
#else
#define SIG_PROG_EXIT	41

static bool TERMINATE_OBJ = false;

static void sig_exit_proc(int signo)
{
	if (signo == SIG_PROG_EXIT)
	{
		TERMINATE_OBJ = true;
	}

	cout << "recive signo = " <<  signo << endl;
}

void handle_pipe(int sig)
{
	//cout << "handler pipe OK!!" << endl;
}

void ignore_pipe() //ºöÂÔPIPEÐÅºÅ
{
	struct sigaction action;
	action.sa_handler = handle_pipe;
	sigemptyset(&action.sa_mask);
	action.sa_flags = 0;
	sigaction(SIGPIPE, &action, NULL);
}

int core_main()
{
	if (signal(SIG_PROG_EXIT, sig_exit_proc) == SIG_ERR)
	{
		cout << "signal failed!!!" << endl;
		return 1;
	}

	while(!TERMINATE_OBJ)
	{
		usleep(10000);
	}

	return 0;
}

#endif
