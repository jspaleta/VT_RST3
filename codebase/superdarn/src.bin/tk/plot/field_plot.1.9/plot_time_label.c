/* plot_time_label.c
   ================= 
   Author: R.J.Barnes
*/


/*
 (c) 2010 JHU/APL & Others - Please Consult LICENSE.superdarn-rst.3.2-beta-4-g32f7302.txt for more information.
 
 
 
*/



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "rmath.h"
#include "rfbuffer.h"
#include "iplot.h"
#include "rtime.h"
#include "text_box.h"



void plot_time_label(struct Plot *plot,
	             float xoff,float yoff,float wdt,float hgt,
                     float pole,int flip,char *sfx,float tme_shft,
	             float rad,float tick,
  	             unsigned int color,unsigned char mask,
                     char *fontname,float fontsize,
                     void *txtdata) {

  char txt[256];
  float txbox[3];
  float lon;
  float x,y,xtxt,ytxt,px,py;
  int i;
  
  if (flip==1) pole=-pole;

  for (i=0;i<24;i+=6) {  
 
     if ((i==0) || (i==12)) sprintf(txt,"%.2d %s",i,sfx);
     else sprintf(txt,"%.2d",i);
   

     lon=15*i-tme_shft;
     if (pole<0) lon=-lon;

     x=sin(lon*PI/180.0);
     y=cos(lon*PI/180.0);

 
     txtbox(fontname,fontsize,strlen(txt),txt,txbox,txtdata);
     xtxt=0;
     ytxt=0; 

     if (pole>0) {
         if (i==0) ytxt=txbox[2];
         else if (i==12) ytxt=txbox[1];
         else if (i==18) xtxt=txbox[0]; 
     } else {
         if (i==0) ytxt=txbox[2];
         else if (i==6) xtxt=txbox[0]; 
         else if (i==12) ytxt=txbox[1];     
     }     

     px=(xoff+wdt/2)+x*(rad+tick+2+xtxt);
     py=(yoff+hgt/2)+y*(rad+tick+2+ytxt); 

     if ((i % 12)==0) px=px-txbox[0]/2;
     else py=py+(txbox[2]-txbox[1])/2; 

     PlotText(plot,NULL,fontname,fontsize,px,py,
               strlen(txt),txt,color,mask,1);
  }
}
  
