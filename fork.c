#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <wait.h>


int selects = 1;

int main(int argc, char** argv)
{
	pid_t pid;

	pid = fork();

	if(pid < 0)
	{
		fprintf(stderr,"Fork failed:%s\n",strerror(errno));
		exit(1);
	}
	else if(pid == 0)
	{
		while(1)
		{
			fprintf(stdout,"Child[%d]selects = %d \n",getpid(),selects);
			//selects ++;
			sleep(1);
		}
	}
	else
	{
	//	wait(NULL);
		while(1)
		{
			fprintf(stdout,"Parent[%d]selects = %d \n",getpid(),selects);
			selects++;
			sleep(1);
		}
	}
	return 0;
}
