/* make_pgrid.c
   ============ 
   Author: R.J.Barnes
*/

/*
 (c) 2010 JHU/APL & Others - Please Consult LICENSE.superdarn-rst.3.2-beta-4-g32f7302.txt for more information.
 
 
 
*/




#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "cnvgrid.h"



int make_pgrid(double latmin,struct CnvGrid *ptr) {
  
  int i,j;

  double lat_step;
  double lon_step;
 
  int nlat,nlon;
  int num;
  int poly; 

  ptr->type=0;
  ptr->num=0;
 
  lat_step=1.0;
  lon_step=2.0;
  num=0;  
  poly=0;

  if (latmin>0)  nlat=(int) (90.0-latmin)/lat_step;
  else  nlat=(int) (90.0+latmin)/lat_step;

  nlon=(int) (360.0/lon_step); 

  if (ptr->vertex !=NULL) free(ptr->vertex);
  if (ptr->lat !=NULL) free(ptr->lat);
  if (ptr->lon !=NULL) free(ptr->lon);
  if (ptr->mag !=NULL) free(ptr->mag);

  ptr->vertex=malloc(sizeof(int)*nlat*nlon*4);
  ptr->lat=malloc(sizeof(double)*nlat*nlon);
  ptr->lon=malloc(sizeof(double)*nlat*nlon);
  ptr->mag=malloc(sizeof(double)*nlat*nlon);
  ptr->ex=malloc(sizeof(double)*nlat*nlon);
  ptr->ey=malloc(sizeof(double)*nlat*nlon);

  if ((ptr->lat==NULL) || (ptr->lon==NULL) || 
      (ptr->mag==NULL) || (ptr->vertex==NULL) ||
      (ptr->ex==NULL) ||  (ptr->ey==NULL)) return -1;

 if (latmin>0) {
    for (i=0;i<nlat;i++) { 
      for (j=0;j<nlon;j++) {
         if (i != nlat-1) {
           ptr->vertex[4*poly]=num;
           if (j !=nlon-1) ptr->vertex[4*poly+1]=num+1;
           else ptr->vertex[4*num+1]=num-nlon+1;
           ptr->vertex[4*poly+2]=ptr->vertex[4*poly+1]+nlon;
           ptr->vertex[4*poly+3]=num+nlon;     
           poly++;  
         } 
         ptr->lat[num]=i*lat_step+latmin;
         ptr->lon[num]=j*lon_step;
         ptr->mag[num]=0;
         num++;
      }
    }
 } else {
   for (i=0;i<nlat;i++) { 
      for (j=0;j<nlon;j++) {
         if (i != nlat-1) {
           ptr->vertex[4*poly]=num;
           if (j !=nlon-1) ptr->vertex[4*poly+1]=num+1;
           else ptr->vertex[4*num+1]=num-nlon+1;
           ptr->vertex[4*poly+2]=ptr->vertex[4*poly+1]+nlon;
           ptr->vertex[4*poly+3]=num+nlon;     
           poly++;  
         } 
         ptr->lat[num]=-i*lat_step+latmin;
         ptr->lon[num]=j*lon_step;
         ptr->mag[num]=0;
         num++;
      }
    }
 }

  ptr->num=num;
  ptr->nlat=nlat;
  ptr->nlon=nlon;
  ptr->poly=poly;
  return 0;
}
 





