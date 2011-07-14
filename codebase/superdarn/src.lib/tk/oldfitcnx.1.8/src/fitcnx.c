/* fitcnx.c
   =========
   Author: R.J.Barnes
*/

/*
 (c) 2010 JHU/APL & Others - Please Consult LICENSE.superdarn-rst.3.2-beta-4-g32f7302.txt for more information.
 
 
 
*/


#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <zlib.h>
#include "rtypes.h"
#include "dmap.h"
#include "rprm.h"
#include "fitdata.h"
#include "connex.h"
#include "decodemsg.h"



int OldFitCnxRead(int num,int *sock,struct RadarParm *prm,
		  struct FitData *fit,int *flag,
		  struct timeval *tmout) {
  fd_set read_set;
  unsigned char *buffer=NULL;
  int status=0,size=0,i=0;
  
  FD_ZERO(&read_set);

  for (i=0;i<num;i++) {
    if (sock[i] !=-1) FD_SET(sock[i],&read_set);
    flag[i]=0;
  }

  if ((status=select(FD_SETSIZE,&read_set,0,0,tmout)) < 0) { 
     perror("while waiting client/master I/O");
     return -1;
  }

  if (status==0) return 0;

  for (i=0;i<num;i++) {
    if ((sock[i] !=-1) && (FD_ISSET(sock[i],&read_set))) {
      size=ConnexReadIP(sock[i],&buffer);
      if (size==-1) return flag[i]=-1;
      if (buffer !=NULL) {
        flag[i]=OldFitCnxDecodeIPMsg(&prm[i],&fit[i],buffer);
        free(buffer);
      }
    } 
  }    
  return status;
}
   

 












