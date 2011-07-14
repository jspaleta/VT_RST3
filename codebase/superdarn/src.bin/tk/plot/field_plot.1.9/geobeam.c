/* geobeam.c
   ========= */


/*
 (c) 2010 JHU/APL & Others - Please Consult LICENSE.superdarn-rst.3.2-beta-4-g32f7302.txt for more information.
 
 
 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "rmath.h"
#include "radar.h" 
#include "rpos.h"
#include "invmag.h"
#include "scandata.h"
#include "geobeam.h"
#include "aacgm.h"





int GeoLocCenter(struct RadarSite *site,int mag,float *lat,float *lon) {

  int s;
  double glat,glon,mlat,mlon,rho;
  RPosGeo(0,8,35,site,180,45,100,
            300.0,&rho,&glat,&glon);

  if (mag) { 
    s=AACGMConvert(glat,glon,300,&mlat,&mlon,&rho,0);
    *lat=mlat;
    *lon=mlon;
  } else {
    *lat=glat;
    *lon=glon;
  }
 return 0;
}

int GeoLocBeam(struct RadarSite *site,int year,
               struct GeoLocScan *geol,struct RadarBeam *bm) {
  int n,s,x;
  int rng;
  double rho,lat,lon,glat,glon,mlat,mlon;
  double geoazm,elv,magazm; 

  for (n=0;n<geol->num;n++) {
    if (geol->bm[n].bm !=bm->bm) continue;
    if (geol->bm[n].frang !=bm->frang) continue;
    if (geol->bm[n].rsep !=bm->rsep) continue;
    if (geol->bm[n].rxrise !=bm->rxrise) continue;
    if (geol->bm[n].nrang !=bm->nrang) continue;
    break;
  }
 
  if (n<geol->num) return n;
   
  /* we've got a new beam */
   
  if (geol->bm==NULL) geol->bm=malloc(sizeof(struct GeoLocBeam));
  else {
    struct GeoLocBeam *tmp=NULL;
    tmp=realloc(geol->bm,(geol->num+1)*sizeof(struct GeoLocBeam));
    if (tmp==NULL) return -1;
    geol->bm=tmp;
  }
  geol->bm[n].bm=bm->bm;
  geol->bm[n].frang=bm->frang;
  geol->bm[n].rsep=bm->rsep;
  geol->bm[n].rxrise=bm->rxrise;
  geol->bm[n].nrang=bm->nrang;




  for (x=0;x<3;x++) {
    geol->bm[n].glat[x]=malloc(sizeof(float)*(bm->nrang+1));
    geol->bm[n].glon[x]=malloc(sizeof(float)*(bm->nrang+1));
    geol->bm[n].mlat[x]=malloc(sizeof(float)*(bm->nrang+1));
    geol->bm[n].mlon[x]=malloc(sizeof(float)*(bm->nrang+1));
  }
  geol->bm[n].gazm=malloc(sizeof(float)*bm->nrang);
  geol->bm[n].mazm=malloc(sizeof(float)*bm->nrang);

  for (rng=0;rng<=bm->nrang;rng++) {
    RPosGeo(0,bm->bm,rng,site,bm->frang,bm->rsep,bm->rxrise,
               300.0,&rho,&lat,&lon);
    geol->bm[n].glat[0][rng]=lat;
    geol->bm[n].glon[0][rng]=lon;
    RPosGeo(0,bm->bm+1,rng,site,bm->frang,bm->rsep,bm->rxrise,
               300.0,&rho,&lat,&lon);
    geol->bm[n].glat[2][rng]=lat;
    geol->bm[n].glon[2][rng]=lon;
    if (rng<bm->nrang) {
      RPosGeo(1,bm->bm,rng,site,bm->frang,bm->rsep,bm->rxrise,
                 300.0,&rho,&lat,&lon);
      geol->bm[n].glat[1][rng]=lat;
      geol->bm[n].glon[1][rng]=lon;
      RPosRngBmAzmElv(bm->bm,rng,year,site,
                   bm->frang,bm->rsep,bm->rxrise,
                   300.0,&geoazm,&elv);
      geol->bm[n].gazm[rng]=geoazm;
    }
    
    glat=geol->bm[n].glat[0][rng];
    glon=geol->bm[n].glon[0][rng];
    s=AACGMConvert(glat,glon,300,&mlat,&mlon,&rho,0);
    geol->bm[n].mlat[0][rng]=mlat;
    geol->bm[n].mlon[0][rng]=mlon;
    
    glat=geol->bm[n].glat[2][rng];
    glon=geol->bm[n].glon[2][rng];
    s=AACGMConvert(glat,glon,300,&mlat,&mlon,&rho,0);
    geol->bm[n].mlat[2][rng]=mlat;
    geol->bm[n].mlon[2][rng]=mlon;
    if (rng<bm->nrang) {
      RPosInvMag(bm->bm,rng,year,site,bm->frang,bm->rsep,bm->rxrise,
              300.0,&mlat,&mlon,&magazm);
      geol->bm[n].mlat[1][rng]=mlat;
      geol->bm[n].mlon[1][rng]=mlon;
      geol->bm[n].mazm[rng]=magazm;
    }
  }
  geol->num++;

  return n;
}


  




