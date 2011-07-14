/* cfitread.c
   =========
   Author: R.J.Barnes
*/

/*
 (c) 2010 JHU/APL & Others - Please Consult LICENSE.superdarn-rst.3.2-beta-4-g32f7302.txt for more information.
 
 
 
*/


#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <zlib.h>
#include "rtypes.h"
#include "rconvert.h"
#include "cfitdata.h"
#include "cfitread.h"



int CFitRead(struct CFitfp *fptr,struct CFitdata *ptr) {
  float val;
  unsigned char gsct;
  int i;
  if (fptr==NULL) return -1;
  

  if (fptr->fbuf==NULL) {
    if (ConvertReadIntZ(fptr->fp,&ptr->version.major) !=0) return -1;
    if (ConvertReadIntZ(fptr->fp,&ptr->version.minor) !=0) return -1;
    if (ConvertReadDoubleZ(fptr->fp,&ptr->time) !=0) return -1;
    if (ConvertReadShortZ(fptr->fp,&ptr->stid) !=0) return -1;
    if (ConvertReadShortZ(fptr->fp,&ptr->scan) !=0) return -1;
    if (ConvertReadShortZ(fptr->fp,&ptr->cp) !=0) return -1;
    if (ConvertReadShortZ(fptr->fp,&ptr->bmnum) !=0) return -1;
    if (ConvertReadFloatZ(fptr->fp,&ptr->bmazm) !=0) return -1;
    if (ConvertReadShortZ(fptr->fp,&ptr->channel) !=0) return -1;
    if (ConvertReadShortZ(fptr->fp,&ptr->intt.sc) !=0) return -1;
    if (ConvertReadIntZ(fptr->fp,&ptr->intt.us) !=0) return -1;
    if (ConvertReadShortZ(fptr->fp,&ptr->frang) !=0) return -1;
    if (ConvertReadShortZ(fptr->fp,&ptr->rsep) !=0) return -1;
    if (ConvertReadShortZ(fptr->fp,&ptr->rxrise) !=0) return -1;
    if (ConvertReadShortZ(fptr->fp,&ptr->tfreq) !=0) return -1;
    if (ConvertReadFloatZ(fptr->fp,&ptr->noise) !=0) return -1;
    if (ConvertReadShortZ(fptr->fp,&ptr->atten) !=0) return -1;
    if (ConvertReadShortZ(fptr->fp,&ptr->nave) !=0) return -1;
    if (ConvertReadShortZ(fptr->fp,&ptr->nrang) !=0) return -1;
    
  } else {
    if ((fptr->fptr+sizeof(double)+13*sizeof(int16)+
         3*sizeof(int32)+2*sizeof(float))>fptr->fsze) return -1;
    ConvertToInt(fptr->fbuf+fptr->fptr,&ptr->version.major);
    fptr->fptr+=sizeof(int32);
    ConvertToInt(fptr->fbuf+fptr->fptr,&ptr->version.minor);
    fptr->fptr+=sizeof(int32);
    ConvertToDouble(&fptr->fbuf[fptr->fptr],&ptr->time);
    fptr->fptr+=sizeof(double);
    ConvertToShort(fptr->fbuf+fptr->fptr,&ptr->stid);
    fptr->fptr+=sizeof(int16);
    ConvertToShort(fptr->fbuf+fptr->fptr,&ptr->scan);
    fptr->fptr+=sizeof(int16);
    ConvertToShort(fptr->fbuf+fptr->fptr,&ptr->cp);
    fptr->fptr+=sizeof(int16);
    ConvertToShort(fptr->fbuf+fptr->fptr,&ptr->bmnum);
    fptr->fptr+=sizeof(int16);
    ConvertToFloat(fptr->fbuf+fptr->fptr,&ptr->bmazm);
    fptr->fptr+=sizeof(float);
    ConvertToShort(fptr->fbuf+fptr->fptr,&ptr->channel);
    fptr->fptr+=sizeof(int16);
    ConvertToShort(fptr->fbuf+fptr->fptr,&ptr->intt.sc);
    fptr->fptr+=sizeof(int16);
    ConvertToInt(fptr->fbuf+fptr->fptr,&ptr->intt.us);
    fptr->fptr+=sizeof(int32);
    ConvertToShort(fptr->fbuf+fptr->fptr,&ptr->frang);
    fptr->fptr+=sizeof(int16);
    ConvertToShort(fptr->fbuf+fptr->fptr,&ptr->rsep);
    fptr->fptr+=sizeof(int16);
    ConvertToShort(fptr->fbuf+fptr->fptr,&ptr->rxrise);
    fptr->fptr+=sizeof(int16);
    ConvertToShort(fptr->fbuf+fptr->fptr,&ptr->tfreq);
    fptr->fptr+=sizeof(int16);
    ConvertToFloat(fptr->fbuf+fptr->fptr,&ptr->noise);
    fptr->fptr+=sizeof(float);
    ConvertToShort(fptr->fbuf+fptr->fptr,&ptr->atten);
    fptr->fptr+=sizeof(int16);
    ConvertToShort(fptr->fbuf+fptr->fptr,&ptr->nave);
    fptr->fptr+=sizeof(int16);
    ConvertToShort(fptr->fbuf+fptr->fptr,&ptr->nrang);
    fptr->fptr+=sizeof(int16);
  }
  fptr->ctime=ptr->time;

  if (fptr->fbuf==NULL) {
    if (ConvertReadShortZ(fptr->fp,&ptr->num) !=0) return -1;
  } else {
    if (fptr->fptr>=fptr->fsze) return -1;
    ConvertToShort(fptr->fbuf+fptr->fptr,&ptr->num);
    fptr->fptr+=sizeof(int16);
  }
  CFitSetRng(ptr,ptr->num);

  if (fptr->fbuf==NULL) {
    for (i=0;i<ptr->num;i++) 
      if (ConvertReadShortZ(fptr->fp,&ptr->rng[i]) !=0) return -1;
  } else {
    if ((fptr->fptr+sizeof(int16)*ptr->num)>=fptr->fsze) return -1;
    for (i=0;i<ptr->num;i++) {
      ConvertToShort(fptr->fbuf+fptr->fptr,&ptr->rng[i]);
      fptr->fptr+=sizeof(int16);
    }
  }

  for (i=0;i<ptr->num;i++) {
    if (fptr->fbuf==NULL) {
      if (gzread(fptr->fp,&gsct,1) !=1) return -1;
      ptr->data[i].gsct=gsct;
      if (ConvertReadFloatZ(fptr->fp,&val) !=0) return -1;
      ptr->data[i].p_0=val;  
      if (ConvertReadFloatZ(fptr->fp,&val) !=0) return -1;
      ptr->data[i].p_0_e=val;      
      if (ConvertReadFloatZ(fptr->fp,&val) !=0) return -1;
      ptr->data[i].v=val;
      if (ConvertReadFloatZ(fptr->fp,&val) !=0) return -1;
      ptr->data[i].p_l=val;
      if (ConvertReadFloatZ(fptr->fp,&val) !=0) return -1;
      ptr->data[i].w_l=val;
      if (ConvertReadFloatZ(fptr->fp,&val) !=0) return -1;
      ptr->data[i].v_e=val;
      if (ConvertReadFloatZ(fptr->fp,&val) !=0) return -1;
      ptr->data[i].p_l_e=val;
      if (ConvertReadFloatZ(fptr->fp,&val) !=0) return -1;
      ptr->data[i].w_l_e=val;
    } else {
      if ((fptr->fptr+1+sizeof(float)*6)>=fptr->fsze) return -1;
      ptr->data[i].gsct=fptr->fbuf[fptr->fptr];
      fptr->fptr+=1;
      ConvertToFloat(fptr->fbuf+fptr->fptr,&val);
      fptr->fptr+=sizeof(float);
      ptr->data[i].p_0=val;
      ConvertToFloat(fptr->fbuf+fptr->fptr,&val);
      fptr->fptr+=sizeof(float);
      ptr->data[i].p_0_e=val;
      ConvertToFloat(fptr->fbuf+fptr->fptr,&val);
      fptr->fptr+=sizeof(float);
      ptr->data[i].v=val;
      ConvertToFloat(fptr->fbuf+fptr->fptr,&val);
      fptr->fptr+=sizeof(float);
      ptr->data[i].p_l=val;
      ConvertToFloat(fptr->fbuf+fptr->fptr,&val);
      fptr->fptr+=sizeof(float);
      ptr->data[i].w_l=val;
      ConvertToFloat(fptr->fbuf+fptr->fptr,&val);
      fptr->fptr+=sizeof(float);
      ptr->data[i].v_e=val;
      ConvertToFloat(fptr->fbuf+fptr->fptr,&val);
      fptr->fptr+=sizeof(float);
      ptr->data[i].p_l_e=val;
      ConvertToFloat(fptr->fbuf+fptr->fptr,&val);
      fptr->fptr+=sizeof(float);
      ptr->data[i].w_l_e=val;
    }
  }
  
  return 0;
}









