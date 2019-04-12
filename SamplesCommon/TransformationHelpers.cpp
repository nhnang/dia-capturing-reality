//
// Capturing Reality s.r.o.
//
// This file 'TransformationHelpers.cpp' is a part of the RC Engine Samples. You may use it freely.
//

#include "stdafx.h"
#include "MathHelpers.h"
#include "TransformationHelpers.h"
#include "SamplesCommonLib.h"

HRESULT ConvertGcsBoxToSfmBox(
	__in CoordinateSystemPoint centre,
	__in_ecount(3) double *pWidthHeightDepth,
	__in CapturingReality::CoordinateSystems::ICoordinateSystem *pCoordinatesystem,
	__in CapturingReality::Sfm::ISfmReconstruction *pSfmReconstruction,
	__out CapturingReality::Geometry::GlobalReconstructionVolume *pBox )
{
	if ( ( pCoordinatesystem == NULL )||( pBox == NULL ) )
	{
		return E_INVALIDARG;
	};

	//Convert from the global coordinate system to the sfm coordinate system. The Sfm must be geo-referenced!
	CoordinateSystemPoint pt[4];
		
	pt[0] = centre;
		
	pt[1] = pt[0]; pt[1].x += pWidthHeightDepth[0];
	pt[2] = pt[0]; pt[2].y += pWidthHeightDepth[1];
	pt[3] = pt[0]; pt[3].z += pWidthHeightDepth[2];

	HRESULT hr = pCoordinatesystem->ToEuclidean( 4, pt );
	if ( SUCCEEDED( hr ) )
	{
		CoordinateSystemGroundPlane gp = pSfmReconstruction->GetGroundPlane();
		CoordinateSystemAnchor		an = pSfmReconstruction->GetAnchor();
		double euclid2sfm[16];
		GetCoordinateSystemTransformInv( &an, &gp, euclid2sfm );
		for ( UINT i = 0; i < 4; i++ )
		{
			TransformPoint( &pt[i], euclid2sfm, &pt[i] );
		};

		cspSub( &pt[1], &pt[0], &pt[1] );
		cspSub( &pt[2], &pt[0], &pt[2] );
		cspSub( &pt[3], &pt[0], &pt[3] );

		pBox->position.x = pt[0].x;
		pBox->position.y = pt[0].y;
		pBox->position.z = pt[0].z;
		pBox->extents.x = (float)cspLength( &pt[1] );
		pBox->extents.y = (float)cspLength( &pt[2] );
		pBox->extents.z = (float)cspLength( &pt[3] );


		double axes[9];
		
		axes[0] = pt[1].x / pBox->extents.x;
		axes[1] = pt[1].y / pBox->extents.x;
		axes[2] = pt[1].z / pBox->extents.x;
		axes[3] = pt[2].x / pBox->extents.y;
		axes[4] = pt[2].y / pBox->extents.y;
		axes[5] = pt[2].z / pBox->extents.y;
		axes[6] = pt[3].x / pBox->extents.z;
		axes[7] = pt[3].y / pBox->extents.z;
		axes[8] = pt[3].z / pBox->extents.z;
		
		//orthogonalize & ensure right-handed
		Vec3Cross( &axes[6], &axes[0], &axes[3] );
		Vec3Cross( &axes[3], &axes[0], &axes[6] );
		axes[3] = -axes[3];
		axes[4] = -axes[4];
		axes[5] = -axes[5];


		pBox->orientation.axes[0] = (float)axes[0];
		pBox->orientation.axes[1] = (float)axes[1];
		pBox->orientation.axes[2] = (float)axes[2];
		pBox->orientation.axes[3] = (float)axes[3];
		pBox->orientation.axes[4] = (float)axes[4];
		pBox->orientation.axes[5] = (float)axes[5];
		pBox->orientation.axes[6] = (float)axes[6];
		pBox->orientation.axes[7] = (float)axes[7];
		pBox->orientation.axes[8] = (float)axes[8];

	};		

	return hr;	
};

HRESULT ApproximateReconstructionBox(
	__out GlobalReconstructionVolume* pBox,
	__in CapturingReality::Sfm::IStructureFromMotion* pSfm,
	__in CapturingReality::Sfm::ISfmReconstruction* pReconstruction,
	__in bool bAxisAlignedBox
)
{
	if (!pBox || !pSfm || !pReconstruction)
	{
		_ASSERT(pBox);
		_ASSERT(pSfm);
		_ASSERT(pReconstruction);
		return E_INVALIDARG;
	}

	// scene reconstruction box axes
	double u[3] = { 0 };
	double v[3] = { 0 };
	double n[3] = { 0.0, 0.0 , 1.0 };

	// relative box dimensions
	double minv[3];
	double maxv[3];

	if (bAxisAlignedBox)
	{
		u[0] = 1.0;
		v[1] = 1.0;
	}

	CComPtr< CapturingReality::Sfm::ISfmReconstructionBox > spBox;

	HRESULT hr = CapturingReality::Sfm::CreateSfmAutoReconstructionBox(pSfm, pReconstruction, &spBox);
	if (SUCCEEDED(hr))
	{
		hr = spBox->CalculateSceneBox(n, u, v, bAxisAlignedBox, minv, maxv);
	}
	if (SUCCEEDED(hr))
	{
		double R[9];
		R[0] = u[0];    R[1] = u[1];    R[2] = u[2];
		R[3] = v[0];    R[4] = v[1];    R[5] = v[2];
		R[6] = n[0];    R[7] = n[1];    R[8] = n[2];

		_ASSERT(Mat33Det(R) > 0);

		double t[3]; // relative box centre
		t[0] = (minv[0] + maxv[0]) / 2;
		t[1] = (minv[1] + maxv[1]) / 2;
		t[2] = (minv[2] + maxv[2]) / 2;

		double c[3]; // absolute box centre
		MulV3M33(t, R, c);

		pBox->position.x = c[0];
		pBox->position.y = c[1];
		pBox->position.z = c[2];

		Vec3Scale(u, (maxv[0] - minv[0]) / 2);
		Vec3Scale(v, (maxv[1] - minv[1]) / 2);
		Vec3Scale(n, (maxv[2] - minv[2]) / 2);

		pBox->extents.x = (float)Vec3Len(u);
		pBox->extents.y = (float)Vec3Len(v);
		pBox->extents.z = (float)Vec3Len(n);

		Vec3Normalize(u);
		Vec3Normalize(v);
		Vec3Cross(n, u, v); // ensure right-handedness

		pBox->orientation.axes[0] = (float)u[0];
		pBox->orientation.axes[1] = (float)u[1];
		pBox->orientation.axes[2] = (float)u[2];

		pBox->orientation.axes[3] = (float)v[0];
		pBox->orientation.axes[4] = (float)v[1];
		pBox->orientation.axes[5] = (float)v[2];

		pBox->orientation.axes[6] = (float)n[0];
		pBox->orientation.axes[7] = (float)n[1];
		pBox->orientation.axes[8] = (float)n[2];
	}
	return hr;
}