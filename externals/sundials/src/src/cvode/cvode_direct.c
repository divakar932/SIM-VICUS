/*
 * -----------------------------------------------------------------
 * $Revision: 4749 $
 * $Date: 2016-04-23 18:42:38 -0700 (Sat, 23 Apr 2016) $
 * ----------------------------------------------------------------- 
 * Programmer: Radu Serban @ LLNL
 * -----------------------------------------------------------------
 * LLNS Copyright Start
 * Copyright (c) 2014, Lawrence Livermore National Security
 * This work was performed under the auspices of the U.S. Department 
 * of Energy by Lawrence Livermore National Laboratory in part under 
 * Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
 * Produced at the Lawrence Livermore National Laboratory.
 * All rights reserved.
 * For details, see the LICENSE file.
 * LLNS Copyright End
 * -----------------------------------------------------------------
 * This is the implementation file for the CVDLS linear solvers
 * -----------------------------------------------------------------
 */

/* 
 * =================================================================
 * IMPORTED HEADER FILES
 * =================================================================
 */

#include <stdio.h>
#include <stdlib.h>

#include "cvode_impl.h"
#include "cvode_direct_impl.h"
#include <sundials/sundials_math.h>
#include <sundials/sundials_timer.h>

/* 
 * =================================================================
 * FUNCTION SPECIFIC CONSTANTS
 * =================================================================
 */

/* Constant for DQ Jacobian approximation */
#define MIN_INC_MULT RCONST(1000.0)

#define ZERO         RCONST(0.0)
#define ONE          RCONST(1.0)
#define TWO          RCONST(2.0)

/*
 * =================================================================
 * READIBILITY REPLACEMENTS
 * =================================================================
 */

#define f         (cv_mem->cv_f)
#define user_data (cv_mem->cv_user_data)
#define uround    (cv_mem->cv_uround)
#define nst       (cv_mem->cv_nst)
#define tn        (cv_mem->cv_tn)
#define h         (cv_mem->cv_h)
#define gamma     (cv_mem->cv_gamma)
#define gammap    (cv_mem->cv_gammap)
#define gamrat    (cv_mem->cv_gamrat)
#define ewt       (cv_mem->cv_ewt)

#define lmem      (cv_mem->cv_lmem)

#define mtype     (cvdls_mem->d_type)
#define n         (cvdls_mem->d_n)
#define ml        (cvdls_mem->d_ml)
#define mu        (cvdls_mem->d_mu)
#define smu       (cvdls_mem->d_smu)
#define jacDQ     (cvdls_mem->d_jacDQ)
#define djac      (cvdls_mem->d_djac)
#define bjac      (cvdls_mem->d_bjac)
#define M         (cvdls_mem->d_M)
#define nje       (cvdls_mem->d_nje)
#define nfeDQ     (cvdls_mem->d_nfeDQ)
#define last_flag (cvdls_mem->d_last_flag)

/* 
 * =================================================================
 * EXPORTED FUNCTIONS
 * =================================================================
 */
              
/*
 * CVDlsSetDenseJacFn specifies the dense Jacobian function.
 */
int CVDlsSetDenseJacFn(void *cvode_mem, CVDlsDenseJacFn jac)
{
  CVodeMem cv_mem;
  CVDlsMem cvdls_mem;

  /* Return immediately if cvode_mem is NULL */
  if (cvode_mem == NULL) {
    cvProcessError(NULL, CVDLS_MEM_NULL, "CVDLS", "CVDlsSetDenseJacFn", MSGD_CVMEM_NULL);
    return(CVDLS_MEM_NULL);
  }
  cv_mem = (CVodeMem) cvode_mem;

  if (lmem == NULL) {
    cvProcessError(cv_mem, CVDLS_LMEM_NULL, "CVDLS", "CVDlsSetDenseJacFn", MSGD_LMEM_NULL);
    return(CVDLS_LMEM_NULL);
  }
  cvdls_mem = (CVDlsMem) lmem;

  if (jac != NULL) {
    jacDQ = FALSE;
    djac = jac;
  } else {
    jacDQ = TRUE;
  }

  return(CVDLS_SUCCESS);
}

/*
 * CVDlsSetBandJacFn specifies the band Jacobian function.
 */
int CVDlsSetBandJacFn(void *cvode_mem, CVDlsBandJacFn jac)
{
  CVodeMem cv_mem;
  CVDlsMem cvdls_mem;

  /* Return immediately if cvode_mem is NULL */
  if (cvode_mem == NULL) {
    cvProcessError(NULL, CVDLS_MEM_NULL, "CVDLS", "CVDlsSetBandJacFn", MSGD_CVMEM_NULL);
    return(CVDLS_MEM_NULL);
  }
  cv_mem = (CVodeMem) cvode_mem;

  if (lmem == NULL) {
    cvProcessError(cv_mem, CVDLS_LMEM_NULL, "CVDLS", "CVDlsSetBandJacFn", MSGD_LMEM_NULL);
    return(CVDLS_LMEM_NULL);
  }
  cvdls_mem = (CVDlsMem) lmem;

  if (jac != NULL) {
    jacDQ = FALSE;
    bjac = jac;
  } else {
    jacDQ = TRUE;
  }

  return(CVDLS_SUCCESS);
}

/*
 * CVDlsGetWorkSpace returns the length of workspace allocated for the
 * CVDLS linear solver.
 */
int CVDlsGetWorkSpace(void *cvode_mem, long int *lenrwLS, long int *leniwLS)
{
  CVodeMem cv_mem;
  CVDlsMem cvdls_mem;

  /* Return immediately if cvode_mem is NULL */
  if (cvode_mem == NULL) {
    cvProcessError(NULL, CVDLS_MEM_NULL, "CVDLS", "CVDlsGetWorkSpace", MSGD_CVMEM_NULL);
    return(CVDLS_MEM_NULL);
  }
  cv_mem = (CVodeMem) cvode_mem;

  if (lmem == NULL) {
    cvProcessError(cv_mem, CVDLS_LMEM_NULL, "CVDLS", "CVDlsGetWorkSpace", MSGD_LMEM_NULL);
    return(CVDLS_LMEM_NULL);
  }
  cvdls_mem = (CVDlsMem) lmem;

  if (mtype == SUNDIALS_DENSE) {
    *lenrwLS = 2*n*n;
    *leniwLS = n;
  } else if (mtype == SUNDIALS_BAND) {
    *lenrwLS = n*(smu + mu + 2*ml + 2);
    *leniwLS = n;
  }

  return(CVDLS_SUCCESS);
}

/*
 * CVDlsGetNumJacEvals returns the number of Jacobian evaluations.
 */
int CVDlsGetNumJacEvals(void *cvode_mem, long int *njevals)
{
  CVodeMem cv_mem;
  CVDlsMem cvdls_mem;

  /* Return immediately if cvode_mem is NULL */
  if (cvode_mem == NULL) {
    cvProcessError(NULL, CVDLS_MEM_NULL, "CVDLS", "CVDlsGetNumJacEvals", MSGD_CVMEM_NULL);
    return(CVDLS_MEM_NULL);
  }
  cv_mem = (CVodeMem) cvode_mem;

  if (lmem == NULL) {
    cvProcessError(cv_mem, CVDLS_LMEM_NULL, "CVDLS", "CVDlsGetNumJacEvals", MSGD_LMEM_NULL);
    return(CVDLS_LMEM_NULL);
  }
  cvdls_mem = (CVDlsMem) lmem;

  *njevals = nje;

  return(CVDLS_SUCCESS);
}

/*
 * CVDlsGetNumRhsEvals returns the number of calls to the ODE function
 * needed for the DQ Jacobian approximation.
 */
int CVDlsGetNumRhsEvals(void *cvode_mem, long int *nfevalsLS)
{
  CVodeMem cv_mem;
  CVDlsMem cvdls_mem;

  /* Return immediately if cvode_mem is NULL */
  if (cvode_mem == NULL) {
    cvProcessError(NULL, CVDLS_MEM_NULL, "CVDLS", "CVDlsGetNumRhsEvals", MSGD_CVMEM_NULL);
    return(CVDLS_MEM_NULL);
  }
  cv_mem = (CVodeMem) cvode_mem;

  if (lmem == NULL) {
    cvProcessError(cv_mem, CVDLS_LMEM_NULL, "CVDLS", "CVDlsGetNumRhsEvals", MSGD_LMEM_NULL);
    return(CVDLS_LMEM_NULL);
  }
  cvdls_mem = (CVDlsMem) lmem;

  *nfevalsLS = nfeDQ;

  return(CVDLS_SUCCESS);
}

/*
 * CVDlsGetReturnFlagName returns the name associated with a CVDLS
 * return value.
 */
char *CVDlsGetReturnFlagName(long int flag)
{
  char *name;

  name = (char *)malloc(30*sizeof(char));

  switch(flag) {
  case CVDLS_SUCCESS:
    sprintf(name,"CVDLS_SUCCESS");
    break;   
  case CVDLS_MEM_NULL:
    sprintf(name,"CVDLS_MEM_NULL");
    break;
  case CVDLS_LMEM_NULL:
    sprintf(name,"CVDLS_LMEM_NULL");
    break;
  case CVDLS_ILL_INPUT:
    sprintf(name,"CVDLS_ILL_INPUT");
    break;
  case CVDLS_MEM_FAIL:
    sprintf(name,"CVDLS_MEM_FAIL");
    break;
  case CVDLS_JACFUNC_UNRECVR:
    sprintf(name,"CVDLS_JACFUNC_UNRECVR");
    break;
  case CVDLS_JACFUNC_RECVR:
    sprintf(name,"CVDLS_JACFUNC_RECVR");
    break;
  default:
    sprintf(name,"NONE");
  }

  return(name);
}

/*
 * CVDlsGetLastFlag returns the last flag set in a CVDLS function.
 */
int CVDlsGetLastFlag(void *cvode_mem, long int *flag)
{
  CVodeMem cv_mem;
  CVDlsMem cvdls_mem;

  /* Return immediately if cvode_mem is NULL */
  if (cvode_mem == NULL) {
    cvProcessError(NULL, CVDLS_MEM_NULL, "CVDLS", "CVDlsGetLastFlag", MSGD_CVMEM_NULL);
    return(CVDLS_MEM_NULL);
  }
  cv_mem = (CVodeMem) cvode_mem;

  if (lmem == NULL) {
    cvProcessError(cv_mem, CVDLS_LMEM_NULL, "CVDLS", "CVDlsGetLastFlag", MSGD_LMEM_NULL);
    return(CVDLS_LMEM_NULL);
  }
  cvdls_mem = (CVDlsMem) lmem;

  *flag = last_flag;

  return(CVDLS_SUCCESS);
}

/* 
 * =================================================================
 * DQ JACOBIAN APPROXIMATIONS
 * =================================================================
 */

/*
 * -----------------------------------------------------------------
 * cvDlsDenseDQJac 
 * -----------------------------------------------------------------
 * This routine generates a dense difference quotient approximation to
 * the Jacobian of f(t,y). It assumes that a dense matrix of type
 * DlsMat is stored column-wise, and that elements within each column
 * are contiguous. The address of the jth column of J is obtained via
 * the macro DENSE_COL and this pointer is associated with an N_Vector
 * using the N_VGetArrayPointer/N_VSetArrayPointer functions. 
 * Finally, the actual computation of the jth column of the Jacobian is 
 * done with a call to N_VLinearSum.
 * -----------------------------------------------------------------
 */ 

int cvDlsDenseDQJac(long int N, realtype t,
                    N_Vector y, N_Vector fy, 
                    DlsMat Jac, void *data,
                    N_Vector tmp1, N_Vector tmp2, N_Vector tmp3)
{
  realtype fnorm, minInc, inc, inc_inv, yjsaved, srur;
  realtype *tmp2_data, *y_data, *ewt_data;
  N_Vector ftemp, jthCol;
  long int j;
  int retval = 0;

  CVodeMem cv_mem;
  CVDlsMem cvdls_mem;

  (void)tmp3;

  /* data points to cvode_mem */
  cv_mem = (CVodeMem) data;
  cvdls_mem = (CVDlsMem) lmem;

  /* Save pointer to the array in tmp2 */
  tmp2_data = N_VGetArrayPointer(tmp2);

  /* Rename work vectors for readibility */
  ftemp = tmp1; 
  jthCol = tmp2;

  /* Obtain pointers to the data for ewt, y */
  ewt_data = N_VGetArrayPointer(ewt);
  y_data   = N_VGetArrayPointer(y);

  /* Set minimum increment based on uround and norm of f */
  srur = SUNRsqrt(uround);
  fnorm = N_VWrmsNorm(fy, ewt);
  minInc = (fnorm != ZERO) ?
           (MIN_INC_MULT * SUNRabs(h) * uround * N * fnorm) : ONE;

  for (j = 0; j < N; j++) {

    /* Generate the jth col of J(tn,y) */

    N_VSetArrayPointer(DENSE_COL(Jac,j), jthCol);

    yjsaved = y_data[j];
    inc = SUNMAX(srur*SUNRabs(yjsaved), minInc/ewt_data[j]);
    y_data[j] += inc;

    SUNDIALS_TIMED_FUNCTION(SUNDIALS_TIMER_FEVAL_JACOBIAN_GENERATION,
      retval = f(t, y, ftemp, user_data)
    );
    nfeDQ++;
    if (retval != 0) break;
    
    y_data[j] = yjsaved;

    inc_inv = ONE/inc;
    N_VLinearSum(inc_inv, ftemp, -inc_inv, fy, jthCol);

    DENSE_COL(Jac,j) = N_VGetArrayPointer(jthCol);
  }

  /* Restore original array pointer in tmp2 */
  N_VSetArrayPointer(tmp2_data, tmp2);

  return(retval);
}

/*
 * -----------------------------------------------------------------
 * cvDlsBandDQJac
 * -----------------------------------------------------------------
 * This routine generates a banded difference quotient approximation to
 * the Jacobian of f(t,y).  It assumes that a band matrix of type
 * DlsMat is stored column-wise, and that elements within each column
 * are contiguous. This makes it possible to get the address of a column
 * of J via the macro BAND_COL and to write a simple for loop to set
 * each of the elements of a column in succession.
 * -----------------------------------------------------------------
 */

int cvDlsBandDQJac(long int N, long int mupper, long int mlower,
                   realtype t, N_Vector y, N_Vector fy, 
                   DlsMat Jac, void *data,
                   N_Vector tmp1, N_Vector tmp2, N_Vector tmp3)
{
  N_Vector ftemp, ytemp;
  realtype fnorm, minInc, inc, inc_inv, srur;
  realtype *col_j, *ewt_data, *fy_data, *ftemp_data, *y_data, *ytemp_data;
  long int group, i, j, width, ngroups, i1, i2;
  int retval = 0;

  CVodeMem cv_mem;
  CVDlsMem cvdls_mem;

  (void)t; (void)tmp3;

  /* data points to cvode_mem */
  cv_mem = (CVodeMem) data;
  cvdls_mem = (CVDlsMem) lmem;

  /* Rename work vectors for use as temporary values of y and f */
  ftemp = tmp1;
  ytemp = tmp2;

  /* Obtain pointers to the data for ewt, fy, ftemp, y, ytemp */
  ewt_data   = N_VGetArrayPointer(ewt);
  fy_data    = N_VGetArrayPointer(fy);
  ftemp_data = N_VGetArrayPointer(ftemp);
  y_data     = N_VGetArrayPointer(y);
  ytemp_data = N_VGetArrayPointer(ytemp);

  /* Load ytemp with y = predicted y vector */
  N_VScale(ONE, y, ytemp);

  /* Set minimum increment based on uround and norm of f */
  srur = SUNRsqrt(uround);
  fnorm = N_VWrmsNorm(fy, ewt);
  minInc = (fnorm != ZERO) ?
           (MIN_INC_MULT * SUNRabs(h) * uround * N * fnorm) : ONE;

  /* Set bandwidth and number of column groups for band differencing */
  width = mlower + mupper + 1;
  ngroups = SUNMIN(width, N);

  /* Loop over column groups. */
  for (group=1; group <= ngroups; group++) {
    
    /* Increment all y_j in group */
    for(j=group-1; j < N; j+=width) {
      inc = SUNMAX(srur*SUNRabs(y_data[j]), minInc/ewt_data[j]);
      ytemp_data[j] += inc;
    }

    /* Evaluate f with incremented y */

    SUNDIALS_TIMED_FUNCTION(SUNDIALS_TIMER_FEVAL_JACOBIAN_GENERATION,
      retval = f(tn, ytemp, ftemp, user_data)
    );
    nfeDQ++;
    if (retval != 0) break;

    /* Restore ytemp, then form and load difference quotients */
    for (j=group-1; j < N; j+=width) {
      ytemp_data[j] = y_data[j];
      col_j = BAND_COL(Jac,j);
      inc = SUNMAX(srur*SUNRabs(y_data[j]), minInc/ewt_data[j]);
      inc_inv = ONE/inc;
      i1 = SUNMAX(0, j-mupper);
      i2 = SUNMIN(j+mlower, N-1);
      for (i=i1; i <= i2; i++)
        BAND_COL_ELEM(col_j,i,j) = inc_inv * (ftemp_data[i] - fy_data[i]);
    }
  }
  
  return(retval);
}


/*
 * -----------------------------------------------------------------
 * cvDlsBTridiagDQJac
 * -----------------------------------------------------------------
 * This routine generates a block-tridiagonal difference quotient
 * approximation to the Jacobian of f(t,y).  It assumes that a
 * block-tridiagonal matrix of type DlsMat is used.
 * -----------------------------------------------------------------
 */


int cvDlsBTridiagDQJac(long int N,realtype t,
                   N_Vector y, N_Vector fy,
                   DlsMat Jac, void *data,
                   N_Vector tmp1, N_Vector tmp2, N_Vector tmp3)
{
  N_Vector ftemp, ytemp;
  realtype fnorm, minInc, inc, inc_inv, srur;
  realtype *ewt_data, *fy_data, *ftemp_data, *y_data, *ytemp_data, *block;
  long int nb, blocksize, i, j, k, group, i1, i2;
  int retval = 0;

  CVodeMem cv_mem;
  CVDlsMem cvdls_mem;

  (void)t; (void)tmp3;

  /* data points to cvode_mem */
  cv_mem = (CVodeMem) data;
  cvdls_mem = (CVDlsMem) lmem;

  /* we do not want M to be defined here, this prevents access to Jac->M */
#undef M

  /* retrieve block-based sizes */
  nb = Jac->N;
  blocksize = Jac->M;
  /* Rename work vectors for use as temporary values of y and f */
  ftemp = tmp1;
  ytemp = tmp2;

  /* Obtain pointers to the data for ewt, fy, ftemp, y, ytemp */
  ewt_data   = N_VGetArrayPointer(ewt);
  fy_data    = N_VGetArrayPointer(fy);
  ftemp_data = N_VGetArrayPointer(ftemp);
  y_data     = N_VGetArrayPointer(y);
  ytemp_data = N_VGetArrayPointer(ytemp);

  /* Load ytemp with y = predicted y vector */
  N_VScale(ONE, y, ytemp);

  /* Set minimum increment based on uround and norm of f */
  srur = SUNRsqrt(uround);
  fnorm = N_VWrmsNorm(fy, ewt);
  minInc = (fnorm != ZERO) ?
           (MIN_INC_MULT * SUNRabs(h) * uround * N * fnorm) : ONE;

  /* TODO : special case tridiag (blocksize = 1), and blocksize = 2 */

  /* Loop over block-based column groups (=3 for any tridiag matrix) . */
  for (group=0; group<3; ++group) {
    /* Loop over all columns in block */
    for (j=0; j<blocksize; ++j) {

      /* Increment all y_i in all blocks k of the group */
      for (k=group; k<nb; k+=3) {
        /* determine global column or row i in y vector */
        i=k*blocksize+j;
        inc = SUNMAX(srur*SUNRabs(y_data[i]), minInc/ewt_data[i]);
        ytemp_data[i] += inc;
      }

      /* Evaluate f with incremented y */
      SUNDIALS_TIMED_FUNCTION(SUNDIALS_TIMER_FEVAL_JACOBIAN_GENERATION,
        retval = f(tn, ytemp, ftemp, user_data)
      );
      nfeDQ++;
      if (retval != 0) return retval;

      /* Restore ytemp, then form and load difference quotients
         k is the block-column index
      */
      for (k=group; k<nb; k+=3) {
        /* i = index of modified y value (y_data[i]) */
        i=k*blocksize+j;
        ytemp_data[i] = y_data[i];
        inc = SUNMAX(srur*SUNRabs(y_data[i]), minInc/ewt_data[i]);
        inc_inv = ONE/inc;

        /* we have to fill-in all the DQ elements in
           all rows of the blocks k-1, k and k+1 */
        if (k>0) {
          /* set values in upper band */
          block = BTRIDIAG_UPPER(Jac, k-1);
          /* i2 = index of first equation affected by y_data[i] */
          i2 = (k-1)*blocksize;
          for (i1=0; i1<blocksize; ++i1, ++i2) {
            BTRIDIAG_BLOCK_ELEM(Jac, block, i1, j) = inc_inv * (ftemp_data[i2] - fy_data[i2]);
          }
        }

        /*! special case, block L[0] = J_{0,2} */
        if (k==2) {
          /* set values in first block in lower band */
          block = BTRIDIAG_LOWER(Jac, 0);
          i2 = 0;
          for (i1=0; i1<blocksize; ++i1, ++i2) {
            BTRIDIAG_BLOCK_ELEM(Jac, block, i1, j) = inc_inv * (ftemp_data[i2] - fy_data[i2]);
          }
        }

        /* set values in main band */
        block = BTRIDIAG_MAIN(Jac, k);
        i2 = k*blocksize;
        for (i1=0; i1<blocksize; ++i1, ++i2) {
          BTRIDIAG_BLOCK_ELEM(Jac, block, i1, j) = inc_inv * (ftemp_data[i2] - fy_data[i2]);
        }

        if (k<nb-1) {
          /* set values in lower band */
          block = BTRIDIAG_LOWER(Jac, k+1);
          i2 = (k+1)*blocksize;
          for (i1=0; i1<blocksize; ++i1, ++i2) {
            BTRIDIAG_BLOCK_ELEM(Jac, block, i1, j) = inc_inv * (ftemp_data[i2] - fy_data[i2]);
          }
        }

        /*! special case, block U[n] = J_{n-1,n-3} */
        if (k==nb-3) {
          /* set values in last block in upper band */
          block = BTRIDIAG_UPPER(Jac, nb-1);
          i2 = (nb-1)*blocksize;
          for (i1=0; i1<blocksize; ++i1, ++i2) {
            BTRIDIAG_BLOCK_ELEM(Jac, block, i1, j) = inc_inv * (ftemp_data[i2] - fy_data[i2]);
          }
        }

      } /* for (k=group; k<nb; k+=3) */
    } /* for (j=0; j<blocksize; ++j) */
  } /* for (group=0; group<3; ++group) */

  /* PrintMat(Jac); */

  return(retval);
}

int cvDlsInitializeCounters(CVDlsMem cvdls_mem)
{
  cvdls_mem->d_nje   = 0;
  cvdls_mem->d_nfeDQ = 0;
  cvdls_mem->d_nstlj = 0;
  return(0);
}


