/* make_grid.c
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
#include <sys/types.h>
#include "rtypes.h"
#include "rfbuffer.h"
#include "iplot.h"
#include "polygon.h"
#include "rmap.h"



struct PolygonData *make_grid(float lonspc,float latspc,int max) {

  struct PolygonData *ptr=NULL;
  float lat,lon,l;
  float latmin,latmax;
  float pnt[2];

  ptr=PolygonMake(2*sizeof(float),NULL);
  if (ptr==NULL) return NULL;

  latmin=-90+latspc;
  latmax=90-latspc;

  if (max) latmin=-90;
  if (max) latmax=90;

  for (lat=latmin;lat<latmax;lat+=latspc) {
    for (lon=0;lon<360;lon+=lonspc) {
      PolygonAddPolygon(ptr,1);
      pnt[0]=lat;
      pnt[1]=lon;
      PolygonAdd(ptr,pnt);
      for (l=1;l<=latspc;l+=1) {
        pnt[0]=lat+l;
        PolygonAdd(ptr,pnt);
      }
      pnt[0]=lat+latspc;
      for (l=0;l<=lonspc;l+=1) {
        pnt[1]=lon+l;
        PolygonAdd(ptr,pnt);
      }
      pnt[1]=lon+lonspc;
      for (l=1;l<=latspc;l+=1) {
        pnt[0]=lat+latspc-l;
        PolygonAdd(ptr,pnt);
      }
      pnt[0]=lat;
      for (l=1;l<lonspc;l+=1) {
        pnt[1]=lon+lonspc-l;
        PolygonAdd(ptr,pnt);
      }

    }
  }
  return ptr;
}
