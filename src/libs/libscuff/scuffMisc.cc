/*
 * scuffMisc.cc  -- miscellaneous stuff for libscuff
 *
 * homer reid  -- 10/2006
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "libscuff.h"

namespace scuff {

/*--------------------------------------------------------------*/
/*--------------------------------------------------------------*/
/* vector routines                                              */
/*--------------------------------------------------------------*/
/*--------------------------------------------------------------*/

/* v <= 0 */
void VecZero(double *v)
{ memset(v,0,3*sizeof(double));
}


/* v *= alpha */
double *VecScale(double *v, double alpha)
{ v[0]*=alpha;
  v[1]*=alpha;
  v[2]*=alpha;
  return v;
}

/* v3 = v1 + alpha*v2 */
double *VecScaleAdd(double *v1, double alpha, double *v2, double *v3)
{ v3[0]=v1[0] + alpha*v2[0];
  v3[1]=v1[1] + alpha*v2[1];
  v3[2]=v1[2] + alpha*v2[2];
  return v3;
}

/* v3 = alpha*v1 + beta*v2 */
double *VecLinComb(double alpha, double *v1, double beta, double *v2, double *v3)
{ v3[0]=alpha*v1[0] + beta*v2[0];
  v3[1]=alpha*v1[1] + beta*v2[1];
  v3[2]=alpha*v1[2] + beta*v2[2];
  return v3;
}

/* v3 = v1 + v2 */
double *VecAdd(double *v1, double *v2, double *v3)
{ v3[0]=v1[0] + v2[0];
  v3[1]=v1[1] + v2[1];
  v3[2]=v1[2] + v2[2];
  return v3;
}

/* v3 = v1 - v2 */
double *VecSub(double *v1, double *v2, double *v3)
{ v3[0]=v1[0] - v2[0];
  v3[1]=v1[1] - v2[1];
  v3[2]=v1[2] - v2[2];
  return v3;
}

/* v1 += alpha*v2 */
double *VecPlusEquals(double *v1, double alpha, double *v2)
{ v1[0]+=alpha*v2[0];
  v1[1]+=alpha*v2[1];
  v1[2]+=alpha*v2[2];
  return v1;
}

/* v3 = v1 \times v2 */
double *VecCross(double *v1, double *v2, double *v3)
{ v3[0]=v1[1]*v2[2] - v1[2]*v2[1];
  v3[1]=v1[2]*v2[0] - v1[0]*v2[2];
  v3[2]=v1[0]*v2[1] - v1[1]*v2[0];
  return v3;
}

/* return v1 \dot v2 */
double VecDot(double *v1, double *v2)
{ return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2]; }

double VecNorm2(double *v)
{ return VecDot(v,v); }

double VecNorm(double *v)
{ return sqrt(VecDot(v,v)); }

double VecDistance(double *v1, double *v2)
{ double v3[3];
  VecSub(v1,v2,v3);
  return VecNorm(v3);
}

double VecDistance2(double *v1, double *v2)
{ double v3[3];
  VecSub(v1,v2,v3);
  return VecDot(v3,v3);
}

double VecNormalize(double *v)
{ double d=VecNorm(v);  
  v[0]/=d;
  v[1]/=d;
  v[2]/=d;
  return d;
}

/*--------------------------------------------------------------*/
/*--------------------------------------------------------------*/
/*- miscellaneous miscellany------------------------------------*/
/*--------------------------------------------------------------*/
/*--------------------------------------------------------------*/

double RelDiff(double x, double y)
{ return 2.0*fabs(x-y) / ( fabs(x) + fabs(y) ); }

/***************************************************************/
/* RWGErrExit:  print an error message and quit.            *******/
/***************************************************************/
void RWGErrExit(const char *format, ...)
{
 va_list ap;
 char buffer[200];

 va_start(ap,format);
 vsnprintf(buffer,200,format,ap);
 fprintf(stderr,"error: %s (aborting)\n",buffer);
 //Log("error: %s (aborting)",buffer);
 va_end(ap);
 exit(1);

}

/***************************************************************/
/* mallocEC with error checking                                  */
/***************************************************************/
void *mallocEC(int size)
{
 void *p=mallocEC(size);

 if (!p) RWGErrExit("out of memory");
 return p;
}

} // namespace scuff
