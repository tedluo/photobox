#include "main.h"
#include "functions.c"

int main(int argc,char **argv)
{
	unsigned char* fbp;

	if(usb() != 0)
	{
		fprintf(stdout,"Faile to mount USB.\n");
		exit(1);
	}
	if(get_media_info(&my_usb_info) != 0)
	{
		fprintf(stdout,"%s:%d:Faled to get usb device information.\n ",__FILE__,__LINE__);
		exit(1);
	}

	if((fbp =init_framebuff(&vinfo,&finfo)) ==NULL)
	{
		fprintf(stdout,"Failed to init framebuff.\n");
		exit(1);
	}
	int code;
	pthread_t pthread_key_ctr;
	if((code = pthread_create(&pthread_key_ctr,NULL,key_ctr,NULL)) < 0)
	{
		fprintf(stderr,"Create thread failed:%s\n",strerror(errno));
		exit(1);
	}
	chdir("./mountPos");	
	//int i;
	//fprintf(stdout,"counter_p =%d\n",my_usb_info.counter_p);
	//fprintf(stdout,"buff_media_picture[1] =%s\n",my_usb_info.buff_media_picture[1]);

	while(1)
	{
		if(picture_mode == 1)
		{
			int i;
			for(i = 0;i < my_usb_info.counter_p;i++)
			{
				memset(buffer,0,screensize);
				picture_to_memory_copy(my_usb_info.buff_media_picture[i]);
				//int line_size =  vinfo.xres *vinfo.bits_per_pixel/8;
				//for(j = 0;j <= vinfo.yres;j++)
				//{	
				//	memcpy(fbp + j * line_size,buffer,line_size);
				//	usleep(100);
				//}
				memset(fbp,0,screensize);
				memcpy(fbp,buffer,screensize);
				memset(buffer,0,screensize);
				sleep(1);
				if(quit_flag == 1)
				{
					break;
				}
				if(back_menu == 1)
				{
					back_menu = 0;
					picture_mode = 0;
					break;
				}

			}
		}
		else if(picture_mode == 2)
		{	
			if(selects_flag == 1)
			{
				//fprintf(stdout,"%s:%d:Can'i go here?\n",__FILE__,__LINE__);
				//fflush(stdout);
				memset(buffer,0,screensize);
				picture_to_memory_copy(my_usb_info.buff_media_picture[selects]);
				//fprintf(stdout,"%s:%d:Can'i go here?\n",__FILE__,__LINE__);
				//fflush(stdout);
				//memset(fbp,0,screensize);
				memcpy(fbp,buffer,screensize);
				memset(buffer,0,screensize);
				selects_flag = 0;
				key_down_flag = 0;
				while(1)
				{
					if(un_selects_flag == 1)
					{
						un_selects_flag = 0;
						key_down_flag = 0;
						break;
					}
					if(back_menu == 1)
					{
						back_menu = 0;
						key_down_flag = 0;
						break;
					}
					if(quit_flag == 1)
					{
						break;
					}
				}
			}	
			show_pictures();
			while(!key_down_flag)
			{
				un_selects_flag = 0;
			
			}
			if(back_menu == 1)
			{
				back_menu = 0;
				picture_mode = 0;
				continue;
			}
			if(quit_flag == 1)
			{
				break;
			}
		}

		if(quit_flag == 1)
		{
			quit_flag = 0;
			break;
		}
	}

	chdir("..");
	umount("./mountPos");
	system("rm  -r mountPos");
	return 0;

}






















