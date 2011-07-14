/* plot_time.c
   =========== 
   Author: R.J.Barnes
*/


/*
 (c) 2010 JHU/APL & Others - Please Consult LICENSE.superdarn-rst.3.2-beta-4-g32f7302.txt for more information.
 
 
 
*/



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#include "rfbuffer.h"
#include "iplot.h"
#include "rtime.h"
#include "text_box.h"



void plot_time(struct Plot *plot,
               float xoff,float yoff,float wdt,float hgt,int flg,
               double stime,double etime,
               unsigned int color,unsigned char mask,
               char *fontname,float fontsize,
               void *txtdata) {
  int i;
  char txt[256];
  float txbox[3];
  char *month[]={"Jan","Feb","Mar","Apr","May","Jun",
               "Jul","Aug","Sep","Oct","Nov","Dec",0};
  char *tmeC="0";
  float cwdt;
  float x,y;
  int yr,mo,dy,shr,smt,ssc,ehr,emt,esc;
  double sec;

  txtbox(fontname,fontsize,strlen(tmeC),tmeC,txbox,txtdata);
  cwdt=txbox[0];

  TimeEpochToYMDHMS(etime,&yr,&mo,&dy,&ehr,&emt,&sec);
  esc=sec;
  TimeEpochToYMDHMS(stime,&yr,&mo,&dy,&shr,&smt,&sec);
  ssc=sec;

  if (flg==2) {
    sprintf(txt,"%.2d %s %d %.4d%.2d%.2d %.2d:%.2d:%.2d - %.2d:%.2d:%.2d UT",
            dy,month[mo-1],yr,yr,mo,dy,
            shr,smt,ssc,ehr,emt,esc);
    txtbox(fontname,fontsize,strlen(txt),txt,txbox,txtdata);
    x=xoff+wdt-txbox[0]*0.90;
    y=yoff;
    for (i=0;txt[i] !=0;i++) {
      txtbox(fontname,fontsize,1,txt+i,txbox,txtdata);
      PlotText(plot,NULL,fontname,fontsize,x,
              y,1,txt+i,color,mask,1);
      if (isdigit(txt[i])) x+=cwdt;
      else x+=txbox[0];
    }
    return;
  }
  
  if (flg==0) {
    sprintf(txt,"%.2d %s %d",dy,month[mo-1],yr);
    txtbox(fontname,fontsize,strlen(txt),txt,txbox,txtdata);
    x=xoff;
    y=yoff+txbox[2];
    for (i=0;txt[i] !=0;i++) {
      txtbox(fontname,fontsize,1,txt+i,txbox,txtdata);
      PlotText(plot,NULL,fontname,fontsize,x,
              y,1,txt+i,color,mask,1);
      if (isdigit(txt[i])) x+=cwdt;
      else x+=txbox[0];
    }
  }

  if (stime==etime) sprintf(txt,"%.2d:%.2d:%.2d0UT",shr,smt,ssc);
  else sprintf(txt,"%.2d:%.2d:%.2d0-0%.2d:%.2d:%.2d0UT",
               shr,smt,ssc,ehr,emt,esc);

  txtbox(fontname,fontsize,strlen(txt),txt,txbox,txtdata);

  if (stime==etime) sprintf(txt,"%.2d:%.2d:%.2d UT",shr,smt,ssc);
  else sprintf(txt,"%.2d:%.2d:%.2d - %.2d:%.2d:%.2d UT",
               shr,smt,ssc,ehr,emt,esc);

  if (txbox[0]<wdt) x=xoff+wdt-txbox[0]*0.90;  
  else x=xoff;
  y=yoff+txbox[2];
  for (i=0;txt[i] !=0;i++) {
    txtbox(fontname,fontsize,1,txt+i,txbox,txtdata);
    PlotText(plot,NULL,fontname,fontsize,x,
              y,1,txt+i,color,mask,1);
    if (isdigit(txt[i])) x+=cwdt;
    else x+=txbox[0];
  }
}
  