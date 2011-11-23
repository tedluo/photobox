unsigned char*  init_framebuff(struct fb_var_screeninfo* vinfo,struct fb_fix_screeninfo* finfo)
{
	int fd;
	//step1: open device
	if((fd = open(device,O_RDWR)) < 0)
	{
		fprintf(stdout,"Open device failed:%s\n",strerror(errno));
		return NULL;
	}


	//step2: get device information->
	if(ioctl(fd,FBIOGET_FSCREENINFO,finfo) < 0)
	{
		fprintf(stderr,"Get device fix information failed:%s\n",strerror(errno));
		return NULL;
	}
	if(ioctl(fd,FBIOGET_VSCREENINFO,vinfo) < 0)
	{
		fprintf(stderr,"Get device var information failed:%s\n",strerror(errno));
		return NULL;
	}
	
	//step3: mmap device adress to memory
	screensize = vinfo->xres * vinfo->yres * vinfo->bits_per_pixel / 8;

	if((fbp = mmap(NULL,screensize,PROT_READ | PROT_WRITE, MAP_SHARED,fd,0)) == (void *) -1)
	{
		fprintf(stderr,"memory map error:%s\n",strerror(errno));
		return NULL;
	}
	if((buffer = (unsigned char*)malloc(screensize)) == NULL)
	{
		fprintf(stderr,"Memory Error with Malloc 1:%s\n",strerror(errno));
		return NULL;
	}
	close(fd);
	return fbp;
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
				key_down_flag = 1;
			}
			else if(buffer_char[0] == 'u' || buffer_char[0] == 'U')
			{
				picture_mode = 2;
				key_down_flag = 1;
			}
			else if(buffer_char[0] == 'M' || buffer_char[0] == 'm')
			{
				music_flag = 1;
				key_down_flag = 1;
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
				key_down_flag = 1;
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
				key_down_flag = 1;
			}
			else if(buffer_char[0] == 'W' || buffer_char[0] == 'w')
			{
				if(selects >= (sqrt(my_usb_info.counter_p/25) + 1) * 5)
				{
					selects -= (sqrt(my_usb_info.counter_p/25) + 1) * 5;
				}
				key_down_flag = 1;
				
			}
			else if(buffer_char[0] == 's' || buffer_char[0] == 'S')
			{
				if(selects <= my_usb_info.counter_p - (my_usb_info.counter_p%((int)(sqrt(my_usb_info.counter_p/25)+1))*5))
				{
					int selects_copy = selects + ((sqrt(my_usb_info.counter_p/25) + 1)*5);
					if(selects_copy <= my_usb_info.counter_p)
					{	
						selects += (sqrt(my_usb_info.counter_p/25) + 1) * 5;
					}
					else
					{
						selects = selects;
					}
				}
				key_down_flag = 1;

			}
			else if(buffer_char[0] == 'b' || buffer_char[0] == 'B')
			{
				back_menu = 1;
				key_down_flag = 1;
			}
			else if(buffer_char[0] == 'q' || buffer_char[0] == 'Q')
			{
				quit_flag = 1;
				key_down_flag = 1;
			}
			else if(buffer_char[0] == '\n')
			{
				selects_flag = 1;
				key_down_flag = 1;
			}
			else if(buffer_char[0] == ' ')
			{
				un_selects_flag = 1;
				key_down_flag = 1;
			}
			else
			{
				key_down_flag = 1;
			}

		}
	}
	pthread_exit((void *) 0);
}

int show_pictures()
{
	//init_framebuff(&vinfo,&finfo);
	int i;
	for(i = 0; i < my_usb_info.counter_p ; i ++)
	{
		if((show_all_jpeg_decompress_by_line(&vinfo,&finfo,&cinfo,buffer,my_usb_info.buff_media_picture[i],i)) == NULL)
		{
			return 1;
		}
		//FIXME:add a flag to every pictrue?
	}
	memcpy(fbp,buffer,screensize);
	//free(buffer);
	return 0;

}

int picture_to_memory_copy(const char * picture)
{
	struct jpeg_error_mgr jerror;


	FILE* fp;

	if((fp = fopen(picture,"rb")) ==NULL)
	{
		fprintf(stderr,"Load picture %s failed:%s\n",picture,strerror(errno));
		exit(1);
	}

	cinfo.err = jpeg_std_error(&jerror);
	jpeg_create_decompress(&cinfo);

	jpeg_stdio_src(&cinfo,fp);
	jpeg_read_header(&cinfo,TRUE);

	jpeg_start_decompress(&cinfo);

	unsigned char *buffer_line;

	if((buffer_line = (unsigned char*)malloc(cinfo.output_width * cinfo.output_components)) == NULL)
	{
		fprintf(stderr,"%s:%d:Memory Error with Malloc 1:%s\n",__FILE__,__LINE__,strerror(errno));
		exit(1);
	}
	//unsigned screensize = vinfo.xres*vinfo.yres*vinfo.bits_per_pixel/8;
#if 0
	if((buffer = (unsigned char*)malloc(vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8)) == NULL)
	{
		fprintf(stderr,"%s:%d:Memory Error with Malloc 1:%s\n",__FILE__,__LINE__,strerror(errno));
		exit(1);
	}
#endif

	unsigned int x = 0;
	unsigned int y = 0;
	unsigned int width = vinfo.xres;
	unsigned int height =  vinfo.yres;
	unsigned int location;
	int y_copy = 0;
	int x_copy = 0;

	if((cinfo.output_width <= width) && (cinfo.output_height <= height))
	{
		//location  deciede where to put the picture.
		location = ((((height - cinfo.output_height) / 2)) * finfo.line_length / (vinfo.bits_per_pixel / 8) \
																			+ (width - cinfo.output_width)/2);
		while(cinfo.output_scanline < cinfo.output_height)
		{
			jpeg_read_scanlines(&cinfo,&buffer_line,1);
			if(vinfo.bits_per_pixel == 16)
			{
				unsigned short color16;
				for(x = 0;x < cinfo.output_width;x ++)
				{
					color16 = RGB24_To_RGB16(buffer_line[3 * x],buffer_line[3 * x + 1],buffer_line[3 * x + 2]);
					setpixel16(buffer,width,height,x,y,color16,location);
				}
			}
			else if(vinfo.bits_per_pixel == 32)
			{
				unsigned int color32;
				for(x = 0;x < cinfo.output_width;x++)
				{
					color32 = RGB24_To_RGB32(buffer_line[3 * x],buffer_line[3 * x + 1],buffer_line[3 * x + 2]);
					setpixel32(buffer,width,height,x,y,color32,location);
				}
			}
			y++;
		}
	}
	else
	{
		location = ((((height - cinfo.output_height/2) / 2)) * vinfo.xres + (width - cinfo.output_width/2)/2);

		while(cinfo.output_scanline < cinfo.output_height)
		{
			jpeg_read_scanlines(&cinfo,&buffer_line,1);
			if(y % 2 == 0)
			{
				y_copy = y / 2;
				if(vinfo.bits_per_pixel == 16)
				{
					unsigned short color16;
					for(x = 0;x < cinfo.output_width;x ++)
					{
						if(x % 2 ==0)
						{
							x_copy = x / 2 ;
							color16 = RGB24_To_RGB16(buffer_line[3 * x],buffer_line[3 * x + 1],buffer_line[3 * x + 2]);
							setpixel16(buffer,width,height,x_copy,y_copy,color16,location);
						}
					}
				}
				else if(vinfo.bits_per_pixel == 32)
				{
					unsigned int color32;
					for(x = 0;x < cinfo.output_width;x++)
					{
						if(x % 2 == 0)
						{
							x_copy = x / 2;
							color32 = RGB24_To_RGB32(buffer_line[3 * x],buffer_line[3 * x + 1],buffer_line[3 * x + 2]);
							setpixel32(buffer,width,height,x_copy,y_copy,color32,location);
						}
					}
				}
			}
			y++;
		}
	}

	//memcpy(fbp,buffer,screensize);
	//free(buffer);
	free(buffer_line);
	fclose(fp);

	return 0;
}


unsigned char* show_all_jpeg_decompress_by_line(struct fb_var_screeninfo *vinfo,struct fb_fix_screeninfo *finfo,\
										struct jpeg_decompress_struct* cinfo,unsigned char* buffer,const char * picture,int n)
{
	struct jpeg_error_mgr jerror;


	FILE* fp;

	if((fp = fopen(picture,"rb")) ==NULL)
	{
		fprintf(stderr,"Load picture %s failed:%s\n",picture,strerror(errno));
		return(NULL);
	}

	cinfo->err = jpeg_std_error(&jerror);
	jpeg_create_decompress(cinfo);

	jpeg_stdio_src(cinfo,fp);
	jpeg_read_header(cinfo,TRUE);

	jpeg_start_decompress(cinfo);
	

	unsigned char *buffer_line;

	if((buffer_line = (unsigned char*)malloc(cinfo->output_width * cinfo->output_components)) == NULL)
	{
		fprintf(stderr,"%s:%d:Memory Error with Malloc 1:%s\n",__FILE__,__LINE__,strerror(errno));
		return(NULL);
	}

	unsigned int x;
	unsigned int y = 0;
	unsigned int width = vinfo->xres;
	unsigned int height =  vinfo->yres;

	//fprintf(stdout,"vinfo's xres =  %d\n",vinfo->xres);
	//fprintf(stdout,"vinfo's yres =  %d\n",vinfo->yres);
	//fprintf(stdout,"vinfo's line length =  %d\n",finfo->line_length);
	//propotions for double 5 
	int propotion =sqrt(my_usb_info.counter_p / 25) + 1;
	int lines = (n / (5 * propotion));
	int rows  = n % (5 * propotion);
	//fprintf(stdout,"lines = %d\n",lines);

	unsigned int location;

	//location  deciede where to put the picture.
	if((cinfo->output_width <= width) && (cinfo->output_height <= height))
	{
		location = ((((((height - cinfo->output_height) / 2) / propotion) / 5 ) / 2) *  finfo->line_length \
						+ (width - cinfo->output_width)/(2 * propotion * 5)) + rows * (width / (5 * propotion))\
				  			 + lines * (height /(5 * propotion * (vinfo->bits_per_pixel / 8)))* finfo->line_length ;
		while(cinfo->output_scanline < cinfo->output_height)
		{
			jpeg_read_scanlines(cinfo,&buffer_line,1);
			if( y % (propotion * 5) == 0)
			{
				int y0 = y / (5 * propotion);
				if(vinfo->bits_per_pixel == 16)
				{
					unsigned short color16;
					for(x = 0;x < cinfo->output_width;x ++)
					{
						if((x % (propotion * 5)) == 0)// every propotion * 5 pixels get one pixel
						{
				 			int x0 = x / (5 * propotion);// the pixel 's current xres.
							//This code below 11 lines to make sure that between every picture three will 
							//have a space
							if( x >= 15 && (x <= vinfo->xres - 15) &&  y>= 15 && (y <= vinfo->yres - 20))
							{
								color16 = RGB24_To_RGB16(buffer_line[3 * x],buffer_line[3 * x + 1],buffer_line[3 * x + 2]);
								setpixel16(buffer,width,height,x0,y0,color16,location);
							}
							else
							{
								color16 = 0x00;
								setpixel16(buffer,width,height,x0,y0,color16,location);
							}
							//These code below 4 line to make a flag for every selected pictures 
							if((n == selects) && ((x<=15 || (x >= cinfo->output_width -15)) ||( y <= 15 ||( y >= cinfo->output_height -20)))) 
							{
								color16 = 0xFF;
								setpixel16(buffer,width,height,x0,y0,color16,location);
							}
						}
					}
				}
				else if(vinfo->bits_per_pixel == 32)
				{
			 		unsigned int color32;
					for(x = 0;x < cinfo->output_width;x++)
					{
						if(( x %(propotion * 5)) == 0)
						{
							int x0 = x / (propotion * 5 );
							if( x >= 15 && (x <= vinfo->xres - 15) &&  y>= 15 && (y <= vinfo->yres - 20))
							{
								color32 = RGB24_To_RGB32(buffer_line[3 * x],buffer_line[3 * x + 1],buffer_line[3 * x + 2]);
								setpixel32(buffer,width,height,x0,y0,color32,location);
							}
							else
							{
								color32 = 0x0000;
								setpixel32(buffer,width,height,x0,y0,color32,location);
							}
							if((n == selects) && ((x<=15 || (x >= cinfo->output_width -15)) ||( y <= 15 ||( y >= cinfo->output_height -20)))) 
							{
								color32 = 0xFFFF;
								setpixel32(buffer,width,height,x0,y0,color32,location);
							}
							//color32 = RGB24_To_RGB32(buffer_line[3 * x],buffer_line[3 * x + 1 ],buffer_line[3 * x + 2]);
							//setpixel32(buffer,width,height,x0,y0,color32,location);
						}
					}	
				}
			}
			y++;
		}
	}
	else if((cinfo->output_width < width) && (cinfo->output_height > height))
	{
		location = ((width - cinfo->output_width)/(2 * propotion * 5)) + rows * (width / (5 * propotion))\
				  			 + lines * (height /(5 * propotion * (vinfo->bits_per_pixel / 8)))* finfo->line_length ;
		while(cinfo->output_scanline < vinfo->yres)
		{
			jpeg_read_scanlines(cinfo,&buffer_line,1);
			if( y % (propotion * 5) == 0)
			{
				int y0 = y / (5 * propotion);
				if(vinfo->bits_per_pixel == 16)
				{
					unsigned short color16;
					for(x = 0;x < cinfo->output_width;x ++)
					{
						if((x % (propotion * 5)) == 0)// every propotion * 5 pixels get one pixel
						{
				 			int x0 = x / (5 * propotion);// the pixel 's current xres.
							if( x >= 15 && (x <= vinfo->xres - 15) &&  y>= 15 && (y <= vinfo->yres - 20))
							{
								color16 = RGB24_To_RGB16(buffer_line[3 * x],buffer_line[3 * x + 1],buffer_line[3 * x + 2]);
								setpixel16(buffer,width,height,x0,y0,color16,location);
							}
							else
							{
								color16 = 0x00;
								setpixel16(buffer,width,height,x0,y0,color16,location);
							}
							if((n == selects) && ((x<=15 || (x >= cinfo->output_width -15)) ||( y <= 15 ||( y >= vinfo->yres -20)))) 
							{
								color16 = 0xFF;
								setpixel16(buffer,width,height,x0,y0,color16,location);
							}
						}
					}
				}
				else if(vinfo->bits_per_pixel == 32)
				{
			 		unsigned int color32;
					for(x = 0;x < cinfo->output_width;x++)
					{
						if(( x %(propotion * 5)) == 0)
						{
							int x0 = x / (propotion * 5 );
							if( x >= 15 && (x <= vinfo->xres - 15) &&  y>= 15 && (y <= vinfo->yres - 20))
							{
								color32 = RGB24_To_RGB32(buffer_line[3 * x],buffer_line[3 * x + 1],buffer_line[3 * x + 2]);
								setpixel32(buffer,width,height,x0,y0,color32,location);
							}
							else
							{
								color32 = 0x0000;
								setpixel32(buffer,width,height,x0,y0,color32,location);
							}
							if((n == selects) && ((x<=15 || (x >= cinfo->output_width -15)) ||( y <= 15 ||( y >= vinfo->yres -20)))) 
							{
								color32 = 0xFFFF;
								setpixel32(buffer,width,height,x0,y0,color32,location);
							}
							//color32 = RGB24_To_RGB32(buffer_line[3 * x],buffer_line[3 * x + 1 ],buffer_line[3 * x + 2]);
							//setpixel32(buffer,width,height,x0,y0,color32,location);
						}
					}	
				}
			}
			y++;
		}
	}
	else if((cinfo->output_width > width) && (cinfo->output_height < height))
	{
		location = (((((height - cinfo->output_height) / 2) / propotion) / 5 ) / 2) *  finfo->line_length \
									+ rows * (width / (5 * propotion))+ lines * (height /(5 * propotion * \
														(vinfo->bits_per_pixel / 8)))* finfo->line_length ;
		while(cinfo->output_scanline < cinfo->output_height)
		{
			jpeg_read_scanlines(cinfo,&buffer_line,1);
			if( y % (propotion * 5) == 0)
			{
				int y0 = y / (5 * propotion);
				if(vinfo->bits_per_pixel == 16)
				{
					unsigned short color16;
					for(x = 0;x < vinfo->xres;x ++)
					{
						if((x % (propotion * 5)) == 0)// every propotion * 5 pixels get one pixel
						{
				 			int x0 = x / (5 * propotion);// the pixel 's current xres.
							if( x >= 15 && (x <= vinfo->xres - 15) &&  y>= 15 && (y <= vinfo->yres - 20))
							{
								color16 = RGB24_To_RGB16(buffer_line[3 * x],buffer_line[3 * x + 1],buffer_line[3 * x + 2]);
								setpixel16(buffer,width,height,x0,y0,color16,location);
							}
							else
							{
								color16 = 0x00;
								setpixel16(buffer,width,height,x0,y0,color16,location);
							}
							if((n == selects) && ((x<=15 || (x >= vinfo->xres -15)) ||( y <= 15 ||( y >= cinfo->output_height -20)))) 
							{
								color16 = 0xFF;
								setpixel16(buffer,width,height,x0,y0,color16,location);
							}
						}
					}
				}
				else if(vinfo->bits_per_pixel == 32)
				{
			 		unsigned int color32;
					for(x = 0;x < vinfo->xres;x++)
					{
						if(( x %(propotion * 5)) == 0)
						{
							int x0 = x / (propotion * 5 );
							if( x >= 15 && (x <= vinfo->xres - 15) &&  y>= 15 && (y <= vinfo->yres - 20))
							{
								color32 = RGB24_To_RGB32(buffer_line[3 * x],buffer_line[3 * x + 1],buffer_line[3 * x + 2]);
								setpixel32(buffer,width,height,x0,y0,color32,location);
							}
							else
							{
								color32 = 0x0000;
								setpixel32(buffer,width,height,x0,y0,color32,location);
							}
							if((n == selects) && ((x<=15 || (x >= vinfo->xres -15)) ||( y <= 15 ||( y >= cinfo->output_height -20)))) 
							{
								color32 = 0xFFFF;
								setpixel32(buffer,width,height,x0,y0,color32,location);
							}
							//color32 = RGB24_To_RGB32(buffer_line[3 * x],buffer_line[3 * x + 1 ],buffer_line[3 * x + 2]);
							//setpixel32(buffer,width,height,x0,y0,color32,location);
						}
					}	
				}
			}
			y++;
		}
	}

	//unsigned int pwidth = cinfo->output_width;
	//unsigned int pheight = cinfo->output_height;
	else
	{
		location =  rows * (width / (5 * propotion))\
				  			 + lines * (height /(5 * propotion * (vinfo->bits_per_pixel / 8)))* finfo->line_length ;
		while(cinfo->output_scanline < vinfo->yres)
		{
			jpeg_read_scanlines(cinfo,&buffer_line,1);
			if( y % (propotion * 5) == 0)
			{
				int y0 = y / (5 * propotion);
				if(vinfo->bits_per_pixel == 16)
				{
					unsigned short color16;
					for(x = 0;x < vinfo->xres;x ++)
					{
						if((x % (propotion * 5)) == 0)// every propotion * 5 pixels get one pixel
						{
				 			int x0 = x / (5 * propotion);// the pixel 's current xres.
							if( x >= 15 && (x <= vinfo->xres - 15) &&  y>= 15 && (y <= vinfo->yres - 20))
							{
								color16 = RGB24_To_RGB16(buffer_line[3 * x],buffer_line[3 * x + 1],buffer_line[3 * x + 2]);
								setpixel16(buffer,width,height,x0,y0,color16,location);
							}
							else
							{
								color16 = 0x00;
								setpixel16(buffer,width,height,x0,y0,color16,location);
							}
							if((n == selects) && ((x<=15 || (x >= vinfo->xres -15)) ||( y <= 15 ||( y >= vinfo->yres -20)))) 
							{
								color16 = 0xFF;
								setpixel16(buffer,width,height,x0,y0,color16,location);
							}
						}
					}
				}
				else if(vinfo->bits_per_pixel == 32)
				{
			 		unsigned int color32;
					for(x = 0;x < vinfo->xres;x++)
					{
						if(( x %(propotion * 5)) == 0)
						{
							int x0 = x / (propotion * 5 );
							if( x >= 15 && (x <= vinfo->xres - 15) &&  y>= 15 && (y <= vinfo->yres - 20))
							{
								color32 = RGB24_To_RGB32(buffer_line[3 * x],buffer_line[3 * x + 1],buffer_line[3 * x + 2]);
								setpixel32(buffer,width,height,x0,y0,color32,location);
							}
							else
							{
								color32 = 0x0000;
								setpixel32(buffer,width,height,x0,y0,color32,location);
							}
							if((n == selects) && ((x<=15 || (x >= vinfo->xres -15)) ||( y <= 15 ||( y >= vinfo->yres -20)))) 
							{
								color32 = 0xFFFF;
								setpixel32(buffer,width,height,x0,y0,color32,location);
							}
							//color32 = RGB24_To_RGB32(buffer_line[3 * x],buffer_line[3 * x + 1 ],buffer_line[3 * x + 2]);
							//setpixel32(buffer,width,height,x0,y0,color32,location);
						}
					}	
				}
			}
			y++;
		}
	}

	fclose(fp);
	free(buffer_line);
	return buffer;
}

unsigned short RGB24_To_RGB16(unsigned char red,unsigned char green,unsigned char blue)
{
	unsigned short newblue = (blue >> 3) & 0x001F;
	unsigned short newgreen = ((green >> 2) << 5) & 0x07E0;
	unsigned short newred = ((red >> 3) << 11) & 0xF800;
	
	return (unsigned short)(newblue | newgreen | newred);
}

unsigned int RGB24_To_RGB32(unsigned char red,unsigned char green,unsigned char blue)
{
	unsigned int newblue = blue & 0x000000FF;
	unsigned int newgreen = (green & 0x000000FF) << 8;
	unsigned int newred = (red & 0x000000FF) << 16;
	return ((unsigned int)(newblue | newgreen | newred));

}

int setpixel16(void* memoff,int width,int height,int x,int y,unsigned short color,unsigned int location)
{
	if((x <= width) && (y <= height))
	{
		unsigned char bit1 = (unsigned char )(color & 0x00FF);
		unsigned char bit2 = (unsigned char )((color & 0xFF00) >> 8);
		unsigned char *dest =(unsigned char*)(memoff +location * 2 + y * width * 2  + x * 2);
		*dest = bit1;
		*(dest + 1) = bit2;
	}
	return 0;
}


int setpixel32(void* memoff,int width,int height,int x,int y,unsigned int color, unsigned int location)
{
	if((x <= width) && (y <= height))
	{
		unsigned int *dest = (unsigned int *)((memoff + location*4 ) + y * width * 4 + x * 4);
		*dest = color;
	}
	return 0;
}



//usb
int usb (void)
{
  DIR *dir;
  struct dirent *dt;
  system("mkdir mountPos");

  if ((dir = opendir ("/dev")) == NULL)
    {
      perror ("opendir error");
      return 1;
    }
  while ((dt = readdir (dir)) != NULL)
    {
	 if (strcmp (dt->d_name, "sdb") == 0)
	{
	  mount("/dev/sdb", "./mountPos", "vfat", MS_SYNCHRONOUS, NULL);
	  return 0;
	}
      if (strcmp (dt->d_name, "sdb1") == 0)
	{
	  mount("/dev/sdb1", "./mountPos", "vfat", MS_SYNCHRONOUS, NULL);
	  return 0;
	}
      if (strcmp (dt->d_name, "sdc1") == 0)
	{
	  mount("/dev/sdc1", "./mountPos", "vfat", MS_SYNCHRONOUS, NULL);
	  return 0;
	}
      if (strcmp (dt->d_name, "sdd1") == 0)
	{
	  mount("/dev/sdd1", "./mountPos", "vfat", MS_SYNCHRONOUS, NULL);
	  return 0;
	}
      if (strcmp (dt->d_name, "sde1") == 1)
	{
	  mount("/dev/sde1", "./mountPos", "vfat", MS_SYNCHRONOUS, NULL);
	  return 0;
	}
      if (strcmp (dt->d_name, "sdc4") == 1)
	{
	  mount("/dev/sdc4", "./mountPos", "vfat", MS_SYNCHRONOUS, NULL);
	  return 0;
	}
      else
	{
	  continue;
	}
    }
  return 1;
}

int get_media_info(struct usb_info *my_usb_info)
{	
//	char *buff_old_device[BUFF_SIZE];
//	int counter_old = 0;

//	char *buff_new_device[BUFF_SIZE];
//	int counter_new = 0;

	char *buff_media_picture[BUFFER_SIZE_LONG];
	int counter_p = 0;

	char *buff_media_music[BUFFER_SIZE_LONG];
	int counter_m = 0;


	int i;
#if 0
	for(i =0 ;i < BUFF_SIZE; i++)
	{
		if((buff_old_device[i] = malloc(CBUFFER_SIZE)) == NULL)
		{
			fprintf(stdout,"Memory Error!\n");
			exit(1);
		}
	}
#endif
#if 0
	for(i =0 ;i < BUFF_SIZE; i++)
	{
		if((buff_new_device[i] = malloc(CBUFFER_SIZE)) == NULL)
		{
			fprintf(stdout,"Memory Error!\n");
			exit(1);
		}
	}
#endif
	for(i =0 ;i < BUFFER_SIZE_LONG; i++)
	{
		if((buff_media_picture[i] = malloc(CBUFFER_SIZE_LONG)) == NULL)
		{
			fprintf(stdout,"Memory Error!\n");
			exit(1);
		}
	}
	for(i =0 ;i < BUFFER_SIZE_LONG; i++)
	{
		if((buff_media_music[i] = malloc(CBUFFER_SIZE_LONG)) == NULL)
		{
			fprintf(stdout,"Memory Error!\n");
			exit(1);
		}
	}
	char cbuffer_line[CBUFFER_SIZE];
	char buffer_line[BUFFER_SIZE];

	int fd_picture_name;
	int fd_music_name;
	int fd_stdout_save;

	if((fd_stdout_save =  dup(STDOUT_FILENO)) < 0)
	{
		fprintf(stderr,"save for stdout failed:%s\n",strerror(errno));
		exit(1);
	}
	
	if((fd_picture_name = open("./picture_name",O_RDWR | O_TRUNC )) < 0)
	{
		fprintf(stdout,"open devices_name failed:%s\n",strerror(errno));
		exit(1);
	}

	if((fd_music_name = open("./music_name",O_RDWR | O_TRUNC )) < 0)
	{
		fprintf(stdout,"open devices_name failed:%s\n",strerror(errno));
		exit(1);
	}

	if(chdir("./mountPos") < 0)
	{
		fprintf(stderr,"Failed to get in ./mountPos:%s\n",strerror(errno));
		exit(1);
	}
	system("pwd");	


	dup2(fd_picture_name,STDOUT_FILENO);
	system("ls | grep .*.jpg");
	system("ls | grep .*.JPG");
	system("ls | grep .*.jpeg");
	dup2(fd_stdout_save,STDOUT_FILENO);

	dup2(fd_music_name,STDOUT_FILENO);
	system("ls | grep .*.mp3");
	system("ls | grep .*.MP3");
	dup2(fd_stdout_save,STDOUT_FILENO);

	ssize_t read_bytes;
	lseek(fd_picture_name,0,SEEK_SET);

	if((read_bytes = read(fd_picture_name,buffer_line,sizeof(buffer_line))) < 0)
	{
		fprintf(stderr,"read devices failed:%s\n",strerror(errno));
		exit(1);
	}
	
	buffer_line[read_bytes] = '\0';

	int j = 0;
	for(i = 0; i < read_bytes; i ++)
	{
		if(buffer_line[i] != '\n')
		{
			cbuffer_line[j++] = buffer_line[i];
		}
		else
		{
			cbuffer_line[j] = '\0';
			memcpy(buff_media_picture[counter_p],cbuffer_line,strlen(cbuffer_line)+1);
			//buff_media_picture[*counter_p][j] = '\0';
			counter_p++;
			memset(cbuffer_line,0,sizeof(cbuffer_line));
	  		j = 0;
		}
	}

	lseek(fd_music_name,0,SEEK_SET);

	if((read_bytes = read(fd_music_name,buffer_line,sizeof(buffer_line))) < 0)
	{
		fprintf(stderr,"read devices failed:%s\n",strerror(errno));
		exit(1);
	}
	
	buffer_line[read_bytes] = '\0';

	for(i = 0; i < read_bytes; i ++)
	{
		if(buffer_line[i] != '\n')
		{
			cbuffer_line[j++] = buffer_line[i];
		}
		else
		{
			cbuffer_line[j] = '\0';
	  		j = 0;
			memcpy(buff_media_music[counter_m],cbuffer_line,strlen(cbuffer_line)+1);
			counter_m++;
			memset(cbuffer_line,0,sizeof(cbuffer_line));
		}
	}
	my_usb_info->buff_media_picture = buff_media_picture;
	my_usb_info->buff_media_music = buff_media_music;
	//my_usb_info->device_name  = buff_new_device[counter_new -1];
	my_usb_info->counter_p = counter_p;
	my_usb_info->counter_m = counter_m;
	chdir("..");

	return 0;
}



















