/*
$Log: combvec.c,v $

Revision 1.0  2009/03/03 17:05:20  aj
this code reads in a fit file and produces an ascii file

*/
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>
#include <math.h>
#include <zlib.h>
#include "rtypes.h"
#include "dmap.h"
#include "option.h"
#include "rtime.h"
#include "radar.h"
#include "rprm.h"
#include "invmag.h" 
#include "rpos.h"
#include "fitdata.h"
#include "fitread.h"
#include "scandata.h"
#include "fitscan.h"
#include "mergedata.h"

struct RadarNetwork *network;
struct Radar *radar1;
struct Radar *radar2;
struct RadarSite *site1;
struct RadarSite *site2;
struct OptionData opt;

struct combvec
{
  int matched;
  double diff;
};

double haversine(double theta)
{
  return sin(theta/2.)*sin(theta/2.);
}

/*convert year/month/day to day of year*/
int yrmondaytodoy(int y,int m,int d)
{
  int i;
  int doy = 0;
  int monarr[12] = {31,28,31,30,31,30,31,31,30,31,30,31};

  if((y % 4) == 0)
    monarr[1] = 29;


  for(i=0;i<m-1;i++)
  {
    doy += monarr[i];
  }
  doy += (d-1);
  return doy;
}

double getazm(double lat1, double lon1, double lat2, double lon2)
{
  return atan2(sin(lon2-lon1)*cos(lat2),cos(lat1)*sin(lat2)-sin(lat1)*cos(lat2)*cos(lon2-lon1));
}

void getendpoints(double lat1,double lon1,double * lat2, double * lon2, double azm)
{
  double d = 25./6378.,pi=3.14159,degrad=3.14159/180.,q,tlat2,tlon2;
  lat1 *= degrad;
  lon1 *= degrad;
  azm *= degrad;
  double dLat = d*cos(azm);
  tlat2 = lat1 + dLat;
  double dPhi = log(tan(tlat2/2.+pi/4.)/tan(lat1/2.+pi/4.));
  if(dPhi != 0.) q = dLat/dPhi;
  else q = cos(lat1);

  if(fabs(tlat2) > pi/2.)
  {
    if(tlat2 > 0) tlat2 = pi-tlat2;
    else tlat2 = -(pi-tlat2);
  }

  double dLon = d*sin(azm)/q;
  int wrap = (int)((lon1+dLon+pi)/(2.*pi));
  tlon2 = (lon1+dLon+pi)-(double)wrap*(2.*pi) - pi;

  *lon2 = tlon2/degrad;
  *lat2 = tlat2/degrad;
}

int main(int argc,char *argv[])
{
  /*declarations*/
  int arg = 0;
  double grho, lat1, lat2, lon1, lon2;
  double azm1, azm2, del_lat, del_lon, h, d, v_e, v_n, v1, v2, theta1, theta2, sc;
  int j, k, l, m, p, yr, mo, dy, hr, mn;
  double pi = 3.14159;
  int mag_flg = 0;
  FILE *fp;
  FILE *fitfp1 = NULL;
  FILE *fitfp2 = NULL;
  char *envstr;
  unsigned char help = 0;
  unsigned char option = 0;
  double azm,elv;
  int sthr = -1, edhr = 26;
  int s1 = 0, s2 = 0, vb = 0, state1 = 0, state2 = 0;

  struct RadarScan *scan1 = RadarScanMake();
  struct RadarScan *scan2 = RadarScanMake();
  struct FitData *tempfit1 = FitMake();
	struct FitData *tempfit2 = FitMake();
  struct RadarParm *tempprm1 = RadarParmMake();
	struct RadarParm *tempprm2 = RadarParmMake();


  envstr=getenv("SD_RADAR");
  if(envstr==NULL)
  {
    fprintf(stderr,"Environment variable 'SD_RADAR' must be defined.\n");
    return -1;
  }
  fp=fopen(envstr,"r");

  if(fp==NULL)
  {
    fprintf(stderr,"Could not locate radar information file.\n");
    exit(-1);
  }

  network=RadarLoad(fp);
  fclose(fp);
  if(network==NULL)
  {
    fprintf(stderr,"Failed to read radar information.\n");
    exit(-1);
  }

  envstr=getenv("SD_HDWPATH");
  if(envstr==NULL)
  {
    fprintf(stderr,"Environment variable 'SD_HDWPATH' must be defined.\n");
    exit(-1);
  }

  RadarLoadHardware(envstr,network);

  /*add options to control performance*/
  OptionAdd(&opt,"-help",'x',&help);
  OptionAdd(&opt,"-option",'x',&option);
  OptionAdd(&opt,"vb",'x',&vb);
  OptionAdd(&opt,"sthr",'i',&sthr);
  OptionAdd(&opt,"edhr",'i',&edhr);

  /*process the options*/
  arg=OptionProcess(1,argc,argv,&opt,NULL);
  if(option == 1)
  {
    OptionDump(stdout,&opt);
    exit(0);
  }


  /*read the first radar's file*/
  fitfp1=fopen(argv[arg],"r");
  fprintf(stderr,"%s\n",argv[arg]);
  if(fitfp1==NULL)
  {
    fprintf(stderr,"File %s not found.\n",argv[arg]);
    exit(-1);
  }
  /*read the first entry in the fit file to get cpid*/
  s1=FitFread(fitfp1,&tempprm1,&tempfit1);
  if(s1 == -1)
  {
    fprintf(stderr,"Error reading file %s.\n",argv[arg]);
    exit(-1);
  }
  if(tempprm1->channel != 0)
		mychn1 = 1;
	fclose(fitfp1);
	memset(tempprm1,0,sizeof(struct RadarParm));
	memset(tempfit1,0,sizeof(struct FitData));

	s1=FitFreadRadarScan(fitfp1,&state1,scan1,tempprm1,tempfit1,0,0,mychn1);




  radar1=RadarGetRadar(network,scan1->stid);
  if (radar1==NULL)
  {
    fprintf(stderr,"Failed to get radar information.\n");
    exit(-1);
  }
  TimeEpochToYMDHMS(scan1->st_time,&yr,&mo,&dy,&hr,&mn,&sc);
  site1=RadarYMDHMSGetSite(radar1,yr,mo,dy,hr,mn,sc);


  /*read the second radar's file*/
  fitfp2=fopen(argv[arg+1],"r");
  fprintf(stderr,"%s\n",argv[arg+1]);
  if(fitfp2==NULL)
  {
    fprintf(stderr,"File %s not found.\n",argv[arg]);
    exit(-1);
  }
  /*read the first entry in the fit file to get cpid*/
  s2=FitFread(fitfp2,tempprm2,tempfit2);
  if(s2 == -1)
  {
    fprintf(stderr,"Error reading file %s.\n",argv[arg+1]);
    exit(-1);
  }
  if(tempprm2->channel != 0)
		mychn2 = 1;
	fclose(fitfp2);
	memset(tempprm2,0,sizeof(struct RadarParm));
	memset(tempfit2,0,sizeof(struct FitData));

	s2=FitFreadRadarScan(fitfp2,&state2,scan2,tempprm2,tempfit2,0,0,mychn1);


  radar2=RadarGetRadar(network,scan2->stid);
  if (radar2==NULL)
  {
    fprintf(stderr,"Failed to get radar information.\n");
    exit(-1);
  }
  TimeEpochToYMDHMS(scan2->st_time,&yr,&mo,&dy,&hr,&mn,&sc);
  site2=RadarYMDHMSGetSite(radar2,yr,mo,dy,hr,mn,sc);


  /*read the second radars file until it catches up to the first radar*/
  while(scan1->st_time-scan2->st_time > 0)
	{
		if(tempprm2->channel != 0) mychn2 = 1;
		else mychn2 = 0;
		s2=FitFreadRadarScan(fitfp2,&state2,scan2,tempprm2,tempfit2,0,0,mychn2);
	}

	fprintf(stderr,"start merging\n");
  do
  {
    /*array to keep track of matched points*/
    struct combvec **comb = malloc(scan1->num*sizeof(struct combvec *));
    for(j=0;j<scan1->num;j++)
    {
      comb[j] = malloc(scan1->bm[j].nrang*sizeof(struct combvec));
      for(k=0;k<scan1->bm[j].nrang;k++)
      {
        comb[j][k].matched = 0;
        comb[j][k].diff = 51.;
      }
    }
		struct MergeData **data = malloc(scan1->num*sizeof(struct MergeData *));
		for(p=0;p<scan1->num;p++)
		{
			data[p] = malloc(scan1->bm[p].nrang*sizeof(struct MergeData));
			for(k=0;k<scan1->bm[p].nrang;k++)
				memset(&data[p][k],0,sizeof(struct MergeData));
		}
    TimeEpochToYMDHMS(scan1->st_time,&yr,&mo,&dy,&hr,&mn,&sc);
    /*if we have caught up to the first radar, do the combination*/
    if(s1 != -1 && s2 != -1 && scan2->st_time-scan1->st_time < scan1->ed_time-scan1->st_time
			&& scan2->st_time-scan1->st_time >= 0 && hr >= sthr)
    {
      if(vb)
        fprintf(stderr,"%d  %d  %d  %d  %d  %lf\n",yr, mo, dy, hr, mn, sc);
      /*
      fprintf(stderr,"out %lf  %lf\n",scan1->stime,scan2->stime);*/
      for(p=0;p<scan1->num;p++)
      {
        for(k=0;k<scan1->bm[p].nrang;k++)
        {
					fprintf(stderr,"%d\n",scan1->bm[p].channel);
          if(scan1->bm[p].sct[k] == 0 || comb[p][k].matched)
            continue;
          for(m=0;m<scan2->num;m++)
          {
            for(l=0;l<scan2->bm[m].nrang;l++)
            {
              if(scan2->bm[m].sct[l] == 0)
                continue;
							/*FIRST DO GEOGRAPHIC POSITION*/
							for(mag_flg=0;mag_flg<2;mag_flg++)
							{
								if(!mag_flg)
								{
									RPosGeo(1,scan1->bm[p].bm,k,site1,scan1->bm[p].frang,scan1->bm[p].rsep,
													scan1->bm[p].rxrise,300.,&grho,&lat1,&lon1);
									RPosRngBmAzmElv(scan1->bm[p].bm,k,yr,site1,scan1->bm[p].frang,
													scan1->bm[p].rsep,scan1->bm[p].rxrise,300.,&azm1,&elv);
									RPosGeo(1,scan2->bm[m].bm,l,site2,scan2->bm[m].frang,scan2->bm[m].rsep,
													scan2->bm[m].rxrise,300.,&grho,&lat2,&lon2);
									RPosRngBmAzmElv(scan2->bm[m].bm,l,yr,site2,scan2->bm[m].frang,
													scan2->bm[m].rsep,scan2->bm[m].rxrise,300.,&azm2,&elv);
								}
								else
								{
									RPosInvMag(scan1->bm[p].bm,k,yr,site1,scan1->bm[p].frang,
													scan1->bm[p].rsep,scan1->bm[p].rxrise,300.,&lat1,&lon1,&azm1);
									RPosInvMag(scan2->bm[m].bm,l,yr,site2,scan2->bm[m].frang,
													scan2->bm[m].rsep,scan2->bm[m].rxrise,300.,&lat2,&lon2,&azm2);
								}

								lat1 *= pi/180.;
								lat2 *= pi/180.;
								lon1 *= pi/180.;
								lon2 *= pi/180.;
								azm1 *= pi/180.;
								azm2 *= pi/180.;
								v1 = scan1->bm[p].rng[k].v*-1.;
								v2 = scan2->bm[m].rng[l].v*-1.;
								theta1 = pi/2.-azm1;
								theta2 = pi/2.-azm2;
								del_lat = (lat1-lat2);
								del_lon = (lon1-lon2);
								h = haversine(del_lat)+cos(lat1)*cos(lat2)*haversine(del_lon);
								d = 2.*6378.*asin(sqrt(h));
								/*if we have 2 close, unmatched points, or a better match for a point*/
								if((d < 51. && !comb[p][k].matched) || d < comb[p][k].diff)
								{
									double Det_A  =  cos(azm1)*sin(azm2) - cos(azm2)*sin(azm1);
									v_n    = (1./Det_A) * (v1*sin(azm2)  - v2*sin(azm1));
									v_e    = (1./Det_A) * (-v1*cos(azm2) + v2*cos(azm1));
									azm = atan2(v_n,v_e)*180./pi;
									if(v_e >= 0. && v_n >= 0.)
										azm = 90.-azm;
									else if(v_e < 0. && v_n >= 0.)
										azm = 360.-(azm-90.);
									else if(v_e < 0. && v_n < 0.)
										azm = -270.-azm;
									else if(v_e >= 0. && v_n < 0.)
										azm = 90.-azm;
									comb[p][k].matched = 1;
									comb[p][k].diff = d;

									if(!mag_flg)
									{
										data[p][k].radar1.stid=scan1->stid;
										data[p][k].radar1.cpid=scan1->bm[p].cpid;
										data[p][k].radar1.beam=scan1->bm[p].bm;
										data[p][k].radar1.qflg=scan1->bm[p].sct[k];
										data[p][k].radar1.rng=k;
										data[p][k].radar1.gsflg=scan1->bm[p].rng[k].gsct;
										data[p][k].radar1.vel=scan1->bm[p].rng[k].v;
										data[p][k].radar1.vel_err=scan1->bm[p].rng[k].v_e;
										data[p][k].radar1.pwr=scan1->bm[p].rng[k].p_l;
										data[p][k].radar1.w_l=scan1->bm[p].rng[k].w_l;
										data[p][k].radar1.glat=lat1*180./pi;
										data[p][k].radar1.glon=lon1*180./pi;
										data[p][k].radar1.gazm=azm1*180./pi;
										data[p][k].radar1.st_time=scan1->st_time;
										data[p][k].radar1.ed_time=scan1->ed_time;

										data[p][k].radar2.stid=scan2->stid;
										data[p][k].radar2.cpid=scan2->bm[m].cpid;
										data[p][k].radar2.beam=scan2->bm[m].bm;
										data[p][k].radar2.qflg=scan2->bm[m].sct[l];
										data[p][k].radar2.rng=l;
										data[p][k].radar2.gsflg=scan2->bm[m].rng[l].gsct;
										data[p][k].radar2.vel=scan2->bm[m].rng[l].v;
										data[p][k].radar2.vel_err=scan2->bm[m].rng[l].v_e;
										data[p][k].radar2.pwr=scan2->bm[m].rng[l].p_l;
										data[p][k].radar2.w_l=scan2->bm[m].rng[l].w_l;
										data[p][k].radar2.glat=lat2*180./pi;
										data[p][k].radar2.glon=lon2*180./pi;
										data[p][k].radar2.gazm=azm2*180./pi;
										data[p][k].radar2.st_time=scan2->st_time;
										data[p][k].radar2.ed_time=scan2->ed_time;

										data[p][k].st_time=scan1->st_time;
										data[p][k].ed_time=scan1->ed_time;

										data[p][k].velg=sqrt(v_e*v_e+v_n*v_n);
										data[p][k].glat=lat1*180./pi;
										data[p][k].glon=lon1*180./pi;
										data[p][k].gazm=azm;
									}
									else
									{
										data[p][k].radar1.mlat=lat1*180./pi;
										data[p][k].radar1.mlon=lon1*180./pi;
										data[p][k].radar1.mazm=azm1*180./pi;
										data[p][k].radar2.mlat=lat2*180./pi;
										data[p][k].radar2.mlon=lon2*180./pi;
										data[p][k].radar2.mazm=azm2*180./pi;
										data[p][k].velm=sqrt(v_e*v_e+v_n*v_n);
										data[p][k].mlat=lat1*180./pi;
										data[p][k].mlon=lon1*180./pi;
										data[p][k].mazm=azm;
									}
								}
							}
            }
          }
        }
      }
    }

		for(p=0;p<scan1->num;p++)
			for(k=0;k<scan1->bm[p].nrang;k++)
				if(data[p][k].radar1.qflg != 0 && data[p][k].radar2.qflg != 0)
					MergeFwrite(stdout,&data[p][k]);

		for(p=0;p<scan1->num;p++)
			free(data[p]);
		free(data);

    for(j=0;j<scan1->num;j++)
      free(comb[j]);
    free(comb);

    if(s2 != -1 && scan2->st_time-scan1->st_time < scan1->ed_time-scan1->st_time)
		{
			if(tempprm2->channel != 0) mychn2 = 1;
			else mychn2 = 0;
			s2=FitFreadRadarScan(fitfp2,&state2,scan2,tempprm2,tempfit2,0,0,mychn2);
		}

    /*read the next record from the first radar*/
    if(s1 != -1 && scan2->st_time-scan1->st_time >= scan1->ed_time-scan1->st_time)
		{
			if(tempprm2->channel != 0) mychn2 = 1;
			else mychn2 = 0;
			s1=FitFreadRadarScan(fitfp1,&state1,scan1,tempprm1,tempfit1,0,0,0);
		}
    TimeEpochToYMDHMS(scan1->st_time,&yr,&mo,&dy,&hr,&mn,&sc);

  } while(s1 != -1 && s2 != -1 && hr < edhr);

  fprintf(stderr,"Done merging\n");
  /*fprintf(stdout,"%d\n",mag_flg);*/

	fclose(fitfp1);
	fclose(fitfp2);
  return 0;
}
