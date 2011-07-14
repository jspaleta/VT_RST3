/* aacgm.c
   =======
   Author: R.J.Barnes
*/


/* 
 (c) 2010 JHU/APL & Others - Please Consult LICENSE.superdarn-rst.3.2-beta-4-g32f7302.txt for more information.
 
 
 
*/




#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "default.h"
#include "convert_geo_coord.h"


extern struct {
    double coef[121][3][5][2];
} sph_harm_model;

int AACGMLoadCoefFP(FILE *fp) {
  char tmp[64];
  int f,l,a,t,i;
  if (fp==NULL) return -1;
  for (f=0;f<2;f++) { 
    for (l=0;l<5;l++) {
      for (a=0;a<3;a++) { 
        for (t=0;t<121;t++) {
	  if (fscanf(fp,"%s",tmp) !=1) {
             fclose(fp);
             return -1;
	  }
          for (i=0;(tmp[i] !=0) && (tmp[i] !='D');i++);
          if (tmp[i]=='D') tmp[i]='e';
          sph_harm_model.coef[t][a][l][f]=atof(tmp);
	}
      }
    }
  }
  return 0;
}



int AACGMLoadCoef(char *fname) {
  FILE *fp;
  fp=fopen(fname,"r");
  if (fp==NULL) return -1;
  AACGMLoadCoefFP(fp);
  fclose(fp);
  return 0;
}

int AACGMInit(int year) {
  char fname[256];
  char yrstr[32];  
  if (year==0) year=DEFAULT_YEAR;
  sprintf(yrstr,"%4.4d",year);  
  strcpy(fname,getenv("AACGM_DAT_PREFIX"));  
  if (strlen(fname)==0) return -1;
  strcat(fname,yrstr);
  strcat(fname,".asc");
  return AACGMLoadCoef(fname);
}

int AACGMConvert(double in_lat,double in_lon,double height,
              double *out_lat,double *out_lon,double *r,
              int flag) {

   int err;
   err=convert_geo_coord(in_lat,in_lon,height,
		     out_lat,out_lon,flag,10);
   *r=1.0;
   if (err !=0) return -1;
   return 0;
}

     



