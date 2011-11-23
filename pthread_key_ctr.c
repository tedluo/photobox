#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <termios.h>

int my_usb_info_counter_p = 10;

int picture_mode = 0;
int music_flag = 0;


int selects = 0;
int quit_flag = 0;
int back_menu = 0;

void *key_ctr(void* arg);

int main(int argc,char **argv)
{

	pthread_t pthread_key_ctr;
	int code;
	if((code = pthread_create(&pthread_key_ctr,NULL,key_ctr,NULL)) < 0)
	{
		fprintf(stderr,"Create thread failed:%s\n",strerror(errno));
		exit(1);
	}
	while(1)
	{
		fprintf(stdout,"Hello?,now selects = %d\n",selects);
		sleep(1);
		if(quit_flag == 1)
		{
			break;
		}
	}
	pthread_exit((void*)0);
}


void *key_ctr(void* arg)
{
	int num;
	char buffer_char[1];
	struct termios new;
	struct termios old;
	tcgetattr(0,&old);
	new = old;
	new.c_lflag &= ~(ICANON);
	tcsetattr(0,TCSANOW,&new);

	while(1)
	{
		if((num = read(0,buffer_char,1)) == 0)
		{
			continue;
		}
		else
		{
			if(buffer_char[0] == 'y'|| buffer_char[0] == 'Y' )
			{
				picture_mode = 1;
			}
			else if(buffer_char[0] == 'u' || buffer_char[0] == 'U')
			{
				picture_mode = 2;
			}
			else if(buffer_char[0] == 'M' || buffer_char[0] == 'm')
			{
				music_flag = 1;
			}
			else if(buffer_char[0] == 'A' || buffer_char[0] == 'a')
			{
				if(selects == 0)
				{
					selects = my_usb_info.counter_p;
				}
				else
				{
					selects--;
				}
			}
			else if(buffer_char[0] == 'd' || buffer_char[0] == 'D')
			{
				if(selects == my_usb_info.counter_p)
				{
					selects = 0;
				}
				else
				{
					selects++;
				}
			}
			else if(buffer_char[0] == 'W' || buffer_char[0] == 'w')
			{
				if(selects > (sqrt(my_usb_info.counter_p/25) + 1) * 5)
				{
					selects -= (sqrt(my_usb_info.counter_p/25) + 1) * 5;
				}
				
			}
			else if(buffer_char[0] == 's' || buffer_char[0] == 'S')
			{
				if((selects <= (sqrt(my_usb_info.counter_p/25) + 1) * 5*\
						(sqrt(my_usb_info.counter_p/25)-1))&& selects > 0)
				{
					selects += (sqrt(my_usb_info.counter_p/25) + 1) * 5;
				}

			}
			else if(buffer_char[0] == 'b' || buffer_char[0] == 'B')
			{
				back_menu = 1;
			}
			else if(buffer_char[0] == 'q' || buffer_char[0] == 'Q')
			{
				quit_flag = 1;
			}
		}
	}
	pthread_exit((void *) 0);
}
