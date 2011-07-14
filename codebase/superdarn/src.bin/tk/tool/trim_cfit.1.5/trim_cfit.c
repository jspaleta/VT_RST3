/* trim_cfit.c
   ==========
   Author: R.J.Barnes
*/

/*
 (c) 2010 JHU/APL & Others - Please Consult LICENSE.superdarn-rst.3.2-beta-4-g32f7302.txt for more information.
 
 
 
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ctype.h>
#include <zlib.h>
#include "rtypes.h"
#include "rtime.h"
#include "option.h"
#include "cfitdata.h"
#include "cfitread.h"
#include "cfitwrite.h"

#include "errstr.h"
#include "hlpstr.h"




struct CFitdata *cfit;

int fnum=0;

int cpid=-1;
int channel=-1;

struct OptionData opt;

double strdate(char *text) {
  double tme;
  int val;
  int yr,mo,dy;
  val=atoi(text);
  dy=val % 100;
  mo=(val / 100) % 100;
  yr=(val / 10000);
  if (yr<1970) yr+=1900;  
  tme=TimeYMDHMSToEpoch(yr,mo,dy,0,0,0);

  return tme;
}

double strtime(char *text) {
  int hr,mn;
  int i;
  for (i=0;(text[i] !=':') && (text[i] !=0);i++);
  if (text[i]==0) return atoi(text)*3600L;
  text[i]=0;
  hr=atoi(text);
  mn=atoi(text+i+1);
  return (double) hr*3600L+mn*60L;
}   

int main (int argc,char *argv[]) {

  int arg;

  int status=0;

  struct CFitfp *cfitfp;
  gzFile gzfp=0;
  
  int yr,mo,dy,hr,mt;
  double sc;
 
  double atime;

  char *stmestr=NULL;
  char *etmestr=NULL;
  char *sdtestr=NULL;
  char *edtestr=NULL;
  char *exstr=NULL;

  double stime=-1;
  double etime=-1;
  double extime=0;
 
  double sdate=-1;
  double edate=-1;

  unsigned char vb=0;
  unsigned char help=0;
  unsigned char option=0;
  unsigned char set_channel=0;
  
  char *chnstr=NULL;
  char *cpstr=NULL;
  
  cfit=CFitMake();

  OptionAdd(&opt,"-help",'x',&help);
  OptionAdd(&opt,"-option",'x',&option);
  OptionAdd(&opt,"vb",'x',&vb);
  OptionAdd(&opt,"st",'t',&stmestr);
  OptionAdd(&opt,"et",'t',&etmestr);
  OptionAdd(&opt,"sd",'t',&sdtestr);
  OptionAdd(&opt,"ed",'t',&edtestr);
  OptionAdd(&opt,"ex",'t',&exstr);
  OptionAdd(&opt,"cp",'t',&cpstr);
  OptionAdd(&opt,"cn",'t',&chnstr);
  OptionAdd(&opt,"sc",'x',&set_channel);


  arg=OptionProcess(1,argc,argv,&opt,NULL);

  if (help==1) {
    OptionPrintInfo(stdout,hlpstr);
    exit(0);
  }


  if (option==1) {
    OptionDump(stdout,&opt);
    exit(0);
  }

  if  (argc-arg<1) {
    OptionPrintInfo(stderr,errstr);
    exit(1);
  }

  if (exstr !=NULL) extime=strtime(exstr);
  if (stmestr !=NULL) stime=strtime(stmestr);
  if (etmestr !=NULL) etime=strtime(etmestr);
  if (sdtestr !=NULL) sdate=strdate(sdtestr);
  if (edtestr !=NULL) edate=strdate(edtestr);

  if (cpstr !=NULL) cpid=atoi(cpstr);
   
  if (chnstr !=NULL) {
    if (tolower(chnstr[0])=='a') channel=1;
    if (tolower(chnstr[0])=='b') channel=2;
  }
    


  cfitfp=CFitOpen(argv[arg]);
  if (cfitfp==NULL) {
    fprintf(stderr,"File not found.\n");
    exit(-1);
  }

  if (CFitRead(cfitfp,cfit)==-1) {
    fprintf(stderr,"Error reading file\n");
    exit(-1);
  }
  atime=cfit->time;

  gzfp=gzdopen(fileno(stdout),"w");  

  /* skip here */

  if ((stime !=-1) || (sdate !=-1)) { 
    int yr,mo,dy,hr,mt;
    double sc;  
      
    if (stime==-1) stime= ( (int) atime % (24*3600));
    if (sdate==-1) stime+=atime - ( (int) atime % (24*3600));
    else stime+=sdate;


    TimeEpochToYMDHMS(stime,&yr,&mo,&dy,&hr,&mt,&sc);
    status=CFitSeek(cfitfp,yr,mo,dy,hr,mt,sc,NULL);
    if (status ==-1) {
      fprintf(stderr,"File does not contain the requested interval.\n");
      exit(-1);
    }
    if (CFitRead(cfitfp,cfit)==-1) {
      fprintf(stderr,"Error reading file\n");
      exit(-1);
    }
  } else stime=atime;
   
  if (etime !=-1) {
    if (edate==-1) etime+=atime - ( (int) atime % (24*3600));
    else etime+=edate;
  }
  if (extime !=0) etime=stime+extime;


  do {
    if ((etime !=-1) && (atime>=etime)) break;

    if ((cpid !=-1) && (cfit->cp !=cpid)) continue;
    if ((set_channel==0) &&
        (channel !=-1) && (cfit->channel !=channel)) continue;
 
    if (set_channel==1) cfit->channel=channel;
    
    status=CFitWrite(gzfp,cfit);
    
    atime=cfit->time;
    TimeEpochToYMDHMS(atime,&yr,&mo,&dy,&hr,&mt,&sc);
    if (vb==1) fprintf(stderr,"%d-%d-%d %d:%d:%d %.2d %.2d %.4d\n",
                            yr,mo,dy,hr,mt,
                            (int) sc,cfit->channel,
                            cfit->bmnum,cfit->cp);
 
  
  } while (CFitRead(cfitfp,cfit) !=-1);

  gzclose(gzfp);    
  CFitClose(cfitfp);
  return 0; 
} 






















