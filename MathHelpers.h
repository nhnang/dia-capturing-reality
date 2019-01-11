//
// Capturing Reality s.r.o.
//
// This file 'MathHelpers.h' is a part of the RC Engine Samples. You may use it freely.
//

#pragma once

inline double Vec3Len( __in_ecount(3) const double* v )
{
    return sqrt( v[0]*v[0] + v[1]*v[1] + v[2]*v[2] );
}

inline void Vec3Scale( __inout_ecount(3) double* v, __in const double s )
{
    v[0] *= s;
    v[1] *= s;
    v[2] *= s;
}

inline void Vec3Normalize( __inout_ecount(3) double* v )
{
    double rcplen = 1.0 / Vec3Len( v );
    v[0] *= rcplen;
    v[1] *= rcplen;
    v[2] *= rcplen;
}

inline void Vec3Cross( __out_ecount(3) double *c, __in_ecount(3) const double *a, __in_ecount(3) const double *b )
{
    // c = a x b
	c[0] = ( a[1]*b[2] - a[2]*b[1] );
	c[1] = ( a[2]*b[0] - a[0]*b[2] );
	c[2] = ( a[0]*b[1] - a[1]*b[0] );
}

inline void Mat33SetIdentity( __in_ecount(16) double *pMatrix )
{
	ZeroMemory( pMatrix, 16 * sizeof( double ) );
	pMatrix[0]  = 1;
	pMatrix[5]  = 1;
	pMatrix[10] = 1;
	pMatrix[15] = 1;
};

inline void Mat33Transpose( __in_ecount(9) const double *h, __out_ecount(9) double *res )
{
	//  0 1 2
	//  3 4 5
	//  6 7 8

	res[0] = h[0];
	res[3] = h[1];
	res[6] = h[2];

	res[1] = h[3];
	res[4] = h[4];
	res[7] = h[5];

	res[2] = h[6];
	res[5] = h[7];
	res[8] = h[8];
}

inline double Mat33Det( __in_ecount(9) const double* M )
{
	return M[0]*( M[4]*M[8] - M[5]*M[7] ) - M[1]*( M[3]*M[8] - M[5]*M[6] ) + M[2]*( M[3]*M[7] - M[4]*M[6] );
}

inline void MulV3M33( const double *x, const double *M, __out_ecount(3) double *b)
{
	b[0] = (M[0]*x[0] + M[3]*x[1] + M[6]*x[2]);
	b[1] = (M[1]*x[0] + M[4]*x[1] + M[7]*x[2]);
	b[2] = (M[2]*x[0] + M[5]*x[1] + M[8]*x[2]);
}

inline void MulM33V3( __in_ecount(9) const double *M, __in_ecount(3) const double *x, __out_ecount(3) double *b)
{
	b[0] = (M[0]*x[0] + M[1]*x[1] + M[2]*x[2]);
	b[1] = (M[3]*x[0] + M[4]*x[1] + M[5]*x[2]);
	b[2] = (M[6]*x[0] + M[7]*x[1] + M[8]*x[2]);
}

inline void Mat33Mul( __in_ecount(9) const double *A, __in_ecount(9) const double *B, __out_ecount(9) double *o)
{
	o[0] = A[0]*B[0] + A[1]*B[3] + A[2]*B[6];
	o[1] = A[0]*B[1] + A[1]*B[4] + A[2]*B[7];
	o[2] = A[0]*B[2] + A[1]*B[5] + A[2]*B[8];

	o[3] = A[3]*B[0] + A[4]*B[3] + A[5]*B[6];
	o[4] = A[3]*B[1] + A[4]*B[4] + A[5]*B[7];
	o[5] = A[3]*B[2] + A[4]*B[5] + A[5]*B[8];

	o[6] = A[6]*B[0] + A[7]*B[3] + A[8]*B[6];
	o[7] = A[6]*B[1] + A[7]*B[4] + A[8]*B[7];
	o[8] = A[6]*B[2] + A[7]*B[5] + A[8]*B[8];
}

// M*(x|1) = b
inline void MulM34V3( __in_ecount(12) const double *M, __in_ecount(3) const double *x, __out_ecount(3) double *b )
{
	b[0] = (M[0]*x[0] + M[1]*x[1] + M[2]*x[2] + M[3]);
	b[1] = (M[4]*x[0] + M[5]*x[1] + M[6]*x[2] + M[7]);
	b[2] = (M[8]*x[0] + M[9]*x[1] + M[10]*x[2] + M[11]);
}

inline void InvertResidualTransform( __in CapturingReality::CoordinateSystemResidual *pSrc, __out CapturingReality::CoordinateSystemResidual *pInv)
{
    Mat33Transpose(pSrc->R, pInv->R);
    pInv->s = 1.0 / pSrc->s;
    double t[3];
    MulM33V3(pInv->R, pSrc->t, t);
    pInv->t[0] = -t[0] * pInv->s;
    pInv->t[1] = -t[1] * pInv->s;
    pInv->t[2] = -t[2] * pInv->s;
}

inline CapturingReality::CoordinateSystemResidual TransformResidualTransform( __in const CapturingReality::CoordinateSystemResidual*pA, __in const CapturingReality::CoordinateSystemResidual*pB )
{
    CapturingReality::CoordinateSystemResidual r;

    r.s = pA->s * pB->s;
    Mat33Mul(pA->R, pB->R, r.R);

    double t[3];
    MulM33V3(pA->R, pB->t, t);
    r.t[0] = pA->t[0] + pA->s * t[0];
    r.t[1] = pA->t[1] + pA->s * t[1];
    r.t[2] = pA->t[2] + pA->s * t[2];

    return r;
}

inline CapturingReality::CoordinateSystemResidual GetModelToActualTransform( __in CapturingReality::CoordinateSystemResidual *pRootToModel, __in CapturingReality::CoordinateSystemResidual *pRootToActual )
{
	CapturingReality::CoordinateSystemResidual modelToRoot;
	InvertResidualTransform( pRootToModel, &modelToRoot );
	return TransformResidualTransform( pRootToActual, &modelToRoot ); 
};
