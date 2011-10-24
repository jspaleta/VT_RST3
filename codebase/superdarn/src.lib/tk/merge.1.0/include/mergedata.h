/* rprm.h
   ======
   Author: R.J.Barnes
*/


/*
 (c) 2010 JHU/APL & Others - Please Consult LICENSE.superdarn-rst.3.2-beta-4-g32f7302.txt for more information.



*/

#ifndef _MERGEDATA_H
#define _MERGEDATA_H

struct MergeData {

  struct {
    int16 stid;
    int16 cpid;
    int16 beam;
    int16 rng;
    int16 gsflg;
		float vel;
		float vel_err;
		float pwr;
		float w_l;
		int16 qflg;
		float glat;
		float glon;
		float gazm;
		float mlat;
		float mlon;
		float mazm;
		int32 st_time;
		int32 ed_time;
  } radar1;

  struct {
    int16 stid;
    int16 cpid;
    int16 beam;
    int16 rng;
    int16 gsflg;
		float vel;
		float vel_err;
		float pwr;
		float w_l;
		int16 qflg;
		float glat;
		float glon;
		float gazm;
		float mlat;
		float mlon;
		float mazm;
		int32 st_time;
		int32 ed_time;
  } radar2;


	float velg;
	float velm;
	float glat;
	float glon;
	float gazm;
	float mlat;
	float mlon;
	float mazm;
	float dgeo;
	float dmag;

	int32 st_time;
	int32 ed_time;


};

int MergeEncode(struct DataMap *ptr,struct MergeData *data);
int MergeDecode(struct DataMap *ptr,struct MergeData *data);

int MergeRead(int fid,struct MergeData *data);
int MergeFRead(FILE *fp,struct MergeData *data);
int MergeWrite(int fid,struct MergeData *data);
int MergeFwrite(FILE *fp,struct MergeData *data);




#endif
