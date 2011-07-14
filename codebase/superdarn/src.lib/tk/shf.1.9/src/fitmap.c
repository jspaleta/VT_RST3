/* fitmap.c
   ========= 
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
#include "rmath.h"
#include "rfile.h"
#include "aacgm.h"

#include "griddata.h"
#include "cnvgrid.h"
#include "cnvmap.h"

#include "shfvector.h"
#include "fitvector.h"

#include "evallegendre.h"
#include "evalpotential.h"
#include "crdshft.h"





int CnvMapFitMap(struct CnvMapData *map,struct GridData *grd) {
  double terr=0,merr=0;
  double mlat,mlon,tmp;
  double pot_max=-1e6;
  double pot_min=1e6;
  double vx,vy;
  double rms=0;
  struct CnvMapSHFVec *data;
  double *fitvel;
  double *plm;
  double *x;
  double *ph;
  double *mag;
  double lat,lon,lat_step,lon_step,th;
  double tlimit=PI,alpha,a,b;
  double var_max,var_min;
  int num=0,i=0,j,nlat,nlon,m,L,k,n,Lmax;
  int max_sub=0,min_sub=10000;
  int dnum;
  double asum=0;

  double vel_max=2000;
  double verr_min=100; /* used to be 50 for original map_potential */
  int ewt=1,mwt=1;

  fitvel=malloc(sizeof(struct CnvMapSHFVec)*(grd->vcnum+2*map->num_model));
  if (fitvel==NULL) return -1;
  data=malloc(sizeof(struct CnvMapSHFVec)*(grd->vcnum+2*map->num_model));
  if (data==NULL) return -1;

  if (map->coef !=NULL) free(map->coef);
  map->num_coef=CnvMapIndexLegendre(map->fit_order,map->fit_order)+2;
  map->coef=malloc(4*map->num_coef*sizeof(double));
     
  
  for (j=0;j<grd->vcnum;j++) {
    if (grd->data[j].st_id==-1) continue;
    mlat=fabs(grd->data[j].mlat);
    mlon=grd->data[j].mlon;
    tmp=grd->data[j].azm;
       
    if ((map->lat_shft !=0) || (map->lon_shft !=0)) 
    CnvMapCrdShft(&mlat,&mlon,&tmp,map->lat_shft,map->lon_shft);
       
    if (mlat < fabs(map->latmin)) continue;
    if (fabs(grd->data[j].vel.median)>vel_max) continue;
    data[num].lat=mlat;
    data[num].lon=mlon;
    data[num].vlos=grd->data[j].vel.median;
    data[num].verr=grd->data[j].vel.sd;
    if (data[num].verr<=verr_min) data[num].verr=verr_min;
    data[num].cos=-cos(tmp*PI/180)*map->hemisphere;
    data[num].sin=sin(tmp*PI/180);
       
    terr+=1/(data[num].verr*data[num].verr);
    num++;
  }   
  dnum=num; 
  if (num !=0) merr=sqrt(num/terr);
  else merr=verr_min;
  
  /* if the ewt==0 then substitute the averaged error */
  
  if (ewt==0) for (i=0;i<num;i++) data[i].verr=merr;

  /* if mwt==1 then adjust the error according to order */
  
  if (mwt==1) merr=sqrt( (map->fit_order/4.0)*(map->fit_order/4.0) )*merr;

  for (i=0;i<map->num_model;i++) {
    if (map->model[i].vel.median==1) continue; /* screen out boundary vecs */
    mlat=fabs(map->model[i].mlat);
    mlon=map->model[i].mlon;
    vx=map->model[i].vel.median*cos(map->model[i].azm*PI/180);
    vy=map->model[i].vel.median*sin(map->model[i].azm*PI/180);
  
    data[num].lat=mlat;
    data[num].lon=mlon;
    data[num].vlos=vx*map->hemisphere;
    data[num].verr=merr;
    data[num].cos=-1;
    data[num].sin=0;

   
    num++;
  }

  for (i=0;i<map->num_model;i++) {
    if (map->model[i].vel.median==1) continue; /* screen out boundary vecs */
    mlat=fabs(map->model[i].mlat);
    mlon=map->model[i].mlon;
    vx=map->model[i].vel.median*cos(map->model[i].azm*PI/180);
    vy=map->model[i].vel.median*sin(map->model[i].azm*PI/180);

    data[num].lat=mlat;
    data[num].lon=mlon;
    data[num].vlos=vy;
    data[num].verr=merr;
    data[num].cos=0;
    data[num].sin=1;

 

    num++;
  }

  /* okay now do boundary vecs */

  for (i=0;i<map->num_model;i++) {
    if (map->model[i].vel.median !=1) continue;
    mlat=fabs(map->model[i].mlat);
    mlon=map->model[i].mlon;
    tmp=map->model[i].azm;
     
    data[num].lat=mlat;
    data[num].lon=mlon;
    data[num].vlos=map->model[i].vel.median;
    data[num].verr=2*verr_min;
    data[num].cos=-cos(tmp*PI/180);
    data[num].sin=sin(tmp*PI/180);
   
    num++;
  }

  
   

  map->chi_sqr=CnvMapFitVector(num,data,map->coef,fitvel,map->fit_order,
			  fabs(map->latmin));

 

  /* calculate chi_sqr associated with the data values */

  for(i=0;i<dnum;i++) {
    asum+=((fitvel[i]-data[i].vlos)/data[i].verr)*
          ((fitvel[i]-data[i].vlos)/data[i].verr);
  }
  map->chi_sqr_dat=asum;
  
  rms=0;
  for (i=0;i<dnum;i++) {
     rms+=(fitvel[i]-data[i].vlos)*(fitvel[i]-data[i].vlos);
  }
  if (dnum > 0)
    map->rms_err=sqrt(rms/dnum);
  else map->rms_err=1e32;

  /* having done the fit we now need to calculate potential 
     limits and errors */

  lat_step=2.0;
  lon_step=5.0;
  n=0;
  

  Lmax=map->fit_order;  
  nlat=(int) (90.0+fabs(map->latmin))/lat_step;
  nlon=(int) (360.0/lon_step);
  alpha=tlimit/((90.0+fabs(map->latmin))/180.0*PI);  
   
  x=malloc(sizeof(double)*nlat*nlon);
  ph=malloc(sizeof(double)*nlat*nlon);
  mag=malloc(sizeof(double)*nlat*nlon); 

  if ((x==NULL) || (ph==NULL) || (mag==NULL)) return -1;
  for (j=0;j<nlon;j++) {
    for (i=0;i<nlat;i++) { 
       if (map->latmin>0) lat=i*lat_step+map->latmin;
       else lat=-i*lat_step+map->latmin;
       lat=fabs(lat);
       lon=j*lon_step;
       th=alpha*(90.0+lat)/180.0*PI;
       ph[n]=lon/180.0*PI;
       x[n]=cos(th);
       n++;
    }
  }
  plm=malloc(sizeof(double)*(Lmax+1)*(Lmax+1)*n);

  CnvMapEvalLegendre(Lmax,x,n,plm);
  CnvMapEvalPotential(Lmax,n,map->coef,plm,ph,mag);
 
  for (i=0;i<n;i++) {
    if (mag[i]>pot_max) {
      pot_max=mag[i];
      max_sub=i;
    }
    if (mag[i]<pot_min) {
      pot_min=mag[i];
      min_sub=i;
    }
  }

  /* now find the errors */
  var_max=0;
  var_min=0;
  for (m=0;m<=Lmax;m++) {
    for(L=m;L<=Lmax;L++) {
      k=CnvMapIndexLegendre(L,m); 
      if (m==0) {
        a=map->coef[k*4+3];
        var_max+=(a*PLM(L,0,max_sub))*(a*PLM(L,0,max_sub));
        var_min+=(a*PLM(L,0,min_sub))*(a*PLM(L,0,min_sub));
      } else {
        a=map->coef[k*4+3];
        b=map->coef[(k+1)*4+3];
        var_max+=(a*PLM(L,m,max_sub)*cos(m*ph[max_sub]))*
		 (a*PLM(L,m,max_sub)*cos(m*ph[max_sub]))+
		 (b*PLM(L,m,max_sub)*sin(m*ph[max_sub]))*
		 (b*PLM(L,m,max_sub)*sin(m*ph[max_sub]));
        var_min+=(a*PLM(L,m,min_sub)*cos(m*ph[min_sub]))*
 		 (a*PLM(L,m,min_sub)*cos(m*ph[min_sub]))+
		 (b*PLM(L,m,min_sub)*sin(m*ph[min_sub]))*
		 (b*PLM(L,m,min_sub)*sin(m*ph[min_sub]));
      }
    }
  }
  map->pot_min=pot_min;
  map->pot_min_err=sqrt(var_min);
  map->pot_max=pot_max;
  map->pot_max_err=sqrt(var_max);

  map->pot_drop=pot_max-pot_min;
  map->pot_drop_err=sqrt(var_max+var_min);

  free(mag);
  free(plm);
  free(x);
  free(ph);
  free(fitvel);
  free(data);
  return 0;
}















