//
// Capturing Reality s.r.o.
//
// This file 'OrthoProjectionHelpers.cpp' is a part of the RC Engine Samples. You may use it freely.
//
#include "stdafx.h"
#include <string.h>  
#include <string>  
#include "SamplesCommonLib.h"

//Define Macro
#define BUFFER_SIZE 2048

#define RAD_TO_DEG	57.295779513082321
#define DEG_TO_RAD	.017453292519943296

extern double north;
extern double south;
extern double east;
extern double west;

extern UINT projectionWidth;
extern UINT projectionHeight;
extern double projectionDepth;

HRESULT CalculateAxisAlignedOrthoProjection( 
	__in CapturingReality::CoordinateSystemPoint *pCentre, 
	__in UINT width, 
	__in UINT height, 
	__in double depth, 
	__in double pixelSizeX,
	__in double pixelSizeY,
	__in_z const wchar_t *pFriendlyName, 
	__in_z const wchar_t *pDefinition,
	__in CapturingReality::IResourceCache *pResourceCache,
	__in CapturingReality::Sfm::ISfmReconstruction *pSfmReconstruction,
	__in CapturingReality::Mvs::IMvsModel *pMvsModel,
	__deref_out CapturingReality::OrthoProjection::IOrthographicProjection **ppOrthoProjection )
{
	CComPtr< CapturingReality::CoordinateSystems::ICoordinateSystem > spCoordinatesystem;
	HRESULT hr = CapturingReality::CoordinateSystems::CreateCoordinateSystem( pFriendlyName, pDefinition, &spCoordinatesystem );
	if ( SUCCEEDED( hr ) )
	{
		CapturingReality::OrthoProjection::CreateOrthoProjectionParameters opp;

		double axes[9] = { 1,0,0,0,-1,0,0,0,1 }; 

		CapturingReality::ModelTools::CalculateOrthoProjectionMatrix( 
					pCentre, 
					axes, 
					width, 
					height, 
					pixelSizeX, 
					pixelSizeY, 
					opp.csOrthoCameraData.projectionMatrix );

		opp.csOrthoCameraData.width = width;
		opp.csOrthoCameraData.height = height;
		opp.csOrthoCameraData.depth = depth;
	
		hr = pSfmReconstruction->GetReconstructionId( &opp.sfmComponentGuid );
		if ( SUCCEEDED( hr ) )
		{
			opp.coordSys = pDefinition;
			opp.coordType = pFriendlyName;
			opp.coordTag = -1;
			opp.isGeoreferenced = true;
			opp.sfmReconstructionAnchor = pSfmReconstruction->GetAnchor();
			opp.sfmReconstructionGroundPlane = pSfmReconstruction->GetGroundPlane();
			opp.sfmResidualTransform = pSfmReconstruction->GetResidualTransform();
			double R[9] = {1,0,0,0,1,0,0,0,1};
			CopyMemory( opp.modelSfmToActualSfmTransform.R, R, 9 * sizeof( double ) ); 
			opp.modelSfmToActualSfmTransform.s = 1.0;
			opp.modelSfmToActualSfmTransform.t[0] = 0;
			opp.modelSfmToActualSfmTransform.t[1] = 0;
			opp.modelSfmToActualSfmTransform.t[2] = 0;

			if ( pMvsModel->IsTextured() == true )
			{
				opp.colorType = CapturingReality::FrameColorType::FRCT_COLOR_TEXTURING;
			}
			else
			{
				opp.colorType = CapturingReality::FrameColorType::FRCT_COLOR_COLORING;
			};
			opp.generateImage = true;
			opp.generateNearPlaneCut = false;
			opp.backFaceColorType = CapturingReality::BackFaceColorType::BFCT_FIXED_COLOR;
			opp.backFaceColor = 0x0000ff;
			
			hr = CapturingReality::ModelTools::CreateOrthoProjectionFromModel( &opp, pMvsModel, pResourceCache, NULL, ppOrthoProjection );
		};
	};

	return hr;
}

HRESULT ExportWorldFile( 
	__in_z const wchar_t *pFileName, 
	__in CapturingReality::OrthoProjection::IOrthographicProjection *pOrthoProjection )
{
	CComPtr< CapturingReality::OrthoProjection::IOrthoCsConversion > spConversion;
	HRESULT hr = CapturingReality::ModelTools::CreateOrthoCsConversion( pOrthoProjection, &spConversion );
	if ( SUCCEEDED( hr ) )
	{
		CapturingReality::OrthoProjection::WordFileData wfd;
		hr = CapturingReality::ModelTools::GetWorldFileData( 
				spConversion, 
				CapturingReality::OrthoProjection::WorldCoordSystemCentreType::WCSCT_GLOBAL,
				CapturingReality::OrthoProjection::WorldCoordSystemAxesType::WCSAT_GLOBAL_COORDINATE_SYSTEM,					
				&wfd );
		if ( SUCCEEDED( hr ) )
		{
			FILE *f;
			errno_t err = _wfopen_s(&f, pFileName, L"w");
			if (err != 0 )
			{
				hr = E_FAIL;
			}
			else
			{
				fprintf_s(f, "%0.10f\n", wfd.A);
				fprintf_s(f, "%0.10f\n", wfd.D);
				fprintf_s(f, "%0.10f\n", wfd.B);
				fprintf_s(f, "%0.10f\n", wfd.E);
				fprintf_s(f, "%0.10f\n", wfd.C);
				fprintf_s(f, "%0.10f\n", wfd.F);

				fclose(f);
			};
		};
	};

	return hr;
}

HRESULT CalculateOrthoProjectionAndExportToTiffGPS(
	__in_z wchar_t const* output_dir,
	__in CapturingReality::IResourceCache *pResourceCache,
	__in IStructureFromMotion *pSfm,
	__in ISfmReconstruction *pSfmReconstruction,
	__in IMvsModel *pMvsModel,
	__in GlobalReconstructionVolume *pReconstructionRegion,
	__in float cmPerPixel)
{
	HRESULT hr;

	printf("\nCalculating ... \n");

	if (pSfmReconstruction == NULL || pMvsModel == NULL)
	{
		return E_UNEXPECTED;
		exit(-700);
	}
	
	auto const flags = pSfmReconstruction->GetReconstructionFlags();
	printf("\nCalculating orthographic ... flags = %d \n", flags);

	if ((flags & (SCCF_GROUND_CONTROL | SCCF_METRIC)) == 0)
	{
		return E_FAIL;
		exit(-710);
	}

	CComPtr< CapturingReality::CoordinateSystems::ICoordinateSystem > spGpsCoordSystem;
	const wchar_t* g_gps_name = L"GPS (WGS 84)";
	const wchar_t* g_gps_definition = L"+proj=longlat +datum=WGS84 +no_defs";
	hr = CreateCoordinateSystem(g_gps_name, g_gps_definition, &spGpsCoordSystem);

	if (FAILED(hr))
	{
		return hr;
		exit(-720);
	}
	printf("\nCalculating orthographic projection ... \n");
	CapturingReality::CoordinateSystemAnchor anchor = pSfmReconstruction->GetAnchor();
	CapturingReality::CoordinateSystemGroundPlane groundPlane = pSfmReconstruction->GetGroundPlane();

	CapturingReality::CoordinateSystemPoint centreRegion;
	centreRegion.x = pReconstructionRegion->position.x;
	centreRegion.y = pReconstructionRegion->position.y;
	centreRegion.z = pReconstructionRegion->position.z;

	CapturingReality::CoordinateSystemPoint centreGps;

	MulV3M33(&centreRegion.x, groundPlane.R, &centreGps.x);

	centreGps.x += anchor.x;
	centreGps.y += anchor.y;
	centreGps.z += anchor.z;

	hr = spGpsCoordSystem->ToModel(1, &centreGps);

	if (FAILED(hr))
	{
		return hr;
		exit(-730);
	}

	//double minLng = +XM_PI;
	//double maxLng = -XM_PI;
	//double minLat = +XM_PIDIV2;
	//double maxLat = -XM_PIDIV2;
	//for (int j = -1; j <= +1; j += 2)
	//{
	//	for (int i = -1; i <= +1; i += 2)
	//	{
	//		CapturingReality::CoordinateSystemPoint p;
	//		p.x = i * pReconstructionRegion->orientation.axes[0] * pReconstructionRegion->extents.x + j * pReconstructionRegion->orientation.axes[3] * pReconstructionRegion->extents.y;
	//		p.y = i * pReconstructionRegion->orientation.axes[1] * pReconstructionRegion->extents.x + j * pReconstructionRegion->orientation.axes[4] * pReconstructionRegion->extents.y;
	//		p.z = i * pReconstructionRegion->orientation.axes[2] * pReconstructionRegion->extents.x + j * pReconstructionRegion->orientation.axes[5] * pReconstructionRegion->extents.y;
	//		CapturingReality::CoordinateSystemPoint q;
	//		MulV3M33(&p.x, groundPlane.R, &q.x);
	//		q.x += anchor.x;
	//		q.y += anchor.y;
	//		q.z += anchor.z;
	//		spGpsCoordSystem->ToModel(1, &q);
	//		minLng = min(minLng, q.x);
	//		maxLng = max(maxLng, q.x);
	//		minLat = min(minLat, q.y);
	//		maxLat = max(maxLat, q.y);
	//	}
	//}
	
	auto const kmlTileSide = 1024;

	//double const R = 6378100.0;	//m
	////auto const RES_LAT = 100.0; //[resolution] = pixels / m
	//auto const RES_LAT = 100 / cmPerPixel;
	//auto const RES_LNG = RES_LAT * cos(centreGps.y); //[resolution] = pixels / m
	//double const pixelSizeInRadiansX = 1.0 / RES_LNG / R;
	//double const pixelSizeInRadiansY = 1.0 / RES_LAT / R;
	//auto const projectionWidth = (UINT)(((maxLng - minLng) * R) * RES_LNG + 0.5);
	//auto const projectionHeight = (UINT)(((maxLat - minLat) * R) * RES_LAT + 0.5);	
	//auto const projectionDepth = 100.0; // in meters

	const double pixelSizeInRadiansX = (east - west)* DEG_TO_RAD / 1024.0;
	const double pixelSizeInRadiansY = (north - south)* DEG_TO_RAD / 1024.0;

	CComPtr< CapturingReality::OrthoProjection::IOrthographicProjection > spOrthoProjection;
	CComPtr< CapturingReality::OrthoProjection::IOrthoProjectionDataProvider > spOrthoProjectionDataProvider;

	printf( "\nCalculating orthographic projection GPS... \n" );	

	hr = CalculateAxisAlignedOrthoProjection(
		&centreGps, projectionWidth, projectionHeight,
		projectionDepth, pixelSizeInRadiansX, pixelSizeInRadiansY,
		g_gps_name, g_gps_definition,
		pResourceCache, pSfmReconstruction, pMvsModel, &spOrthoProjection
	);

	if (FAILED(hr))
	{
		return hr;
		exit(-740);
	}

	// Export into kmz.
	hr = CapturingReality::ModelTools::CreateOrthoPhotoDataProvider(spOrthoProjection, NULL, &spOrthoProjectionDataProvider);
	if (FAILED(hr))
	{
		return hr;
		exit(-750);
	}

	if (SUCCEEDED(hr))
	{
		bool bGeoTaggsWritten;

		wchar_t tifFileName[BUFFER_SIZE] = { 0 };
		wcscpy_s(tifFileName, BUFFER_SIZE, output_dir);
		wcscat_s(tifFileName, BUFFER_SIZE, L"quarry_mosaic_group1.tif");


		hr = CapturingReality::ModelTools::ExportOrthoProjectionToGeoTiff(
			CapturingReality::OrthoProjection::WorldCoordSystemCentreType::WCSCT_GLOBAL,
			CapturingReality::OrthoProjection::WorldCoordSystemAxesType::WCSAT_GLOBAL_COORDINATE_SYSTEM,
			CapturingReality::OrthoProjection::OrthoPixelType::OPT_COLOR,
			CapturingReality::OrthoProjection::CompressionType::CT_NONE,
			spOrthoProjectionDataProvider,
			tifFileName,
			false,
			NULL,
			&bGeoTaggsWritten
		);

		if (SUCCEEDED(hr))
		{
			wchar_t tfwFileName[BUFFER_SIZE] = { 0 };
			wcscpy_s(tfwFileName, BUFFER_SIZE, output_dir);
			wcscat_s(tfwFileName, BUFFER_SIZE, L"quarry_mosaic_group1.tfw");

			hr = ExportWorldFile(tfwFileName, spOrthoProjection);
		}

		/*if (SUCCEEDED(hr)) {
			wchar_t kmzFileName[BUFFER_SIZE] = { 0 };
			wcscpy_s(kmzFileName, BUFFER_SIZE, output_dir);
			wcscat_s(kmzFileName, BUFFER_SIZE, L"quarry_mosaic_group1.kmz");
			CapturingReality::OrthoProjection::OrthoImageExportParameters parameters;
			parameters.filename = kmzFileName;
			parameters.gcpsCount = 0;
			parameters.pGcps = NULL;
			parameters.imageQuality = 90;
			parameters.kmlExportCompressed = true;
			parameters.kmlTileExtension = L"png";
			parameters.kmlTileSide = kmlTileSide;
			parameters.kmlTileWicCodecGUID = GUID_ContainerFormatPng;
			hr = CapturingReality::ModelTools::ExportOrthoPhotoToKml(&parameters, spOrthoProjectionDataProvider, NULL, NULL);
			if (FAILED(hr))
			{
				return hr;
				exit(-760);
			}
		}*/
		
	}

	return S_OK;
}