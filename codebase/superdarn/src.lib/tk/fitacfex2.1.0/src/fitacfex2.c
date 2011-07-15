
/* fitacfex2.c
   ==========
   Algorithm: R.A.Greenwald, K.Oskavik
   Implementation: R.J.Barnes, R.A.Greenwald
*/


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "rtypes.h"
#include "rmath.h"
#include "nrfit.h"
#include "rprm.h"
#include "rawdata.h"
#include "fitdata.h"
#include "fitblk.h"
#include "fitacfex2.h"
#include "badsmp.h"
#include "badlags.h"
#include "badsmp.h"
#include "lmfit.h"

/*
 $Log: fitacfex2.c,v $
 Revision 1.6  2007/05/22 20:46:00  code
 Fixed bug in determining the mean and calculating ground scatter.


*/


double elevation_ex(struct FitPrm *prm,double phi0) {

  double k;          /* wave number; 1/m */
  double phi;        /* beam direction off boresight; rad */
  double c_phi;      /* cosine of phi                     */
  double dchi_cable; /* phase shift caused by cables; rad */
  double chi_max;    /* maximum phase shift possible; rad */
  double phi_temp;   /* actual phase angle + cable;   rad */
  double psi;        /* actual phase angle - cable;   rad */
  double theta;      /* angle of arrival for horizontal antennas; rad */
  double offset=7.5; /* offset in beam widths to the edge of the array */
  static double antenna_separation= 0.0; /* m */
  static double elev_corr= 0.0;
  /* elevation angle correction, if antennas are at different heights; rad */
  static double phi_sign= 0;
  /* +1 if interferometer antenna is in front of main antenna, -1 otherwise*/

  /* calculate the values that don't change if this hasn't already been done. */

  if (antenna_separation == 0.0) {
    antenna_separation= sqrt(prm->interfer[1]*prm->interfer[1] +
			                 prm->interfer[0]*prm->interfer[0] +
	                         prm->interfer[2]*prm->interfer[2]);
    elev_corr= prm->phidiff* asin( prm->interfer[2]/ antenna_separation);
    if (prm->interfer[1] > 0.0) /* interferometer in front of main antenna */
      phi_sign= 1.0;
    else {                           /* interferometer behind main antenna */
      phi_sign= -1.0;
      elev_corr= -elev_corr;
    }
  }
  offset=prm->maxbeam/2.0-0.5;
  phi= prm->bmsep*(prm->bmnum - offset)* PI/ 180.0;
  c_phi= cos( phi);
  k= 2 * PI * prm->tfreq * 1000.0/C;

  /* the phase difference phi0 is between -pi and +pi and gets positive,  */
  /* if the signal from the interferometer antenna arrives earlier at the */
  /* receiver than the signal from the main antenna. */
  /* If the cable to the interferometer is shorter than the one to */
  /* the main antenna, than the signal from the interferometer     */
  /* antenna arrives earlier. tdiff < 0  --> dchi_cable > 0        */

  dchi_cable= - 2* PI * prm->tfreq * 1000.0 * prm->tdiff * 1.0e-6;

  /* If the interferometer antenna is in front of the main antenna */
  /* then lower elevation angles correspond to earlier arrival     */
  /* and greater phase difference. */
  /* If the interferometer antenna is behind of the main antenna   */
  /* then lower elevation angles correspond to later arrival       */
  /* and smaller phase difference */

  chi_max= phi_sign* k* antenna_separation* c_phi + dchi_cable;

  /* change phi0 by multiples of twopi, until it is in the range   */
  /* (chi_max - twopi) to chi_max (interferometer in front)        */
  /* or chi_max to (chi_max + twopi) (interferometer in the back)  */

  phi_temp= phi0 + 2*PI* floor( (chi_max - phi0)/ (2*PI));
  if (phi_sign < 0.0) phi_temp= phi_temp + (2*PI);

  /* subtract the cable effect */
  psi= phi_temp - dchi_cable;
  theta= psi/ (k* antenna_separation);
  theta= (c_phi* c_phi - theta* theta);
  /* set elevation angle to 0 for out of range values */

  if ( (theta < 0.0) || (fabs( theta) > 1.0) ) theta= - elev_corr;
  else theta= asin( sqrt( theta));


  return 180.0* (theta + elev_corr)/ PI; /* in degree */
}



double bisect(float w_guess, float diff, struct RawData *raw, float *good_lags, int goodcnt,
              int R, float *lagpwr, double pwr, struct RadarParm *prm, int print)
{
  float c_fac = (3. - sqrt(5))/2.;
  float r_fac = 1. - c_fac;
  float xr, xl, x0, x1, x2, x3, f1, f2;
  int count, maxiter;
  maxiter = 100;


  count = 0;
  xl = w_guess-diff;
  xr = w_guess+diff;
  x0 = w_guess-diff;
  x3 = w_guess+diff;
  x1 = w_guess;
  x2 = w_guess + c_fac*diff;
  f1=calc_err(x1,raw,good_lags,goodcnt,R,lagpwr,pwr,prm);
  f2=calc_err(x2,raw,good_lags,goodcnt,R,lagpwr,pwr,prm);

  while (fabs(f1-f2) > 0.005)
  {
    if (f2 < f1)
    {
      x0 = x1;
      x1 = x2;
      x2 = r_fac*x1 + c_fac*x3;
      f1 = f2;
			if(print)
				fprintf(stdout,"%lf\n%lf\n",x2,f2);
      f2=calc_err(x2,raw,good_lags,goodcnt,R,lagpwr,pwr,prm);
    }
    else
    {
      x3 = x2;
      x2 = x1;
      x1 = r_fac*x2 + c_fac*x0;
      f2 = f1;
			if(print)
				fprintf(stdout,"%lf\n%lf\n",x1,f1);
      f1=calc_err(x1,raw,good_lags,goodcnt,R,lagpwr,pwr,prm);
    }
    if (fabs(w_guess-xr) < 0.01)
    {
      fprintf(stderr, "Right Boundary %d:%d:%d Range %d Dist %e\n", prm->time.hr, prm->time.mt, prm->time.sc, R, fabs(w_guess-xr));
      break;
    }
    if (fabs(w_guess-xl) < 0.01)
    {
      fprintf(stderr, "Left Boundary %d:%d:%d Range %d Dist %e\n", prm->time.hr, prm->time.mt, prm->time.sc, R, fabs(w_guess-xl));
      break;
    }
    count += 1;
    if (count > maxiter)
    {
      fprintf(stderr, "Curious, bisection method does not converge: %d:%d:%d Range %d\n", prm->time.hr, prm->time.mt, prm->time.sc, R);
      break;
    }
  }
  if (f1 < f2)
	{
		if(print)
			fprintf(stdout,"%lf\n%lf\n",x1,f1);
    return x1;
	}
  else
	{
		if(print)
			fprintf(stdout,"%lf\n%lf\n",x2,f2);
    return x2;
	}
}

void fitacfex2(struct RadarParm *prm,struct RawData *raw,
              struct FitData *fit, struct FitBlock *fblk, int print)
{
  float minpwr  = 3.0;
  double skynoise = 0.;
  float sderr   = 3.0;
  double phi0;
  int   minlag  = 4;
  int   nslopes = 120;
  int availflg = 0;
  int pwr_flg,sct_flg;
  float a,b,siga,sigb,chi2,q;
  float *model_phi,*model_vels,*model_errors,*xcf_phases;
  float model_slope,model_vel_pos;
  float model_mean,model_sd,model_min;
  float *data_phi_pos,*data_phi_neg,data_phi;
  float *lagpwr=NULL,*logpwr=NULL,*good_lags=NULL;
  float lag0pwr,re,im,pwr,phi;
  float fitted_width=0.0,fitted_power=0.0;
  float delta_pos,delta_neg,error_neg=0,error_pos=0;
  int   *lag_avail=NULL,availcnt=0,goodcnt=0;
  int   mininx=0,lastlag,lag,i,j,p,R,L;
  double prev_err,v_temp,w_guess;
  float diff,err;
  int *badlag = malloc(prm->mplgs * sizeof(int));
  struct FitACFBadSample badsmp;

  /* need this for bisection method */
  diff=(180.0/nslopes);

  /* Find the highest lag, and allocate memory */

  if(!((prm->cp == 3310 || prm->cp == 503 || prm->cp == -503) && prm->mplgs == 18))
  {
    lastlag = 0;
    for (j=0;j<prm->mplgs;j++)
    {
      if (abs(prm->lag[0][j]-prm->lag[1][j])>lastlag)
      {
        lastlag = abs(prm->lag[0][j]-prm->lag[1][j]);
      }
    }
  }
  else
    lastlag=prm->mplgs-1;



  model_phi    = malloc(sizeof(float)*(nslopes+1)*(lastlag+1));
  model_vels   = malloc(sizeof(float)*(2*nslopes+1));
  model_errors = malloc(sizeof(float)*(2*nslopes+1));
  lagpwr       = malloc(sizeof(float)*(lastlag+1));
  xcf_phases   = malloc(sizeof(float)*(lastlag+1));
  logpwr       = malloc(sizeof(float)*(lastlag+1));
  data_phi_pos = malloc(sizeof(float)*(lastlag+1));
  data_phi_neg = malloc(sizeof(float)*(lastlag+1));
  lag_avail    = malloc(sizeof(int)*(lastlag+1));
  good_lags    = malloc(sizeof(float)*(lastlag+1));

/* Generate models that will be used in the velocity determination */
  for(i=0;i<=nslopes;i++)
  {
    model_slope = 180.0*i/nslopes;
    for(j=0;j<=lastlag;j++)
    {
      phi = j*model_slope;
      p = phi/360;
      model_phi[i*(lastlag+1)+j] = phi - p*360;
    }
    model_vel_pos = 2.9979E8/2.0*(1-1000.0*prm->tfreq/
        (1000.0*prm->tfreq+model_slope/360.0/(prm->mpinc*1.0e-6)));
    model_vels[nslopes-i] = -model_vel_pos;
    model_vels[nslopes+i] =  model_vel_pos;
  }

  /*setup fitblock parameter*/
  setup_fblk(prm, raw, fblk);

  FitSetRng(fit,fblk->prm.nrang);
  FitSetXrng(fit,fblk->prm.nrang);
  FitSetElv(fit,fblk->prm.nrang);

  /*calculate noise levels*/
  lm_noise_stat(prm,raw,&skynoise);
  if(prm->cp != 3310 && prm->cp != 503 && prm->cp != -503)
  {
    if(fblk->prm.channel==0) FitACFBadlags(&fblk->prm,&badsmp);
    else FitACFBadlagsStereo(&fblk->prm,&badsmp);
  }

	if(print)
	{
		fprintf(stdout,"%d  %d  %lf  %d  %lf  %d  %lf\n",prm->nrang,prm->mplgs,skynoise,prm->tfreq,prm->mpinc*1.e-6,nslopes,diff);
		fprintf(stdout,"%d  %d  %d  %d  %d  %d  %d  %d\n",prm->stid,prm->time.yr,prm->time.mo,
										prm->time.dy,prm->time.hr,prm->time.mt,(int)prm->time.sc,prm->bmnum);
	}
	/* Loop every range gate and calculate parameters */
  for (R=0;R<prm->nrang;R++)
  {

    raw->acfd[0][R*prm->mplgs] -= skynoise;

    fit->rng[R].v        = 0.;
    fit->rng[R].v_err    = HUGE_VAL;
    fit->rng[R].p_0      = 0.0;
    fit->rng[R].w_l      = 0.0;
    fit->rng[R].w_l_err  = 0.0;
    fit->rng[R].p_l      = 0.0;
    fit->rng[R].p_l_err  = 0.0;
    fit->rng[R].w_s      = 0.0;
    fit->rng[R].w_s_err  = 0.0;
    fit->rng[R].p_s      = 0.0;
    fit->rng[R].p_s_err  = 0.0;
    fit->rng[R].sdev_l   = 0.0;
    fit->rng[R].sdev_s   = 0.0;
    fit->rng[R].sdev_phi = 0.0;
    fit->rng[R].qflg     = 0;
    fit->rng[R].gsct     = 0;
    fit->rng[R].nump     = 0;
    availcnt = 0;

    lag0pwr  = 10.0*log10((raw->acfd[0][R*prm->mplgs] +
                            skynoise)/skynoise);

		if(print)
			fprintf(stdout,"%d  %lf\n",R,raw->acfd[0][R*prm->mplgs]/sqrt(1.0*prm->nave));

    for(j=0;j<=2*nslopes;j++)
      model_errors[j] = 1.0e30;


    prm->mplgexs = 0;

    if((prm->cp == 3310 || prm->cp == 503 || prm->cp == -503) && prm->mplgs == 18)
    {
      for (L=0;L<prm->mplgs;L++)
      {
        lag = L;
        re  = raw->acfd[0][R*prm->mplgs+L];
        im  = raw->acfd[1][R*prm->mplgs+L];
        lagpwr[lag] = sqrt(re*re + im*im);
        if (lagpwr[lag]>raw->acfd[0][R*prm->mplgs]/sqrt(1.0*prm->nave))
        {
            lag_avail[availcnt] = lag;
            availcnt++;
        }
        else lagpwr[lag] = 0.0;
        if(print)
					fprintf(stdout,"%d  %lf  %lf  %d\n",lag,raw->acfd[0][R*prm->mplgs+L],raw->acfd[1][R*prm->mplgs+L],
																						(lagpwr[lag]>raw->acfd[0][R*prm->mplgs]/sqrt(1.0*prm->nave)));
      }
      pwr_flg = (lag0pwr>=minpwr);
    }
    /*check for tauscan operation (lag power checking, no badlag checking, SNR checking)*/
    else if(prm->cp == 3310 || prm->cp == 503 || prm->cp == -503)
    {
      for (L=0;L<prm->mplgs;L++)
      {
        lag = abs(prm->lag[0][L] - prm->lag[1][L]);
        re  = raw->acfd[0][R*prm->mplgs+L];
        im  = raw->acfd[1][R*prm->mplgs+L];
        lagpwr[lag] = sqrt(re*re + im*im);
        if (lagpwr[lag]>raw->acfd[0][R*prm->mplgs]/sqrt(1.0*prm->nave))
        {
            lag_avail[availcnt] = lag;
            availcnt++;
        }
        else lagpwr[lag] = 0.0;
        if(print)
					fprintf(stdout,"%d  %lf  %lf  %d\n",lag,raw->acfd[0][R*prm->mplgs+L],raw->acfd[1][R*prm->mplgs+L],
																								(lagpwr[lag]>raw->acfd[0][R*prm->mplgs]/sqrt(1.0*prm->nave)));
      }
      pwr_flg = (lag0pwr>=minpwr);
    }
    /*check for non-tauscan operation (lag power checking, badlag checking, no SNR checking)*/
    else
    {
      FitACFCkRng(R+1,badlag,&badsmp,&fblk->prm);
      for (L=0;L<prm->mplgs;L++)
      {
        lag = abs(prm->lag[0][L] - prm->lag[1][L]);
        re  = raw->acfd[0][R*prm->mplgs+L];
        im  = raw->acfd[1][R*prm->mplgs+L];
        lagpwr[lag] = sqrt(re*re + im*im);
        if(badlag[L] == 1)
          availflg = 0;
        else
          availflg = 1;
        if(availflg && lagpwr[lag]>raw->acfd[0][R*prm->mplgs]/sqrt(1.0*prm->nave))
        {
          lag_avail[availcnt] = lag;
          availcnt++;
        }
        else lagpwr[lag] = 0.0;
        if(print)
					fprintf(stdout,"%d  %lf  %lf  %d\n",lag,raw->acfd[0][R*prm->mplgs+L],raw->acfd[1][R*prm->mplgs+L],
																						(availflg && lagpwr[lag]>raw->acfd[0][R*prm->mplgs]/sqrt(1.0*prm->nave)));
      }
      pwr_flg = (sqrt(raw->acfd[0][R*prm->mplgs]*raw->acfd[0][R*prm->mplgs])>=skynoise);
      minlag = 4;
    }

    if(print)
			fprintf(stdout,"%d  %d\n",(pwr_flg),(availcnt>=minlag));

    /*if SNR is high enough and we have ge 6 good lags*/
    if((pwr_flg) && (availcnt>=minlag))
    {
      /* Determine Lambda Power and Spectral Width from least square fit */
      goodcnt = 0;
      for(i=0;i<availcnt;i++)
      {
        lag = lag_avail[i];
        logpwr[goodcnt]    = log(lagpwr[lag]);
        good_lags[goodcnt] = lag;
        goodcnt++;
      }
      nrfit(good_lags,logpwr,goodcnt,NULL,0,&a,&b,&siga,&sigb,&chi2,&q);
      fitted_width = -2.9979e8*b/(prm->mpinc*1.e-6)/
                            (2*PI*1000.0*prm->tfreq);
      if(fitted_width<0.00) fitted_width = 0.0;
      fitted_power = 10.0*log10((exp(a) + skynoise)/skynoise);

			if(print)
				fprintf(stdout,"%lf  %lf\n",fitted_power,fitted_width);

      /* Determine Doppler velocity by comparing the phase with models */
      pwr = 0.0;
      for(i=0;i<goodcnt;i++)
      {
        lag = good_lags[i];
        if((prm->cp == 3310 || prm->cp == 503 || prm->cp == -503) && prm->mplgs == 18)
        	L = lag;
        else
        	for(j=0;j<prm->mplgs;j++)
          {
            if(abs(prm->lag[0][L]-prm->lag[1][L])==lag)
            {
              L = j;
            }
          }

        data_phi = atan2(raw->acfd[1][R*prm->mplgs+L],raw->acfd[0][R*prm->mplgs+L])*180.0/PI;
				if(fblk->prm.xcf)
					xcf_phases[i]=atan2(raw->xcfd[1][R*prm->mplgs+L],raw->xcfd[0][R*prm->mplgs+L])*180./PI;
        data_phi_pos[i] = data_phi;
        data_phi_neg[i] = 360 - data_phi;
        if(data_phi<0)
        {
          data_phi_pos[i] += 360;
					data_phi_neg[i]  = -data_phi;
        }
        pwr += lagpwr[lag];
      }
      for(i=0;i<=nslopes;i++)
      {
        error_neg = 0;
        error_pos = 0;
        for(j=0;j<goodcnt;j++)
        {
          lag = good_lags[j];
          delta_pos = fabs(data_phi_pos[j] - model_phi[i*(lastlag+1)+lag]);
          delta_neg = fabs(data_phi_neg[j] - model_phi[i*(lastlag+1)+lag]);
          if (delta_pos>180.0) delta_pos = 360 - delta_pos;
          if (delta_neg>180.0) delta_neg = 360 - delta_neg;
          error_neg += delta_neg*delta_neg*lagpwr[lag]/pwr;
          error_pos += delta_pos*delta_pos*lagpwr[lag]/pwr;
        }
        error_neg = sqrt(error_neg);
        error_pos = sqrt(error_pos);
        model_errors[nslopes-i] = error_neg;
        model_errors[nslopes+i] = error_pos;
      }

      for(i=0;i<nslopes*2+1;i++)
				if(print)
					fprintf(stdout,"%lf  %lf\n",i*diff-180.,model_errors[i]);

      /*check for aliasing limit*/
      int concnt = 0;
      int cons = 1;
      int maxcons = 1;
      for(j=1;j<goodcnt;j++)
      {
        if(good_lags[j] - good_lags[j-1] == 1)
        {
          concnt++;
          cons++;
        }
        else
        {
          if(cons > maxcons)
            maxcons = cons;
          cons = 1;
        }
      }
      int alias = 1;
      /*if we don't have consecutive lags at least twice,
        or at least 3 consecutive lags once, then cut model range in half*/
      if(concnt >= 2 || maxcons >= 3)
        alias = 0;

      model_mean = 0.0;
      model_sd   = 0.0;
      model_min  = 1.0e30;
      mininx     = 0;
      for(i=0;i<=nslopes*2;i++)
      {
        model_mean += model_errors[i];
        if((model_errors[i]<model_min && !alias) ||
            (model_errors[i]<model_min && i > nslopes/2. && i < nslopes*1.5))
        {
          model_min = model_errors[i];
          mininx = i;
        }
      }
      model_mean = model_mean/(nslopes*2+1);


      /* Only keep values giving a fit better than 'sterr' Standard Deviations */
      for(i=0;i<=nslopes*2;i++)
        model_sd += (model_errors[i] - model_mean)*(model_errors[i] - model_mean);

      model_sd = sqrt(model_sd/(nslopes*2));

			if(prm->stid == 204 || prm->stid == 205)
				minpwr = 5.;

      /*tauscan operation, check for exceptional minimum error, more SNR checking*/
      if(prm->cp == 3310 || prm->cp == 503 || prm->cp == -503)
        sct_flg = ((model_min<(model_mean - sderr*model_sd)) &&
                  (10*log10((exp(a) + prm->noise.search)/prm->noise.search) > minpwr));
      /*non-tauscan operation, check for exceptional minimum error, no badlag checking*/
      else if(prm->stid == 204 || prm->stid == 205)
        sct_flg = (model_min<(model_mean - sderr*model_sd) &&
                  (10*log10((exp(a) + skynoise)/skynoise) > minpwr));
      else
        sct_flg = (model_min<(model_mean - sderr*model_sd) &&
                  (10*log10((exp(a) + skynoise)/skynoise) > minpwr));
      fit->rng[R].p_0   = lag0pwr;
			
			if(print)
				fprintf(stdout,"%lf  %d  %d  %d\n",(model_mean - sderr*model_sd),mininx,
																		(10*log10((exp(a) + skynoise)/skynoise) > minpwr),(model_min<(model_mean - sderr*model_sd)));

      if(sct_flg)
      {
        v_temp = model_vels[mininx];
        err=999.;
        prev_err=9999.;
        w_guess = (mininx-nslopes)*diff;


				/*proper bisection method*/
        w_guess = bisect(w_guess,diff,raw,good_lags,goodcnt,R,lagpwr,pwr,prm,print);

				if(print)
					fprintf(stdout,"4321\n");

        fit->rng[R].v     = fblk->prm.vdir*2.9979E8/2.0*(1-1000.0*prm->tfreq/
                            (1000.0*prm->tfreq+w_guess/360.0/(prm->mpinc*1.0e-6)));


        fit->rng[R].v_err   = calc_err(w_guess,raw,good_lags,goodcnt,R,lagpwr,pwr,prm);

				if(print)
					fprintf(stdout,"%lf  %lf  %lf\n",w_guess,fit->rng[R].v,fit->rng[R].v_err);

        fit->rng[R].qflg  = 1;

        fit->rng[R].p_l   = 10.0*(fitted_power/2.3026 - log10(skynoise));

        fitted_power = log(exp(siga) + prm->noise.search);
        fit->rng[R].p_l_err   = fabs(fitted_power);

        fit->rng[R].w_l   = fitted_width;
        fitted_width = 2.9979e8*sigb/(prm->mpinc*1.e-6)/
                            (2*PI*1000.0*prm->tfreq);
        if(fitted_width<0.00) fitted_width = 0.0;
        fit->rng[R].w_l_err = fitted_width;

        fit->rng[R].nump  = goodcnt;

        fit->noise.skynoise = skynoise;

        if ((fabs(fit->rng[R].v)<30) && (fit->rng[R].w_l<30))
              fit->rng[R].gsct = 1;
        else fit->rng[R].gsct = 0;

				if(prm->xcf)
				{
					int wrap_cnt = 0;
					float m_p2,m_p;
					for(i=1;i<goodcnt;i++)
					{
						m_p2 = w_guess * good_lags[i];
						m_p = w_guess * good_lags[i-1];

						while(m_p2 < -180.)
							m_p2 += 360.;
						while(m_p2 > 180.)
							m_p2 -= 360.;

						while(m_p < -180.)
							m_p += 360.;
						while(m_p > 180.)
							m_p -= 360.;

						if((w_guess < 0 && m_p2 > 0 && m_p < 0) || (w_guess > 0 && m_p2 < 0 && m_p > 0))
						{
							wrap_cnt++;
						}
						if(wrap_cnt != 0)
						{
							xcf_phases[i] += (wrap_cnt-1)*(360.);
							if(fabs(xcf_phases[i] - m_p2+(360.*(wrap_cnt-1))) > 180.)
								xcf_phases[i] += (360.);
						}

					}
					phi0 = calc_phi0(good_lags,xcf_phases, w_guess, goodcnt)*PI/180.;
					fit->xrng[R].phi0 = phi0;
					fit->elv[R].normal = elevation_ex(&fblk->prm,phi0);
					fit->elv[R].high = elevation_ex(&fblk->prm,phi0);
					fit->elv[R].low = elevation_ex(&fblk->prm,phi0);
				}
				else
					{
						fit->xrng[R].phi0 = 0.;
						fit->elv[R].normal = 0.;
						fit->elv[R].high = 0.;
						fit->elv[R].low = 0.;
					}
      }
    }
  }

  free(model_phi);
  free(model_vels);
  free(model_errors);
  free(lagpwr);
  free(logpwr);
  free(data_phi_pos);
  free(data_phi_neg);
  free(lag_avail);
  free(good_lags);

  return;
}
