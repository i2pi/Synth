#ifdef __cplusplus
extern "C" {
#endif

/************************************************************************/
/* general function names */

extern int WIDTH, HEIGHT;

 extern unsigned long int *tcono;

 void init_tc(int x,int y);

 void start_tc(void);

 void blit_tc(void);

 void kill_tc(void);

 void settopic_tc(char *);
 void usewindow_tc(int);

/* keyboard functions */
 int getch_tc(void);
 int kbhit_tc(void);
 int keystate_tc(int);        /* win32 only atm */

/* mouse stuff */
 int mousex_tc(void);         /* also win32 only atm */
 int mousey_tc(void);         /* also win32 only atm */
 int mouseb_tc(void);         /* also win32 only atm */

 // if you have a logo then specify the info here
 extern int logoWidth;
 extern int logoHeight;
 extern int* logoPixels;
 
 // wtcGo can be set to 1 to exit the selectionscreen
 extern int wtcGo;

 // the buttons can be specified from here, by default there is 2 buttons
 typedef void (*WTC_BUTTONPROC)();
 extern int wtcButtonCount;
 extern char *wtcButtonText[10];
 extern WTC_BUTTONPROC wtcButtonProc[10];
 extern WTC_BUTTONPROC logoUpdate;

 extern int wtcFinalWidth;
 extern int wtcFinalHeight;
 extern int wtcQuit;

/************************************************************************/
/* Pragmas sealed incase of any other compiler using aux */

#ifdef __cplusplus
};
#endif

