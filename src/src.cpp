#include <iostream>
#include <cv.h>
#include <highgui.h>
#include <stdio.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <opencv2/imgproc/imgproc_c.h>
#include<stdlib.h>


using namespace std;

CvHaarClassifierCascade *cascade_f;
CvHaarClassifierCascade *cascade_e;
CvMemStorage *storage;
IplImage * glasses = cvLoadImage("fire.jpg");
IplImage *resized;
bool initial=true;
int xCentre=0;
int xInitial;
int yInitial;
int xPrev;
int yPrev;
int nodFlag=0;
bool nod;
int yCentre=0;
int flag=0;
IplImage *im_gray;
IplImage* im_bw;
int playing = 300;
char buf[100];
char buf1[100];
int flagmon=0;
int xcord,ycord;
void detectEyes(IplImage *img);
void drawImage(IplImage* target, IplImage* source, int x, int y);
int xFinal,yFinal;


void detectNod(int x, int y,IplImage *img){
	CvPoint pt1 = cvPoint(200,20);
	CvScalar blue = CV_RGB(0,0,250);
	CvScalar red = CV_RGB(255,0,0);
	CvScalar green = CV_RGB(0,255,0);
	double hscale = 1.0;
	double vscale = 0.8;
	double shear = 0.2;
	int thickness = 5;
	int line_type = 8;
	CvFont font1;
	cvInitFont(&font1,CV_FONT_HERSHEY_DUPLEX,hscale,vscale,shear,thickness,line_type);
  nod=false;
  if(y<yPrev && (yPrev-y>2) && nodFlag==0 ){

    printf("%s","trying first");
    printf("%d",yPrev-y);
//    cvPutText(img,"UP",pt1,&font1,blue);
//    system("./up.sh");
    nodFlag=1;
  }
  else if(y>yPrev && (y-yPrev>2) && nodFlag==1 ){
    //upmotion
    printf("%s","trying second");
    printf("%d",y-yPrev);
//    cvPutText(img,"DOWN",pt1,&font1,blue);
//    system("./down.sh");
    nodFlag=0;
    nod = true;
  }
  xPrev=x;
  yPrev=y;
}
void findFaceCentre(CvRect* rect,IplImage *img){
  if(initial){
    printf("%s","in initial");
    xInitial = (rect->x + rect->width)/2;
    yInitial = (rect->y + rect->height)/2;
    xPrev = xInitial;
    yPrev = yInitial;
    initial=false;
  }
  xCentre = (rect->x + rect->width)/2;
  yCentre = (rect->y + rect->height)/2;
  detectNod(xCentre,yCentre,img);
  printf("Centre is  %d %d:\n",xCentre,yCentre);
  Display *dpy;
  dpy = XOpenDisplay(0);
  Screen *screen;
  screen = DefaultScreenOfDisplay(dpy);
  int scaleFactorX=(screen->width)/(40);
  int scaleFactorY=(screen->height)/(40);
  int xCentreScreen= (screen->width)/2;
  int yCentreScreen= (screen->height)/2;
  int dx=xInitial-xCentre;
  int dy=yInitial-yCentre;

  //Scale final coordinates
  dx = scaleFactorX*(dx);
  dy = scaleFactorY*(dy);
  Window root_window;
   xFinal = xCentreScreen+dx;
   yFinal = screen->height-(yCentreScreen+dy);

  root_window = XRootWindow(dpy, 0);
  XSelectInput(dpy, root_window, KeyReleaseMask);
  printf("Final is %d %d:\n",xFinal,yFinal);
  printf("rectangle stuff is %d %d:\n",rect->width,rect->height);
  printf("Screen stuff is %d %d:\n",screen->width,rect->height);
  sprintf(buf, "./src/mousemove.sh %d %d",xFinal,yFinal);
  system(buf);
  XFlush(dpy);
  XCloseDisplay(dpy);
}



int main( int argc, char** argv )
{
   IplImage *img;
   char *file1 = "haarcascade_frontalface_alt_tree.xml";
   char *file2 = "haarcascade_eye.xml";
   CvCapture *capture;
   int key = 1;

   //open default camera device
   capture = cvCaptureFromCAM(0);
   cascade_f = (CvHaarClassifierCascade*)cvLoad(file1, 0, 0, 0);
   cascade_e = (CvHaarClassifierCascade*)cvLoad(file2, 0, 0, 0);
//   	cvSetCaptureProperty( capture, CV_CAP_PROP_FRAME_WIDTH, 1024 );
//      cvSetCaptureProperty( capture, CV_CAP_PROP_FRAME_HEIGHT, 1024 );

   storage = cvCreateMemStorage(0);

   assert(cascade_f && cascade_e && storage);

   cvNamedWindow("w",CV_WINDOW_AUTOSIZE);
   while( key != 'q' ) {
     //capture frame from video stream

     img = cvQueryFrame( capture );
     if( !img ) break;

     //pass current frame to detect face and eyes
     detectEyes(img);
     cvShowImage("w", img);

     key = cvWaitKey(1);
   }

   cvWaitKey(0);
   cvDestroyWindow("w");
   cvReleaseImage(&img);
   return 0;
}


void drawImage(IplImage* target, IplImage* source, int x, int y) {
   for (int ix=0; ix<source->width; ix++) {
       for (int iy=0; iy<source->height; iy++) {
           int r = cvGet2D(source, iy, ix).val[2];
           int g = cvGet2D(source, iy, ix).val[1];
           int b = cvGet2D(source, iy, ix).val[0];
   if(r!= 0 || g!= 0 || b != 0){
     CvScalar bgr = cvScalar(b, g, r);
     cvSet2D(target, iy+y, ix+x, bgr);
   }
       }
   }
}

void detectEyes(IplImage *img)
{
int i;
CvPoint pt1 = cvPoint(200,20);
CvScalar blue = CV_RGB(0,0,250);
CvScalar red = CV_RGB(255,0,0);
CvScalar green = CV_RGB(0,255,0);
double hscale = 1.0;
double vscale = 0.8;
double shear = 0.2;
int thickness = 5;
int line_type = 8;
int flagnoeye=0;
int flagplay=0;
int key1;

CvFont font1;
cvInitFont(&font1,CV_FONT_HERSHEY_DUPLEX,hscale,vscale,shear,thickness,line_type);
//returns an array of rectangles for faces

CvSeq *faces = cvHaarDetectObjects(img, cascade_f, storage,1.1, 3, 0, cvSize(24,24));
if (faces->total == 0) return;
CvRect *r = (CvRect*)cvGetSeqElem(faces, 0);
CvRect *g = (CvRect*)cvGetSeqElem(faces, 0);
CvRect *e = (CvRect*)cvGetSeqElem(faces, 0);
findFaceCentre(e,img);

//Highlight face rectangle with red color
cvRectangle(img,
   cvPoint(e->x, e->y),
   cvPoint(e->x + e->width, e->y + e->height),
   CV_RGB(255, 0, 0), 1, 8, 0);

//clear temporary memory storage
cvClearMemStorage(storage);

cvSetImageROI(img, cvRect(e->x, e->y + (e->height/5.5), e->width, e->height/3.0));
CvSeq* eyes = cvHaarDetectObjects(img,cascade_e,storage,1.1, 3, 0, cvSize(25,15));




 if(flagnoeye == 0)
 {
   flagmon=1;
   if (eyes->total == 0)
     {
flag++;
     }
   else
     {
flag=0;
     }
 }

if(flag>7)
{
 cvPutText(img,"CLICK",pt1,&font1,red);
 sprintf(buf1, "./src/mouseclick.sh %d %d",xFinal,yFinal);
  system(buf1);

 flagplay=1;

}
else if (flag==1)
{
// cvPutText(img,"BLINK",pt1,&font1,green);
 flagplay=0;
 playing=300;
}


double area = 0.0;
double pos1 = 0;
double pos2 = 0;

for( i = 0; i < (eyes ? eyes->total : 0); i++ )
{
 r = (CvRect*)cvGetSeqElem( eyes, i );
 if(area < r->height*r->width && (r->x-e->x >30|| ((e->x+e->width)-(r->x+r->width)) > 30))
 {
   area = r->height*r->width;
   pos1 = i;
 }
}

r = (CvRect*)cvGetSeqElem( eyes, pos1 );
area = 0.0;
for( i = 0; i < (eyes ? eyes->total : 0); i++ )
{
 g = (CvRect*)cvGetSeqElem( eyes, i );
 if(area < g->height*g->width && i != pos1 && (g->x-e->x >30 || ((e->x+e->width)-(g->x+g->width)) > 30))
 {
   if(abs(g->x-r->x)>20)
   {
area = r->height*r->width;
pos2 = i;
   }
 }
}

for( i = 0; i < (eyes ? eyes->total : 0); i++ ) {

 if(i == pos1 || i == pos2)
 {
   r = (CvRect*)cvGetSeqElem( eyes, i );
   IplImage *resized = cvCreateImage(cvSize(r->width , r->height),glasses->depth,glasses->nChannels );
   cvResize(glasses, resized);
   drawImage(img, resized, r->x, r->y);
 }

}

cvResetImageROI(img);
}

//REDUNDANT??
void detectifthereisablink(CvSeq* eyes,IplImage *img )
{
int curcount=0;
int prevcount=0;
int comparecount=0;
int flag=0;
CvPoint pt1 = cvPoint(250,20);
CvScalar blue = CV_RGB(255,255,255);
double hscale = 1.0;
double vscale = 0.8;
double shear = 0.2;
int thickness = 5;
int line_type = 8;
int flagnoeye=0;
int flagbeginmonitor=0;
int flag1=0;
CvFont font1;
cvInitFont(&font1,CV_FONT_HERSHEY_DUPLEX,hscale,vscale,shear,thickness,line_type);

curcount=cvCountNonZero(&eyes);
if(comparecount==0)
{
comparecount=1;
}
if(comparecount==1)
{
if(curcount-prevcount > 400 )
{
//cvPutText(img,"blink",pt1,&font1,blue);
flag=0;
}
else
{
flag=1;
}
if(flag==0)
{
flag1++;
}
else
{
flag1=0;
}
}
if(flag1>4)
{
cvPutText(img,"CLICK",pt1,&font1,blue);

}
}
//	cvSetCaptureProperty( capture, CV_CAP_PROP_FRAME_WIDTH, 3840 );
//    cvSetCaptureProperty( capture, CV_CAP_PROP_FRAME_HEIGHT, 2160 );
