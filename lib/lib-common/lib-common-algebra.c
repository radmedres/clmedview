/*
 * Copyright (C) 2015 Marc Geerlings <m.geerlings@mumc.nl>
 *
 * This file is part of clmedview.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "lib-common-algebra.h"
#include "lib-common-debug.h"

#include <stdlib.h>
#include <math.h>

/*                                                                                                    */
/*                                                                                                    */
/* LOCAL FUNCTIONS                                                                                    */
/*                                                                                                    */
/*                                                                                                    */

td_Matrix3x3 nifti_mat33_inverse( td_Matrix3x3 R )   /* inverse of 3x3 matrix */
{
   double r11,r12,r13,r21,r22,r23,r31,r32,r33 , deti ;
   td_Matrix3x3 Q ;
                                                       /*  INPUT MATRIX:  */
   r11 = R.d_Matrix[0][0]; r12 = R.d_Matrix[0][1]; r13 = R.d_Matrix[0][2];  /* [ r11 r12 r13 ] */
   r21 = R.d_Matrix[1][0]; r22 = R.d_Matrix[1][1]; r23 = R.d_Matrix[1][2];  /* [ r21 r22 r23 ] */
   r31 = R.d_Matrix[2][0]; r32 = R.d_Matrix[2][1]; r33 = R.d_Matrix[2][2];  /* [ r31 r32 r33 ] */

   deti = r11*r22*r33-r11*r32*r23-r21*r12*r33
         +r21*r32*r13+r31*r12*r23-r31*r22*r13 ;

   if( deti != 0.0l ) deti = 1.0l / deti ;

   Q.d_Matrix[0][0] = deti*( r22*r33-r32*r23) ;
   Q.d_Matrix[0][1] = deti*(-r12*r33+r32*r13) ;
   Q.d_Matrix[0][2] = deti*( r12*r23-r22*r13) ;

   Q.d_Matrix[1][0] = deti*(-r21*r33+r31*r23) ;
   Q.d_Matrix[1][1] = deti*( r11*r33-r31*r13) ;
   Q.d_Matrix[1][2] = deti*(-r11*r23+r21*r13) ;

   Q.d_Matrix[2][0] = deti*( r21*r32-r31*r22) ;
   Q.d_Matrix[2][1] = deti*(-r11*r32+r31*r12) ;
   Q.d_Matrix[2][2] = deti*( r11*r22-r21*r12) ;

   return Q ;
}

/*----------------------------------------------------------------------*/
/*! compute the determinant of a 3x3 matrix
*//*--------------------------------------------------------------------*/
float nifti_mat33_determ( td_Matrix3x3 R )   /* determinant of 3x3 matrix */
{
   double r11,r12,r13,r21,r22,r23,r31,r32,r33 ;
                                                       /*  INPUT MATRIX:  */
   r11 = R.d_Matrix[0][0]; r12 = R.d_Matrix[0][1]; r13 = R.d_Matrix[0][2];  /* [ r11 r12 r13 ] */
   r21 = R.d_Matrix[1][0]; r22 = R.d_Matrix[1][1]; r23 = R.d_Matrix[1][2];  /* [ r21 r22 r23 ] */
   r31 = R.d_Matrix[2][0]; r32 = R.d_Matrix[2][1]; r33 = R.d_Matrix[2][2];  /* [ r31 r32 r33 ] */

   return r11*r22*r33-r11*r32*r23-r21*r12*r33
         +r21*r32*r13+r31*r12*r23-r31*r22*r13 ;
}

/*----------------------------------------------------------------------*/
/*! compute the max row norm of a 3x3 matrix
*//*--------------------------------------------------------------------*/
float nifti_mat33_rownorm( td_Matrix3x3 A )  /* max row norm of 3x3 matrix */
{
   float r1,r2,r3 ;

   r1 = fabs(A.d_Matrix[0][0])+fabs(A.d_Matrix[0][1])+fabs(A.d_Matrix[0][2]) ;
   r2 = fabs(A.d_Matrix[1][0])+fabs(A.d_Matrix[1][1])+fabs(A.d_Matrix[1][2]) ;
   r3 = fabs(A.d_Matrix[2][0])+fabs(A.d_Matrix[2][1])+fabs(A.d_Matrix[2][2]) ;
   if( r1 < r2 ) r1 = r2 ;
   if( r1 < r3 ) r1 = r3 ;
   return r1 ;
}

/*----------------------------------------------------------------------*/
/*! compute the max column norm of a 3x3 matrix
*//*--------------------------------------------------------------------*/
float nifti_mat33_colnorm( td_Matrix3x3 A )  /* max column norm of 3x3 matrix */
{
   float r1,r2,r3 ;

   r1 = fabs(A.d_Matrix[0][0])+fabs(A.d_Matrix[1][0])+fabs(A.d_Matrix[2][0]) ;
   r2 = fabs(A.d_Matrix[0][1])+fabs(A.d_Matrix[1][1])+fabs(A.d_Matrix[2][1]) ;
   r3 = fabs(A.d_Matrix[0][2])+fabs(A.d_Matrix[1][2])+fabs(A.d_Matrix[2][2]) ;
   if( r1 < r2 ) r1 = r2 ;
   if( r1 < r3 ) r1 = r3 ;
   return r1 ;
}

/*---------------------------------------------------------------------------*/
/*! polar decomposition of a 3x3 matrix

   This finds the closest orthogonal matrix to input A
   (in both the Frobenius and L2 norms).

   Algorithm is that from NJ Higham, SIAM J Sci Stat Comput, 7:1160-1174.
*//*-------------------------------------------------------------------------*/
td_Matrix3x3 nifti_mat33_polar( td_Matrix3x3 A )
{
   td_Matrix3x3 X , Y , Z ;
   float alp,bet,gam,gmi , dif=1.0 ;
   int k=0 ;

   X = A ;

   /* force matrix to be nonsingular */

   gam = nifti_mat33_determ(X) ;
   while( gam == 0.0 ){        /* perturb matrix */
     gam = 0.00001 * ( 0.001 + nifti_mat33_rownorm(X) ) ;
     X.d_Matrix[0][0] += gam ; X.d_Matrix[1][1] += gam ; X.d_Matrix[2][2] += gam ;
     gam = nifti_mat33_determ(X) ;
   }

   while(1){
     Y = nifti_mat33_inverse(X) ;
     if( dif > 0.3 ){     /* far from convergence */
       alp = sqrt( nifti_mat33_rownorm(X) * nifti_mat33_colnorm(X) ) ;
       bet = sqrt( nifti_mat33_rownorm(Y) * nifti_mat33_colnorm(Y) ) ;
       gam = sqrt( bet / alp ) ;
       gmi = 1.0 / gam ;
     } else {
       gam = gmi = 1.0 ;  /* close to convergence */
     }
     Z.d_Matrix[0][0] = 0.5 * ( gam*X.d_Matrix[0][0] + gmi*Y.d_Matrix[0][0] ) ;
     Z.d_Matrix[0][1] = 0.5 * ( gam*X.d_Matrix[0][1] + gmi*Y.d_Matrix[1][0] ) ;
     Z.d_Matrix[0][2] = 0.5 * ( gam*X.d_Matrix[0][2] + gmi*Y.d_Matrix[2][0] ) ;
     Z.d_Matrix[1][0] = 0.5 * ( gam*X.d_Matrix[1][0] + gmi*Y.d_Matrix[0][1] ) ;
     Z.d_Matrix[1][1] = 0.5 * ( gam*X.d_Matrix[1][1] + gmi*Y.d_Matrix[1][1] ) ;
     Z.d_Matrix[1][2] = 0.5 * ( gam*X.d_Matrix[1][2] + gmi*Y.d_Matrix[2][1] ) ;
     Z.d_Matrix[2][0] = 0.5 * ( gam*X.d_Matrix[2][0] + gmi*Y.d_Matrix[0][2] ) ;
     Z.d_Matrix[2][1] = 0.5 * ( gam*X.d_Matrix[2][1] + gmi*Y.d_Matrix[1][2] ) ;
     Z.d_Matrix[2][2] = 0.5 * ( gam*X.d_Matrix[2][2] + gmi*Y.d_Matrix[2][2] ) ;

     dif = fabs(Z.d_Matrix[0][0]-X.d_Matrix[0][0])+fabs(Z.d_Matrix[0][1]-X.d_Matrix[0][1])
          +fabs(Z.d_Matrix[0][2]-X.d_Matrix[0][2])+fabs(Z.d_Matrix[1][0]-X.d_Matrix[1][0])
          +fabs(Z.d_Matrix[1][1]-X.d_Matrix[1][1])+fabs(Z.d_Matrix[1][2]-X.d_Matrix[1][2])
          +fabs(Z.d_Matrix[2][0]-X.d_Matrix[2][0])+fabs(Z.d_Matrix[2][1]-X.d_Matrix[2][1])
          +fabs(Z.d_Matrix[2][2]-X.d_Matrix[2][2])                          ;

     k = k+1 ;
     if( k > 100 || dif < 3.e-6 ) break ;  /* convergence or exhaustion */
     X = Z ;
   }

   return Z ;
}


/*                                                                                                    */
/*                                                                                                    */
/* GLOBAL FUNCTIONS                                                                                   */
/*                                                                                                    */
/*                                                                                                    */
td_Matrix4x4 tda_memory_quaternion_to_matrix(ts_Quaternion *ps_Source, ts_Quaternion *ps_SourceOffset, double d_Qfac)
{
  td_Matrix4x4 td_Rotation;
  double d_Rotation;

  d_Rotation = 1.0 - (ps_Source->I * ps_Source->I + ps_Source->J * ps_Source->J + ps_Source->K * ps_Source->K);

  if (d_Rotation == 0 )
  {
    /* Special case according to niftii standard !?!*/
    /* There should be a 180 degree rotation        */
    d_Rotation = 1 / sqrt((ps_Source->I * ps_Source->I +  ps_Source->J * ps_Source->J +  ps_Source->K * ps_Source->K));
    ps_Source->I *= d_Rotation;
    ps_Source->J *= d_Rotation;
    ps_Source->K *= d_Rotation;
    ps_Source->W = 0;
  }
  else
  {
    ps_Source->W = sqrt(d_Rotation);
  }

  td_Rotation.d_Matrix[0][0] =     (ps_Source->W * ps_Source->W + ps_Source->I * ps_Source->I - ps_Source->J * ps_Source->J - ps_Source->K * ps_Source->K);
  td_Rotation.d_Matrix[0][1] = 2 * (ps_Source->I * ps_Source->J + ps_Source->W * ps_Source->K);
  td_Rotation.d_Matrix[0][2] = 2 * (ps_Source->I * ps_Source->K - ps_Source->W * ps_Source->J) * d_Qfac;

  td_Rotation.d_Matrix[1][0] = 2 * (ps_Source->I * ps_Source->J - ps_Source->W * ps_Source->K);
  td_Rotation.d_Matrix[1][1] =     (ps_Source->W * ps_Source->W - ps_Source->I * ps_Source->I + ps_Source->J * ps_Source->J - ps_Source->K * ps_Source->K);
  td_Rotation.d_Matrix[1][2] = 2 * (ps_Source->J * ps_Source->K + ps_Source->W * ps_Source->I)* d_Qfac;

  td_Rotation.d_Matrix[2][0] = 2 * (ps_Source->I * ps_Source->K + ps_Source->W * ps_Source->J);
  td_Rotation.d_Matrix[2][1] = 2 * (ps_Source->J * ps_Source->K - ps_Source->W * ps_Source->I);
  td_Rotation.d_Matrix[2][2] =     (ps_Source->W * ps_Source->W - ps_Source->I * ps_Source->I - ps_Source->J * ps_Source->J + ps_Source->K * ps_Source->K) * d_Qfac;


/*
  td_Rotation.d_Matrix[0][0] =     (ps_Source->W * ps_Source->W + ps_Source->I * ps_Source->I - ps_Source->J * ps_Source->J - ps_Source->K * ps_Source->K);
  td_Rotation.d_Matrix[0][1] = 2 * (ps_Source->I * ps_Source->J - ps_Source->W * ps_Source->K);
  td_Rotation.d_Matrix[0][2] = 2 * (ps_Source->I * ps_Source->K + ps_Source->W * ps_Source->J);

  td_Rotation.d_Matrix[1][0] = 2 * (ps_Source->I * ps_Source->J + ps_Source->W * ps_Source->K);
  td_Rotation.d_Matrix[1][1] =     (ps_Source->W * ps_Source->W - ps_Source->I * ps_Source->I + ps_Source->J * ps_Source->J - ps_Source->K * ps_Source->K);
  td_Rotation.d_Matrix[1][2] = 2 * (ps_Source->J * ps_Source->K - ps_Source->W * ps_Source->I);

  td_Rotation.d_Matrix[2][0] = 2 * (ps_Source->I * ps_Source->K - ps_Source->W * ps_Source->J) * d_Qfac;
  td_Rotation.d_Matrix[2][1] = 2 * (ps_Source->J * ps_Source->K + ps_Source->W * ps_Source->I)* d_Qfac;
  td_Rotation.d_Matrix[2][2] =     (ps_Source->W * ps_Source->W - ps_Source->I * ps_Source->I - ps_Source->J * ps_Source->J + ps_Source->K * ps_Source->K) * d_Qfac;
*/
  td_Rotation.d_Matrix[3][0] = ps_SourceOffset->I;
  td_Rotation.d_Matrix[3][1] = ps_SourceOffset->J;
  td_Rotation.d_Matrix[3][2] = ps_SourceOffset->K;

  td_Rotation.d_Matrix[0][3] = 0;
  td_Rotation.d_Matrix[1][3] = 0;
  td_Rotation.d_Matrix[2][3] = 0;
  td_Rotation.d_Matrix[3][3] = 1;


  return td_Rotation;
}

ts_Quaternion ts_memory_matrix_to_quaternion(td_Matrix4x4 *pt_Matrix, double *pd_Qfac)
{
  ts_Quaternion ts_Quat;
  double r11,r12,r13 , r21,r22,r23 , r31,r32,r33 ;

  double xd,yd,zd , a,b,c,d ;

  td_Matrix3x3 P,Q ;

  /* [ r11 r12 r13 v1 ] */
  /* [ r21 r22 r23 v2 ] */
  /* [ r31 r32 r33 v3 ] */
  /* [  0   0   0   1 ] */

  /* load 3x3 matrix into local variables */

  r11 = pt_Matrix->d_Matrix[0][0] ; r12 = pt_Matrix->d_Matrix[1][0] ; r13 = pt_Matrix->d_Matrix[2][0] ;
  r21 = pt_Matrix->d_Matrix[0][1] ; r22 = pt_Matrix->d_Matrix[1][1] ; r23 = pt_Matrix->d_Matrix[2][1] ;
  r31 = pt_Matrix->d_Matrix[0][2] ; r32 = pt_Matrix->d_Matrix[1][2] ; r33 = pt_Matrix->d_Matrix[2][2] ;

  /* compute lengths of each column; these determine grid spacings  */

  xd = sqrt( r11*r11 + r21*r21 + r31*r31 ) ;
  yd = sqrt( r12*r12 + r22*r22 + r32*r32 ) ;
  zd = sqrt( r13*r13 + r23*r23 + r33*r33 ) ;

  /* if a column length is zero, patch the trouble */

  if( xd == 0.0l ){ r11 = 1.0l ; r21 = r31 = 0.0l ; xd = 1.0l ; }
  if( yd == 0.0l ){ r22 = 1.0l ; r12 = r32 = 0.0l ; yd = 1.0l ; }
  if( zd == 0.0l ){ r33 = 1.0l ; r13 = r23 = 0.0l ; zd = 1.0l ; }

   /* normalize the columns */

   r11 /= xd ; r21 /= xd ; r31 /= xd ;
   r12 /= yd ; r22 /= yd ; r32 /= yd ;
   r13 /= zd ; r23 /= zd ; r33 /= zd ;

   /* At this point, the matrix has normal columns, but we have to allow
      for the fact that the hideous user may not have given us a matrix
      with orthogonal columns.

      So, now find the orthogonal matrix closest to the current matrix.

      One reason for using the polar decomposition to get this
      orthogonal matrix, rather than just directly orthogonalizing
      the columns, is so that inputting the inverse matrix to R
      will result in the inverse orthogonal matrix at this point.
      If we just orthogonalized the columns, this wouldn't necessarily hold. */

//   Q.d_Matrix[0][0] = r11 ; Q.d_Matrix[1][0] = r12 ; Q.d_Matrix[2][0] = r13 ; /* load Q */
//   Q.d_Matrix[0][1] = r21 ; Q.d_Matrix[1][1] = r22 ; Q.d_Matrix[2][1] = r23 ;
//   Q.d_Matrix[0][2] = r31 ; Q.d_Matrix[1][2] = r32 ; Q.d_Matrix[2][2] = r33 ;

//   P = nifti_mat33_polar(Q) ;  /* P is orthog matrix closest to Q */

//   r11 = P.d_Matrix[0][0] ; r12 = P.d_Matrix[1][0] ; r13 = P.d_Matrix[2][0] ; /* unload */
//   r21 = P.d_Matrix[0][1] ; r22 = P.d_Matrix[1][1] ; r23 = P.d_Matrix[2][1] ;
//   r31 = P.d_Matrix[0][2] ; r32 = P.d_Matrix[1][2] ; r33 = P.d_Matrix[2][2] ;

   /*                            [ r11 r12 r13 ]               */
   /* at this point, the matrix  [ r21 r22 r23 ] is orthogonal */
   /*                            [ r31 r32 r33 ]               */

   /* compute the determinant to determine if it is proper */

   zd = r11*r22*r33-r11*r32*r23-r21*r12*r33
       +r21*r32*r13+r31*r12*r23-r31*r22*r13 ;  /* should be -1 or 1 */

   if( zd <= 0 )
    {                  /* improper ==> flip 3rd column */
      *pd_Qfac = -1;
  //     ASSIF(qfac,-1.0) ;
     r13 = -r13 ; r23 = -r23 ; r33 = -r33 ;
   }
   else
   {
     *pd_Qfac = 1;
   }


   /* now, compute quaternion parameters */

  a = r11 + r22 + r33 + 1.0l ;

  if( a > 0.5l )
  {                /* simplest case */
    a = 0.5l * sqrt(a) ;
    b = 0.25l * (r32-r23) / a ;
    c = 0.25l * (r13-r31) / a ;
    d = 0.25l * (r21-r12) / a ;
  }
  else
  {                       /* trickier case */
    xd = 1.0 + r11 - (r22+r33) ;  /* 4*b*b */
    yd = 1.0 + r22 - (r11+r33) ;  /* 4*c*c */
    zd = 1.0 + r33 - (r11+r22) ;  /* 4*d*d */

    if( xd > 1.0 )
    {
      b = 0.5l * sqrt(xd) ;
      c = 0.25l* (r12+r21) / b ;
      d = 0.25l* (r13+r31) / b ;
      a = 0.25l* (r32-r23) / b ;
    }
    else if( yd > 1.0 )
    {
      c = 0.5l * sqrt(yd) ;
      b = 0.25l* (r12+r21) / c ;
      d = 0.25l* (r23+r32) / c ;
      a = 0.25l* (r13-r31) / c ;
    }
    else
    {
      d = 0.5l * sqrt(zd) ;
      b = 0.25l* (r13+r31) / d ;
      c = 0.25l* (r23+r32) / d ;
      a = 0.25l* (r21-r12) / d ;
    }
    if( a < 0.0l )
    {
      b=-b;
      c=-c;
      d=-d;
      a=-a;
    }
  }

  //   ASSIF(qb,b) ; ASSIF(qc,c) ; ASSIF(qd,d) ;
  ts_Quat.W = a;
  ts_Quat.I = b;
  ts_Quat.J = c;
  ts_Quat.K = d;

  return ts_Quat;
}


td_Matrix4x4 tda_memory_quaternion_inverse_matrix(td_Matrix4x4 *ps_Matrix)
{
  double d_Determinant;
  double r11,r12,r13,r21,r22,r23,r31,r32,r33,v1,v2,v3;
  td_Matrix4x4 td_Inverse;

  /* [ r11 r12 r13 v1 ] */
  /* [ r21 r22 r23 v2 ] */
  /* [ r31 r32 r33 v3 ] */
  /* [  0   0   0   1 ] */

  r11 = ps_Matrix->d_Matrix[0][0];
  r12 = ps_Matrix->d_Matrix[1][0];
  r13 = ps_Matrix->d_Matrix[2][0];
  v1  = ps_Matrix->d_Matrix[3][0];

  r21 = ps_Matrix->d_Matrix[0][1];
  r22 = ps_Matrix->d_Matrix[1][1];
  r23 = ps_Matrix->d_Matrix[2][1];
  v2  = ps_Matrix->d_Matrix[3][1];

  r31 = ps_Matrix->d_Matrix[0][2];
  r32 = ps_Matrix->d_Matrix[1][2];
  r33 = ps_Matrix->d_Matrix[2][2];
  v3  = ps_Matrix->d_Matrix[3][2];

  d_Determinant = r11*r22*r33 - r11*r32*r23 - r21*r12*r33 + r21*r32*r13 + r31*r12*r23 - r31*r22*r13 ;
  d_Determinant = (d_Determinant == 0 ) ? d_Determinant : 1 / d_Determinant;


  td_Inverse.d_Matrix[0][0] = d_Determinant * ( r22*r33-r32*r23);
  td_Inverse.d_Matrix[1][0] = d_Determinant * (-r12*r33+r32*r13);
  td_Inverse.d_Matrix[2][0] = d_Determinant * ( r12*r23-r22*r13);
  td_Inverse.d_Matrix[3][0] = d_Determinant * (-r12*r23*v3 + r12*v2*r33 + r22*r13*v3 - r22*v1*r33 - r32*r13*v2 + r32*v1*r23);

  td_Inverse.d_Matrix[0][1] = d_Determinant * (-r21*r33+r31*r23);
  td_Inverse.d_Matrix[1][1] = d_Determinant * ( r11*r33-r31*r13);
  td_Inverse.d_Matrix[2][1] = d_Determinant * (-r11*r23+r21*r13);
  td_Inverse.d_Matrix[3][1] = d_Determinant * ( r11*r23*v3 - r11*v2*r33 - r21*r13*v3 + r21*v1*r33 + r31*r13*v2 - r31*v1*r23);

  td_Inverse.d_Matrix[0][2] = d_Determinant * ( r21*r32-r31*r22);
  td_Inverse.d_Matrix[1][2] = d_Determinant * (-r11*r32+r31*r12);
  td_Inverse.d_Matrix[2][2] = d_Determinant * ( r11*r22-r21*r12);
  td_Inverse.d_Matrix[3][2] = d_Determinant * (-r11*r22*v3 + r11*r32*v2 + r21*r12*v3 - r21*r32*v1 - r31*r12*v2 + r31*r22*v1);

  td_Inverse.d_Matrix[0][3] = 0;
  td_Inverse.d_Matrix[1][3] = 0;
  td_Inverse.d_Matrix[2][3] = 0;
  td_Inverse.d_Matrix[3][3] = (d_Determinant == 0) ? 0 : 1 ;

  return td_Inverse;
}

Vector3D ts_memory_matrix_multiply4x4(td_Matrix4x4 *ps_Matrix, Vector3D *ps_Vector)
{
  Vector3D ts_MultiplyVector;

  ts_MultiplyVector.x = ps_Vector->x  * ps_Matrix->d_Matrix[0][0] +
                        ps_Vector->y  * ps_Matrix->d_Matrix[0][1] +
                        ps_Vector->z  * ps_Matrix->d_Matrix[0][2] +
                        ps_Matrix->d_Matrix[0][3];

  ts_MultiplyVector.y = ps_Vector->x  * ps_Matrix->d_Matrix[1][0] +
                        ps_Vector->y  * ps_Matrix->d_Matrix[1][1] +
                        ps_Vector->z  * ps_Matrix->d_Matrix[1][2] +
                        ps_Matrix->d_Matrix[1][3];

  ts_MultiplyVector.z = ps_Vector->x  * ps_Matrix->d_Matrix[2][0] +
                        ps_Vector->y  * ps_Matrix->d_Matrix[2][1] +
                        ps_Vector->z  * ps_Matrix->d_Matrix[2][2] +
                        ps_Matrix->d_Matrix[2][3];

  return ts_MultiplyVector;
}





