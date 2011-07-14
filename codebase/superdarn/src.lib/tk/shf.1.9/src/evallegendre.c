/* evallegendre.c
   ============== 
   Author: R.J.Barnes
*/

/*
 (c) 2010 JHU/APL & Others - Please Consult LICENSE.superdarn-rst.3.2-beta-4-g32f7302.txt for more information.
 
 
 
*/


#include <math.h>
#include <stdio.h>
#include "evallegendre.h"





double CnvMapEvalPLM(int l,int m,double x) {
  double pmm=1.0,pnmp1=0,pll=0;
  double somx2=0;
  double fact=0;
  int i=0,ll=0;
  if (m>0) { 
    somx2=sqrt((1.0-x)*(1.0+x));
    fact=1.0;
    for (i=1;i<=m;i++) {
      pmm=-pmm*fact*somx2;
      fact+=2.0;
    }
  }
  if (l==m) return pmm;
  else {
    pnmp1=x*(2*m+1)*pmm;
    if (l==(m+1)) return pnmp1;
    else {
      for (ll=m+2;ll<=l;ll++) {
        pll=(x*(2*ll-1)*pnmp1-(ll+m-1)*pmm)/(ll-m);
        pmm=pnmp1;
        pnmp1=pll;
      }
    }
  }
  return pll;
}


void CnvMapEvalLegendre(int Lmax, double *x,int n, double *plm) {
 
  int L,m,i;
  
  for (i=0; i<n; ++i) {
    for (L=0;L<=Lmax;L++) {
      for (m=0;m<=Lmax;m++) {
         PLM(L,m,i)=CnvMapEvalPLM(L,m,x[i]);
      }
    }
  }
}
    