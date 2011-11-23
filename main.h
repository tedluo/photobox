#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include "jpeglib.h"
#include "jerror.h"
#include <termios.h>
#include <pthread.h>
#include <dirent.h>


#include <sys/mount.h> 

#define BUFFER_SIZE 4069
#define CBUFFER_SIZE 100
#define BUFF_SIZE 20
#define BUFFER_SIZE_LONG 1024
#define CBUFFER_SIZE_LONG 1024

#define device "/dev/fb0"

//********************************************************************
// global walue
//********************************************************************
struct usb_info{
	char **buff_media_picture;
	char **buff_media_music;
	char *device_name;
	int counter_p;
	int counter_m;
	}my_usb_info;


#define device "/dev/fb0"
//********************************************************************
// 		framebuffer informations These will be initionlized in
//	unsigned char* init_framebuff(struct fb_var_screeninfo* vinfo,
//				struct fb_fix_screeninfo* finfo);
//********************************************************************
struct fb_var_screeninfo vinfo;
struct fb_fix_screeninfo finfo;
//********************************************************************
//	  jpeg picture informations,This will be initionlized in
//		unsigned char* picture_to_memory_copy(const char*);
//********************************************************************
struct jpeg_decompress_struct cinfo;

//********************************************************************
//	  fbp ,a buffer for framebuff,This will be initionlized in
//  unsigned char* init_framebuff(struct fb_var_screeninfo* vinfo,
//				struct fb_fix_screeninfo* finfo);
//********************************************************************
unsigned char *fbp;
//********************************************************************
//	   buffer,a buffer for fbp,This will be initionlized in 
//		unsigned char* picture_to_memory_copy(const char*);
//********************************************************************
unsigned char* buffer;


long screensize;

int key_down_flag = 0;

//********************************************************************
//Use Y or y to change picture_mod,This argument change che mode that
//how to show pictures
//********************************************************************
int picture_mode = 0;

//********************************************************************
//use M or m to change music_flag ,if this argument is set ,then music
//will play 
//********************************************************************
int music_flag = 0;

//********************************************************************
//Usage q or Q to change quit_flag,if this argument is set ,in any ca-
//se the process will quit
//********************************************************************
int quit_flag = 0;
//********************************************************************
//Usage b or B to change back_menu,if this argument is set ,the proce
//ss will quit curren circle
//********************************************************************
int back_menu = 0;
//********************************************************************
//Use a A s S w W D d to change selects,this argument show which pictu
//re is selected now
//********************************************************************
int selects = 0;
//********************************************************************
//Use Enter to change  selects_flag,this argument will printf the pict
//ure which is selected by argument selects
//********************************************************************
int selects_flag = 0;
//********************************************************************
//Use this argument to cancle show the pionted picture 
//********************************************************************
int un_selects_flag = 0;

//********************************************************************
//				functions
//********************************************************************


//********************************************************************
//This fuction decompress the jpeg picture according to the picture's
//size,and the framebuffer sizereturen a pointer  that pointed to the 
//buffer which make a bufffor famebuffer's fbp.
//********************************************************************
int picture_to_memory_copy(const char*);



//********************************************************************
// pthread control funtion,control the key information form the board
//********************************************************************
void *key_ctr(void* arg);


//********************************************************************
// get usb info,include picture,and music,all this informatim will fi-
// ll int struct  usb_info
//********************************************************************
// mount
int get_media_info(struct usb_info*);


//********************************************************************
//get usb information,while there is a usb device mounted to the sys-
//tem,This fun mount the usb device to the pointed dir
//********************************************************************
int usb(void);

//********************************************************************
//This function get the information of framebuffer,and return a pointer
//that point to a memory map which connect to framebuffer's memory.
//********************************************************************
unsigned char *init_framebuff(struct fb_var_screeninfo* vinfo,struct fb_fix_screeninfo* finfo);

//********************************************************************
//The functions below convert jpeg pictures' information to showable 
//formate for current framebuffer
//********************************************************************
int setpixel16(void* memoff,int width,int height,int x,int y,unsigned short color,unsigned int location);
int setpixel32(void* memoff,int width,int height,int x,int y,unsigned int color ,unsigned int location);
unsigned int RGB24_To_RGB32(unsigned char red,unsigned char green,unsigned char blue);
unsigned short RGB24_To_RGB16(unsigned char red,unsigned char green,unsigned char blue);
//usb_picture
//********************************************************************
//This function gives a show of all the pictures in the usb device
//********************************************************************
int show_pictures(void);

//********************************************************************
//This function decompress the jpg picture to the buffer according to 
//the given rules
//********************************************************************
unsigned char* show_all_jpeg_decompress_by_line(struct fb_var_screeninfo *vinfo,struct fb_fix_screeninfo *finfo,\
							struct jpeg_decompress_struct* cinfo,unsigned char* fbp,const char * picture,int n);







