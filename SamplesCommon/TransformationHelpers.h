//
// Capturing Reality s.r.o.
//
// This file 'TransformationHelpers.h' is a part of the RC Engine Samples. You may use it freely.
//

#pragma once

using namespace CapturingReality;

const double DEG_TO_RAD = (3.1415926535897932384626433832795 / 180);


__forceinline double cspLength( __in CoordinateSystemPoint *pV )
{
	return sqrt( pV->x*pV->x + pV->y*pV->y + pV->z*pV->z );
};

__forceinline void cspSub( __in CoordinateSystemPoint *pA, __in CoordinateSystemPoint *pB, __out CoordinateSystemPoint *pOut )
{
	pOut->x = pB->x - pA->x;
	pOut->y = pB->y - pA->y;
	pOut->z = pB->z - pA->z;
};

__forceinline double Mat44Inv( __in_ecount(16) const double *A, __out_ecount(16) double *iA )
{
    // using the Laplace expansion theorem for 4x4 matrix still outperforms GJ elimination in flops

    // calculate 2x2 sub-determinants
    double s0 = A[0] * A[5] - A[4] * A[1];
    double s1 = A[0] * A[6] - A[4] * A[2];
    double s2 = A[0] * A[7] - A[4] * A[3];
    double s3 = A[1] * A[6] - A[5] * A[2];
    double s4 = A[1] * A[7] - A[5] * A[3];
    double s5 = A[2] * A[7] - A[6] * A[3];
         
    double c5 = A[10] * A[15] - A[14] * A[11];
    double c4 = A[9] * A[15] - A[13] * A[11];
    double c3 = A[9] * A[14] - A[13] * A[10];
    double c2 = A[8] * A[15] - A[12] * A[11];
    double c1 = A[8] * A[14] - A[12] * A[10];
    double c0 = A[8] * A[13] - A[12] * A[9];

    double det = (s0 * c5 - s1 * c4 + s2 * c3 + s3 * c2 - s4 * c1 + s5 * c0);
    if ( det == 0 )
    {
        return det;
    }

    double recDet = 1 / det;

    iA[0] = (A[5] * c5 - A[6] * c4 + A[7] * c3) * recDet;
    iA[1] = (-A[1] * c5 + A[2] * c4 - A[3] * c3) * recDet;
    iA[2] = (A[13] * s5 - A[14] * s4 + A[15] * s3) * recDet;
    iA[3] = (-A[9] * s5 + A[10] * s4 - A[11] * s3) * recDet;
    
    iA[4] = (-A[4] * c5 + A[6] * c2 - A[7] * c1) * recDet;
    iA[5] = (A[0] * c5 - A[2] * c2 + A[3] * c1) * recDet;
    iA[6] = (-A[12] * s5 + A[14] * s2 - A[15] * s1) * recDet;
    iA[7] = (A[8] * s5 - A[10] * s2 + A[11] * s1) * recDet;
    
    iA[8] = (A[4] * c4 - A[5] * c2 + A[7] * c0) * recDet;
    iA[9] = (-A[0] * c4 + A[1] * c2 - A[3] * c0) * recDet;
    iA[10] = (A[12] * s4 - A[13] * s2 + A[15] * s0) * recDet;
    iA[11] = (-A[8] * s4 + A[9] * s2 - A[11] * s0) * recDet;
    
    iA[12] = (-A[4] * c3 + A[5] * c1 - A[6] * c0) * recDet;
    iA[13] = (A[0] * c3 - A[1] * c1 + A[2] * c0) * recDet;
    iA[14] = (-A[12] * s3 + A[13] * s1 - A[14] * s0) * recDet;
    iA[15] = (A[8] * s3 - A[9] * s1 + A[10] * s0) * recDet;

    return det;
}

__forceinline void TransformPoint( __in CoordinateSystemPoint *pPin, __in_ecount(16) const double *M, __out CoordinateSystemPoint *pPout )
{
	double x = M[ 0]*pPin->x + M[ 1]*pPin->y + M[2] *pPin->z + M[ 3];
	double y = M[ 4]*pPin->x + M[ 5]*pPin->y + M[6] *pPin->z + M[ 7];
	double z = M[ 8]*pPin->x + M[ 9]*pPin->y + M[10]*pPin->z + M[11];
	double w = M[12]*pPin->x + M[13]*pPin->y + M[14]*pPin->z + M[15];

	_ASSERT( w != 0.0 );

	pPout->x = x / w;
	pPout->y = y / w;
	pPout->z = z / w;
};

__forceinline void GetCoordinateSystemTransform( __in CoordinateSystemAnchor *pSfmReconstructionAnchor, __in CoordinateSystemGroundPlane *pSfmReconstructionGroundPlane, __out_ecount(16) double *pTrn )
{
	pTrn[0] = pSfmReconstructionGroundPlane->R[0];
	pTrn[4] = pSfmReconstructionGroundPlane->R[1];
	pTrn[8] = pSfmReconstructionGroundPlane->R[2];
	pTrn[12] = 0;
		  
	pTrn[1] = pSfmReconstructionGroundPlane->R[3];
	pTrn[5] = pSfmReconstructionGroundPlane->R[4];
	pTrn[9] = pSfmReconstructionGroundPlane->R[5];
	pTrn[13] = 0;

	pTrn[2]  = pSfmReconstructionGroundPlane->R[6];
	pTrn[6]  = pSfmReconstructionGroundPlane->R[7];
	pTrn[10] = pSfmReconstructionGroundPlane->R[8];
	pTrn[14] = 0;

	pTrn[3] =  pSfmReconstructionAnchor->x;
	pTrn[7] =  pSfmReconstructionAnchor->y;
	pTrn[11] = pSfmReconstructionAnchor->z;
	pTrn[15] = 1;
};

__forceinline void GetCoordinateSystemTransformInv( __in CoordinateSystemAnchor *pSfmReconstructionAnchor, __in CoordinateSystemGroundPlane *pSfmReconstructionGroundPlane, __out_ecount(16) double *pTrn )
{
	
	pTrn[0] = pSfmReconstructionGroundPlane->R[0];
	pTrn[4] = pSfmReconstructionGroundPlane->R[3];
	pTrn[8] = pSfmReconstructionGroundPlane->R[6];
	pTrn[12] = 0;
		  
	pTrn[1] = pSfmReconstructionGroundPlane->R[1];
	pTrn[5] = pSfmReconstructionGroundPlane->R[4];
	pTrn[9] = pSfmReconstructionGroundPlane->R[7];
	pTrn[13] = 0;

	pTrn[2]  = pSfmReconstructionGroundPlane->R[2];
	pTrn[6]  = pSfmReconstructionGroundPlane->R[5];
	pTrn[10] = pSfmReconstructionGroundPlane->R[8];
	pTrn[14] = 0;

	pTrn[3] =  - ( pSfmReconstructionAnchor->x * pSfmReconstructionGroundPlane->R[0] + 
		           pSfmReconstructionAnchor->y * pSfmReconstructionGroundPlane->R[1] + 
				   pSfmReconstructionAnchor->z * pSfmReconstructionGroundPlane->R[2] );
	pTrn[7] =  - ( pSfmReconstructionAnchor->x * pSfmReconstructionGroundPlane->R[3] + 
		           pSfmReconstructionAnchor->y * pSfmReconstructionGroundPlane->R[4] + 
				   pSfmReconstructionAnchor->z * pSfmReconstructionGroundPlane->R[5] );
	pTrn[11] = - ( pSfmReconstructionAnchor->x * pSfmReconstructionGroundPlane->R[6] + 
		           pSfmReconstructionAnchor->y * pSfmReconstructionGroundPlane->R[7] + 
				   pSfmReconstructionAnchor->z * pSfmReconstructionGroundPlane->R[8] );
	pTrn[15] = 1;
	

	//double A[16];
	//GetCoordinateSystemTransform( pSfmReconstructionAnchor, pSfmReconstructionGroundPlane, A );
	//Mat44Inv( A, pTrn );
};

HRESULT ConvertGcsBoxToSfmBox(
	__in CoordinateSystemPoint centre,
	__in_ecount(3) double *pWidthHeightDepth,
	__in CapturingReality::CoordinateSystems::ICoordinateSystem *pCoordinatesystem,
	__in CapturingReality::Sfm::ISfmReconstruction *pSfmReconstruction,
	__out CapturingReality::Geometry::GlobalReconstructionVolume *pBox );

HRESULT ApproximateReconstructionBox(
	__out CapturingReality::Geometry::GlobalReconstructionVolume* pBox,
	__in CapturingReality::Sfm::IStructureFromMotion* pSfm,
	__in CapturingReality::Sfm::ISfmReconstruction* pReconstruction,
	__in bool bAxisAlignedBox );