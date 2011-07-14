/* wrap.c
   ====== 
   Author: R.J.Barnes
*/

/*
 (c) 2010 JHU/APL & Others - Please Consult LICENSE.superdarn-rst.3.2-beta-4-g32f7302.txt for more information.
 
 
 
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h> 
#include <sys/types.h>
#include "rtypes.h"
#include "rfbuffer.h"
#include "iplot.h"
#include "polygon.h"
#include "rmath.h"
#include "rmap.h"



struct PolygonData *MapWraparound(struct PolygonData *map) {

  struct PolygonData *clip=NULL;
  struct PolygonData *ptr=NULL;
  struct PolygonData *dst=NULL;
  float *pnt;

  float dpnt[2];

  int c=0,i,j;
  int left,right;
  int top,bottom;
 
  float ox,oy,px,py; 

  clip=MapSquareClip();
  if (clip==NULL) return NULL;

  ptr=PolygonMake(2*sizeof(float),PolygonXYbbox);
  
  for (i=0;i<map->polnum;i++) {
    left=0;
    right=0;
    top=0;
    bottom=0;
 
    PolygonAddPolygon(ptr,map->type[i]);
    pnt=(float *) PolygonRead(map,i,0);
    ox=pnt[0];
    oy=pnt[1];
    dpnt[0]=ox;
    dpnt[1]=oy;
    PolygonAdd(ptr,dpnt);
    c++;
    for (j=1;j<map->num[i];j++) {
      pnt=(float *) PolygonRead(map,i,j);
      px=pnt[0];
      py=pnt[1];
     

      if ((ox-px)>0.95) {
        px+=1.0;
        left=1;
      } else if ((ox-px)<-0.95) {
        px-=1.0;
        right=1;
      }

      if ((oy-py)>0.95) {
	py+=1.0;
        top=1;
      } else if ((oy-py)<-0.95) {
        py-=1.0;
        bottom=1;
      }
      dpnt[0]=px;
      dpnt[1]=py;
      PolygonAdd(ptr,dpnt);
      ox=px;
      oy=py;
    }
    
    if (right) {
      PolygonAddPolygon(ptr,ptr->type[c-1]);
      for (j=0;j<ptr->num[c-1];j++) {
         pnt=(float *) PolygonRead(ptr,c-1,j);
         dpnt[0]=pnt[0]+1.0;
         dpnt[1]=pnt[1];
         PolygonAdd(ptr,dpnt);
      }   
      c++;         
    }
    
    if (left) {
      PolygonAddPolygon(ptr,ptr->type[c-1]);
      for (j=0;j<ptr->num[c-1];j++) {
         pnt=(float *) PolygonRead(ptr,c-1,j);
         dpnt[0]=pnt[0]-1.0;
         dpnt[1]=pnt[1];
         PolygonAdd(ptr,dpnt);
      }       
      c++;
    }
    if (top) {
      PolygonAddPolygon(ptr,ptr->type[c-1]);
      for (j=0;j<ptr->num[c-1];j++) {
         pnt=(float *) PolygonRead(ptr,c-1,j);
         dpnt[0]=pnt[0];
         dpnt[1]=pnt[1]+1.0;
         PolygonAdd(ptr,dpnt);
      }       
      c++;
    }
    if (bottom) {
      PolygonAddPolygon(ptr,ptr->type[c-1]);
      for (j=0;j<ptr->num[c-1];j++) {
         pnt=(float *) PolygonRead(ptr,c-1,j);
         dpnt[0]=pnt[0];
         dpnt[1]=pnt[1]-1.0;
         PolygonAdd(ptr,dpnt);
      }       
      c++;
    }
    
  }      
  dst=PolygonClip(clip,ptr);  
  PolygonFree(clip);
  PolygonFree(ptr);
  return dst;
}
