#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XShm.h>

static int xres,yres;

unsigned char *tcono;
unsigned char *_tcono; // alias used for the converter

extern void _blit_tc_16bpp_fast(char * dest,int skip,int x,int y);
extern void _blit_tc_fastcopy(void * dest,void * src,int count);

static XImage * image;
static XShmSegmentInfo shminfo;
static Display *dp;
static Window win;
static GC gc;
static int depth;
static char * windowcaption="demo";

static void signalh (int signum)
{
	if (signum == SIGINT) {
		kill_tc();
		exit(1);
	}
}

int blit_tc (void)
{
	int i,idx1,idx2;
	unsigned char * p24bpp;
	XEvent xevent;

	if (depth == 32)
	{
		p24bpp=(unsigned char *)image->data;
		_blit_tc_fastcopy(p24bpp,tcono,xres*yres*4);
		XShmPutImage(dp, win, gc, image, 0, 0, 0, 0, xres,yres,False);
	}
	if (depth == 24)
	{

		p24bpp=(unsigned char *)image->data;
		i=xres*yres;
		idx1=0;
		idx2=0;
		while(i--)
		{
			p24bpp[(idx1)+0]=tcono[(idx2)+0]; // B
			p24bpp[(idx1)+1]=tcono[(idx2)+1]; // G
			p24bpp[(idx1)+2]=tcono[(idx2)+2]; // R
			idx1+=3; idx2+=4;
		}
		XShmPutImage(dp, win, gc, image, 0, 0, 0, 0, xres, yres,False);

	}

	if (depth == 16) {
		// my own assembler converter from the windowsversion. _tcono = used label in asm code
                _tcono=tcono;
		_blit_tc_16bpp_fast((char *)image->data,xres*2,xres,yres);
		XShmPutImage(dp, win, gc, image, 0, 0, 0, 0, xres, yres, False);
	}
	if (depth == 8) {

	}
}

int kill_tc (void)
{
	struct shmid_ds buf;	

	XShmDetach(dp, &shminfo);
	XDestroyImage(image);
	shmctl(shminfo.shmid, IPC_RMID, &buf);
	shmdt(shminfo.shmaddr);
	XDestroyWindow(dp, win);
	XCloseDisplay(dp);

//	if (depth == 16 || depth == 8 || depth == 24)
		free(tcono);
}

int kbhit_tc (void)
{
	XEvent event;

	if(XCheckWindowEvent(dp, win, KeyPressMask, &event) == False)
		return 0;

	XPutBackEvent(dp, &event);
	return 1;

}

int getch_tc (void)
{
	XEvent event;
	XKeyEvent *xkeyevent;
	char string[16];
	KeySym key;
	XComposeStatus status;

	do {	
		XWindowEvent(dp, win, KeyPressMask, &event);
		xkeyevent = (XKeyEvent *)&event;
		XLookupString(xkeyevent, string, sizeof(string), &key, &status); 

	} while(string[0] == '\0');

	return string[0];
}

void init_tc (int x,int y)
{
	Screen *screen;
	Window parent;
	int ignore, major, minor;
	Bool pixmaps;
	XGCValues gcvalues;
	
	int counter;
	XPixmapFormatValues * xpfv;
	int xpfvcount;

	xres=x; yres=y;

	if((dp=XOpenDisplay(""))<0)
		{
			printf("Couldn't open display\n");
			exit(-1);
		}

	if (XQueryExtension(dp, "MIT-SHM", &ignore, &ignore, &ignore)) {
		if (XShmQueryVersion(dp, &major, &minor, &pixmaps) == False) {
			printf("Install MIT-SHM\n");
		}
	}
	else {
		printf("Install MIT-SHM\n");
		exit(-1);
	}

	parent=DefaultRootWindow(dp);
	win=XCreateSimpleWindow(dp, parent, 0, 0, xres, yres, 0, BlackPixel(dp, DefaultScreen(dp)), BlackPixel(dp, DefaultScreen(dp)));
	XSelectInput(dp, win, KeyPressMask);
	XMapWindow(dp, win);
	XStoreName(dp, win, windowcaption);
	XFlush(dp);
	depth = DefaultDepth(dp, DefaultScreen(dp));

// comments...	
/*
	printf("defaultdepth of defaultscreen:%i\n",DefaultDepth(dp,DefaultScreen(dp)));
	printf("defaultvisual of defaultscreen:%i\n",DefaultVisual(dp,DefaultScreen(dp)));
*/
//	printf("defaultcells of defaultscreen:%i\n",DefaultCells(dp,DefaultScreen(dp)));
//	printf("defaultplanes of defaultscreen:%i\n",DefaultPlanes(dp,DefaultScreen(dp)));
//	printf("defaultdepth of screen:%i",DefaultDepthOfScreen(ScreenOfDisplay(dp,DefaultScreen(dp))) );

// end commetns
	xpfv=XListPixmapFormats(dp,&xpfvcount);
//	printf("\n%i formats listed\n",xpfvcount);

	if (depth == 24 && xpfv!=NULL)
	{
		for (counter=0;counter<xpfvcount;counter++)
		{
/*
			printf("pixmap format:%i depth:%i bpp:%i\n"
			,counter
			,xpfv[counter].depth
			,xpfv[counter].bits_per_pixel);
*/
			if (xpfv[counter].depth==24 && xpfv[counter].bits_per_pixel==32)
			{

//				printf("upgrading from 24 to 32bpp output\n");

				depth=32;
				break;
			}
		}
	}

	gcvalues.foreground = 0xffffff;
	gcvalues.background = 0x000000;

	gc = XCreateGC(dp, (Drawable)win, 0, &gcvalues);

	image = XShmCreateImage (dp, DefaultVisualOfScreen(ScreenOfDisplay(dp,DefaultScreen(dp))),
				DefaultDepthOfScreen(ScreenOfDisplay(dp,DefaultScreen(dp))),
				ZPixmap, NULL, &shminfo,
				xres, yres);

	if (image == NULL) 
		exit(-1);

	shminfo.shmid = shmget(IPC_PRIVATE, image->bytes_per_line*image->height,
				IPC_CREAT | 0777);

	shminfo.shmaddr = image->data = (char *)shmat(shminfo.shmid, 0, 0);
	if (shminfo.shmaddr == NULL) {
		printf("bleah\n");
		exit(-1);
	}

//	if (depth == 32)
//		tcono = image->data;
//	if (depth == 16 || depth == 8 || depth == 24 )
		tcono = (char *)malloc(xres*yres*4);

	shminfo.readOnly = False;
	XShmAttach(dp, &shminfo);
}

void start_tc(void)
 {
 }

void usewindow_tc(int a)
 {
 }

void settopic_tc(char * topic)
 {
   windowcaption=topic;
 }
