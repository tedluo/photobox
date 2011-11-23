#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>


int main(int argc, char **argv)
{
	pid_t pid;
	int status = 0;
	pid = fork();

	if(pid < 0)
	{
		fprintf(stderr,"Failed to fork process:%s\n",strerror(errno));
		exit(1);
	}
	else if(pid == 0)
	{
		//child.
		system("madplay *.mp3");
		fprintf(stdout,"%s:%d:Can i go here?",__FILE__,__LINE__);
		fflush(stdout);
		exit(1);
	}
	else
	{
		wait(&status);
		fprintf(stdout,"%s:%d:Can i go here?",__FILE__,__LINE__);
		fflush(stdout);
		fprintf(stdout,"status = %d\n",status);
		
	}


	return 0;
}
