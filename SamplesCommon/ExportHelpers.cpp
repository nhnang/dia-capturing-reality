//
// Capturing Reality s.r.o.
//
// This file 'ExportHelpers.cpp' is a part of the RC Engine Samples. You may use it freely.
//

#include "stdafx.h"
#include <iostream>
#include <locale>
#include <vector>
#include <string>
#include <fstream>
#include "SamplesCommonLib.h"

using namespace std;
using namespace CapturingReality;

extern const wchar_t* g_epsg_2451_name;
extern const wchar_t* g_epsg_2451_definition;

extern const wchar_t* g_gps_name;
extern const wchar_t* g_gps_definition;

void GetBBoxHexahedron( __in const FXMVECTOR &LU, __in const FXMVECTOR &RD, __out_ecount(8) XMVECTOR *corners )
{
	corners[0] = XMVectorSet(LU.m128_f32[0],LU.m128_f32[1],LU.m128_f32[2],1.0f);
	corners[1] = XMVectorSet(RD.m128_f32[0],LU.m128_f32[1],LU.m128_f32[2],1.0f);
	corners[2] = XMVectorSet(RD.m128_f32[0],RD.m128_f32[1],LU.m128_f32[2],1.0f);
	corners[3] = XMVectorSet(LU.m128_f32[0],RD.m128_f32[1],LU.m128_f32[2],1.0f);
	corners[4] = XMVectorSet(LU.m128_f32[0],LU.m128_f32[1],RD.m128_f32[2],1.0f);
	corners[5] = XMVectorSet(RD.m128_f32[0],LU.m128_f32[1],RD.m128_f32[2],1.0f);
	corners[6] = XMVectorSet(RD.m128_f32[0],RD.m128_f32[1],RD.m128_f32[2],1.0f);
	corners[7] = XMVectorSet(LU.m128_f32[0],RD.m128_f32[1],RD.m128_f32[2],1.0f);
};

HRESULT ComputePartBoundingBoxInActualSfm( 
	__in CapturingReality::Sfm::ISfmReconstruction *pSfmReconstruction,
	__in CapturingReality::Mvs::IMvsModel *pMvsModel, 
	__in UINT partIndex, 
	__out BoundingBox *pBoundingBoxInActualSfm )
{
    if ( !pMvsModel || !pSfmReconstruction )
    {
        return E_INVALIDARG;
    }

	pBoundingBoxInActualSfm->bbMin = XMVectorSet( FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX );
	pBoundingBoxInActualSfm->bbMax = XMVectorSet(-FLT_MAX,-FLT_MAX,-FLT_MAX,-FLT_MAX );

	CComPtr< CapturingReality::Mvs::IMvsModelPart > spPart;
	HRESULT hr = pMvsModel->GetPart( partIndex, &spPart );
	if ( SUCCEEDED( hr ) )
	{
		BoundingBox box = spPart->GetInternalTrianglesBoxInModelAxisAlignedCS();
		XMVECTOR hexahInModelCS[8];
		GetBBoxHexahedron( box.bbMin, box.bbMax, hexahInModelCS );
		
		CoordinateSystemResidual rootToModel = pMvsModel->GetResidualTransform();
		CoordinateSystemResidual rootToActual = pSfmReconstruction->GetResidualTransform();
		CoordinateSystemResidual modelSfmToActualSfmTransform = GetModelToActualTransform( &rootToModel, &rootToActual );

		CoordinateSystemResidual modelCStoModelSfmTransform;
		modelCStoModelSfmTransform.s = 1.0f;
		modelCStoModelSfmTransform.t[0] = pMvsModel->GetPartParamsPtr( partIndex )->anchorInGCS.x;
		modelCStoModelSfmTransform.t[1] = pMvsModel->GetPartParamsPtr( partIndex )->anchorInGCS.y;
		modelCStoModelSfmTransform.t[2] = pMvsModel->GetPartParamsPtr( partIndex )->anchorInGCS.z;
		pMvsModel->GetModelBBoxToGCSRotation( modelCStoModelSfmTransform.R );

		CoordinateSystemResidual modelCStoActualSfm = TransformResidualTransform( &modelSfmToActualSfmTransform, &modelCStoModelSfmTransform );

		double trn[12] =
			{ modelCStoActualSfm.R[0] * modelCStoActualSfm.s, modelCStoActualSfm.R[1] * modelCStoActualSfm.s, modelCStoActualSfm.R[2] * modelCStoActualSfm.s, modelCStoActualSfm.t[0],
				modelCStoActualSfm.R[3] * modelCStoActualSfm.s, modelCStoActualSfm.R[4] * modelCStoActualSfm.s, modelCStoActualSfm.R[5] * modelCStoActualSfm.s, modelCStoActualSfm.t[1],
				modelCStoActualSfm.R[6] * modelCStoActualSfm.s, modelCStoActualSfm.R[7] * modelCStoActualSfm.s, modelCStoActualSfm.R[8] * modelCStoActualSfm.s, modelCStoActualSfm.t[2] };

		for ( UINT i = 0; i < 8; i++ ) 
		{
			double PModelCS[3] = { (double)hexahInModelCS[i].m128_f32[0], (double)hexahInModelCS[i].m128_f32[1], (double)hexahInModelCS[i].m128_f32[2] };
			double PActSfm[3];
			MulM34V3( trn, PModelCS, PActSfm );

			XMVECTOR pt;
			pt.m128_f32[0] = (float)PActSfm[0];
			pt.m128_f32[1] = (float)PActSfm[1];
			pt.m128_f32[2] = (float)PActSfm[2];
			pt.m128_f32[3] = 1.0f;
		
			pBoundingBoxInActualSfm->bbMin = XMVectorMin( pBoundingBoxInActualSfm->bbMin, pt );
			pBoundingBoxInActualSfm->bbMax = XMVectorMax( pBoundingBoxInActualSfm->bbMax, pt );
		};
	};

	return hr;
};

HRESULT ComputeBoundingBoxInActualSfm( 
	__in CapturingReality::Sfm::ISfmReconstruction *pSfmReconstruction,
	__in CapturingReality::Mvs::IMvsModel *pMvsModel, 
	__out BoundingBox *pBoundingBoxInActualSfm  )
{
	pBoundingBoxInActualSfm->bbMin = XMVectorSet( FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX );
	pBoundingBoxInActualSfm->bbMax = XMVectorSet(-FLT_MAX,-FLT_MAX,-FLT_MAX,-FLT_MAX );

	UINT nParts = pMvsModel->GetPartsCount();

	if ( nParts == 0 )
		return E_INVALIDARG;

	HRESULT hr = S_OK;
	for ( UINT i = 0; i < nParts; i++ )
	{
		BoundingBox bbPart;
		hr = ComputePartBoundingBoxInActualSfm( pSfmReconstruction, pMvsModel, i, &bbPart );
		if ( SUCCEEDED( hr ) )
		{
			pBoundingBoxInActualSfm->bbMin = XMVectorMin( pBoundingBoxInActualSfm->bbMin, bbPart.bbMax );
			pBoundingBoxInActualSfm->bbMax = XMVectorMax( pBoundingBoxInActualSfm->bbMax, bbPart.bbMax );
			pBoundingBoxInActualSfm->bbMin = XMVectorMin( pBoundingBoxInActualSfm->bbMin, bbPart.bbMin );
			pBoundingBoxInActualSfm->bbMax = XMVectorMax( pBoundingBoxInActualSfm->bbMax, bbPart.bbMin );
		};

		if ( FAILED( hr ) )
			break;
	};
	
	return hr;
};

HRESULT GetGridExportTransformationProvider(
	__in CapturingReality::Sfm::ISfmReconstruction *pSfmReconstruction,
	__in CapturingReality::Mvs::IMvsModel *pMvsModel,
	__deref_out CapturingReality::ImportExport::IExportTransformationsProvider **ppExportTransformationsProvider)
{
	CapturingReality::CalibrationExport::ExportTransformationSettings exportTransformation;

	Mat33SetIdentity(exportTransformation.actualSfm2GridPlaneTransform);

	CoordinateSystemResidual rootToActual = pSfmReconstruction->GetResidualTransform();
	InvertResidualTransform(&rootToActual, &exportTransformation.actualSfmToRootSfm);

	CoordinateSystemResidual rootToModel = pMvsModel->GetResidualTransform();
	exportTransformation.modelSfmToActualSfmTransform = GetModelToActualTransform(&rootToModel, &rootToActual);

	exportTransformation.sfmReconstructionAnchor = pSfmReconstruction->GetAnchor();
	exportTransformation.sfmReconstructionGroundPlane = pSfmReconstruction->GetGroundPlane();

	exportTransformation.exportCoordinateSystemType = CapturingReality::CalibrationExport::ExportCoordinateSystemType::ECST_GRID_PLANE;

	exportTransformation.pOutputCoordSys = L"+proj=geocent +ellps=WGS84 +no_defs";
	exportTransformation.pOutputCoordSysName = L"local:1 - Euclidean";

	exportTransformation.settingsAnchor.x = 0.0;
	exportTransformation.settingsAnchor.y = 0.0;
	exportTransformation.settingsAnchor.z = 0.0;
	exportTransformation.settingsScalex = 1.0;
	exportTransformation.settingsScaley = 1.0;
	exportTransformation.settingsScalez = 1.0;

	HRESULT hr = ComputeBoundingBoxInActualSfm(pSfmReconstruction, pMvsModel, &exportTransformation.boundingBoxInActualSfm);
	if (SUCCEEDED(hr))
	{
		hr = CapturingReality::ModelTools::CreateExportTransformationsProvider(&exportTransformation, ppExportTransformationsProvider);
	};

	return hr;
};

HRESULT GetGridExportTransformationProviderWithOutputCoordSys(
	__in CapturingReality::Sfm::ISfmReconstruction *pSfmReconstruction,
	__in CapturingReality::Mvs::IMvsModel *pMvsModel,
	__in const wchar_t* pOutputCoordSys, 
	__in const wchar_t* pOutputCoordSysName,
	__deref_out CapturingReality::ImportExport::IExportTransformationsProvider **ppExportTransformationsProvider)
{
	HRESULT hr;
	CapturingReality::CalibrationExport::ExportTransformationSettings exportTransformation;	

	CComPtr< CapturingReality::CoordinateSystems::ICoordinateSystem > spGpsCoordSystem;
	hr = CreateCoordinateSystem(g_epsg_2451_name, g_epsg_2451_definition, &spGpsCoordSystem);

    Mat33SetIdentity( exportTransformation.actualSfm2GridPlaneTransform );

	CoordinateSystemResidual rootToActual = pSfmReconstruction->GetResidualTransform();
	InvertResidualTransform( &rootToActual, &exportTransformation.actualSfmToRootSfm );

	CoordinateSystemResidual rootToModel = pMvsModel->GetResidualTransform();
	exportTransformation.modelSfmToActualSfmTransform = GetModelToActualTransform( &rootToModel, &rootToActual );

	exportTransformation.sfmReconstructionAnchor = pSfmReconstruction->GetAnchor();
	exportTransformation.sfmReconstructionGroundPlane = pSfmReconstruction->GetGroundPlane();

	exportTransformation.exportCoordinateSystemType = CapturingReality::CalibrationExport::ExportCoordinateSystemType::ECST_PROJECT_OUTPUT;

	exportTransformation.pOutputCoordSys = pOutputCoordSys;
	exportTransformation.pOutputCoordSysName = pOutputCoordSysName;

	exportTransformation.settingsAnchor.x = 0.0;
	exportTransformation.settingsAnchor.y = 0.0;
	exportTransformation.settingsAnchor.z = 0.0;
	exportTransformation.settingsScalex = 1.0;
	exportTransformation.settingsScaley = 1.0;
	exportTransformation.settingsScalez = 1.0;
	
	hr = ComputeBoundingBoxInActualSfm( pSfmReconstruction, pMvsModel, &exportTransformation.boundingBoxInActualSfm );
	if ( SUCCEEDED( hr ) )
	{
		hr = CapturingReality::ModelTools::CreateExportTransformationsProvider( &exportTransformation, ppExportTransformationsProvider );
	};

	return hr;
};

HRESULT ExportModelPly(
	__in_z const wchar_t *dir,
	__in_z const wchar_t *name,
	__in CapturingReality::IResourceCache *pResourceCache,
	__in CapturingReality::Sfm::IStructureFromMotion *pSfm,
	__in CapturingReality::Sfm::ISfmReconstruction *pSfmReconstruction,
	__in CapturingReality::Mvs::IMvsModel *pMvsModel,
	__in_opt CapturingReality::ImportExport::IExportTransformationsProvider *pTransform)
{
	CapturingReality::ImportExport::XmlModelExportParams settings = { 0 };

	settings.pAuthorComment = L"RealityCapture Engine Sample Code";
	settings.exportBinary = true;
	settings.exportInfoFile = true;
	settings.exportVertices = true;
	settings.exportVertexColors = CapturingReality::ImportExport::VertexColorType::PCT_COLOR_PER_VERTEX;
	settings.exportVertexNormals = false;
	settings.exportTriangles = true;
	settings.exportTexturing = false;
	settings.meshColor = 0xff0000ff;
	settings.tileType = CapturingReality::ImportExport::UVTileType::UVTT_TILE_U1V1;
	settings.exportTextureAlpha = false;
	settings.exportToOneTexture = true;
	settings.oneTextureMaxSide = 16384;
	settings.oneTextureUsePow2TexSide = true;

	settings.exportCoordinateSystemType = CapturingReality::CalibrationExport::ExportCoordinateSystemType::ECST_GRID_PLANE;
	settings.settingsAnchor.x = 0.0;
	settings.settingsAnchor.y = 0.0;
	settings.settingsAnchor.z = 0.0;
	settings.settingsScalex = 1.0;
	settings.settingsScaley = 1.0;
	settings.settingsScalez = 1.0;

	wcscpy_s(settings.texturesFileType, L"png");
	wcscpy_s(settings.formatAndVersionUID, L"ply 000 ");

	settings.exportModelByParts = false;
	settings.exportRandomPartColor = false;

	settings.exportCameras = false;
	settings.exportCamerasAsModelPart = false;

	strcpy_s(settings.numberAsciiFormatting, "%.16e");

	HRESULT hr = S_FALSE;
	CComPtr< CapturingReality::ImportExport::IExportTransformationsProvider > spExportTransformationsProvider = pTransform;
	if (!spExportTransformationsProvider)
	{
		hr = GetGridExportTransformationProvider(pSfmReconstruction, pMvsModel, &spExportTransformationsProvider);
	}
	if (SUCCEEDED(hr))
	{
		CComPtr< CapturingReality::ImportExport::IReconstructionExporter > spExporter;
		hr = CapturingReality::ImportExport::CreateReconstructionExporterPly(dir, name, &settings, &spExporter);
		if (SUCCEEDED(hr))
		{
			hr = CapturingReality::ModelTools::ExportModel(spExporter, pMvsModel, spExportTransformationsProvider, NULL, NULL, NULL);
		};
	};

	return hr;
};

HRESULT ExportModelObj(
	__in_z const wchar_t *dir,
	__in_z const wchar_t *name,
	__in CapturingReality::IResourceCache *pResourceCache,
	__in CapturingReality::Sfm::IStructureFromMotion *pSfm,
	__in CapturingReality::Sfm::ISfmReconstruction *pSfmReconstruction,
	__in CapturingReality::Mvs::IMvsModel *pMvsModel,
	__in_opt CapturingReality::ImportExport::IExportTransformationsProvider *pTransform)
{
	CapturingReality::ImportExport::XmlModelExportParams settings = { 0 };;
	settings.pAuthorComment = L"RealityCapture Engine Sample Code";
	settings.exportBinary = false;
	settings.exportInfoFile = true;
	settings.exportTextureAlpha = true;
	settings.exportToOneTexture = true;
	settings.exportTriangles = true;

	settings.exportTexturing = pMvsModel->IsTextured();
	if (pMvsModel->IsTextured() == false)
	{
		settings.exportVertexColors = CapturingReality::ImportExport::VertexColorType::PCT_COLOR_PER_VERTEX;
	}
	else
	{
		settings.exportVertexColors = CapturingReality::ImportExport::VertexColorType::PCT_COLOR_NONE;
	};

	if (pMvsModel->IsColored() == false)
	{
		settings.exportVertexColors = CapturingReality::ImportExport::VertexColorType::PCT_COLOR_NONE;
	};

	settings.exportVertexNormals = true;
	settings.exportVertices = true;
	settings.meshColor = 0xff0000ff;
	settings.oneTextureMaxSide = 8192;
	settings.oneTextureUsePow2TexSide = true;
	settings.settingsAnchor.x = 0.0;
	settings.settingsAnchor.y = 0.0;
	settings.settingsAnchor.z = 0.0;
	settings.settingsScalex = 1.0;
	settings.settingsScaley = 1.0;
	settings.settingsScalez = 1.0;
	settings.tileType = CapturingReality::ImportExport::UVTileType::UVTT_TILE_U1V1;

	wcscpy_s(settings.texturesFileType, L"jpg");
	wcscpy_s(settings.formatAndVersionUID, L"obj 000 ");

	settings.exportModelByParts = false;
	settings.exportRandomPartColor = false;

	settings.exportCameras = false;
	settings.exportCamerasAsModelPart = false;

	strcpy_s(settings.numberAsciiFormatting, "%.16e");

	HRESULT hr = S_FALSE;
	CComPtr< CapturingReality::ImportExport::IExportTransformationsProvider > spExportTransformationsProvider = pTransform;
	if (!spExportTransformationsProvider)
	{
		hr = GetGridExportTransformationProvider(pSfmReconstruction, pMvsModel, &spExportTransformationsProvider);
	}
	if (SUCCEEDED(hr))
	{
		CComPtr< CapturingReality::ImportExport::IReconstructionExporter > spExporter;
		hr = CapturingReality::ImportExport::CreateReconstructionExporterObj(dir, name, name, &settings, &spExporter);
		if (SUCCEEDED(hr))
		{
			hr = CapturingReality::ModelTools::ExportModel(spExporter, pMvsModel, spExportTransformationsProvider, NULL, NULL, NULL);
		};
	};

	return hr;
};

HRESULT ExportModelXYZ(
	__in_z const wchar_t *dir,
	__in_z const wchar_t *name,
	__in CapturingReality::IResourceCache *pResourceCache,
	__in CapturingReality::Sfm::IStructureFromMotion *pSfm,
	__in CapturingReality::Sfm::ISfmReconstruction *pSfmReconstruction,
	__in CapturingReality::Mvs::IMvsModel *pMvsModel,
	__in_opt CapturingReality::ImportExport::IExportTransformationsProvider *pTransform)
{
	CapturingReality::ImportExport::XmlModelExportParams settings = { 0 };
	settings.pAuthorComment = L"Terra Drone Corporation Code";
	settings.exportBinary = false;
	settings.exportInfoFile = true;
	settings.exportTextureAlpha = false;
	settings.exportToOneTexture = false;
	settings.exportTriangles = true;

	settings.exportTexturing = pMvsModel->IsTextured();
	//if (pMvsModel->IsTextured() == false)
	//{
		settings.exportVertexColors = CapturingReality::ImportExport::VertexColorType::PCT_COLOR_PER_VERTEX;
	//}
	//else
	//{
	//	settings.exportVertexColors = CapturingReality::ImportExport::VertexColorType::PCT_COLOR_NONE;
	//};

	//if (pMvsModel->IsColored() == false)
	//{
	//	settings.exportVertexColors = CapturingReality::ImportExport::VertexColorType::PCT_COLOR_NONE;
	//};

	settings.exportVertexNormals = false;
	settings.exportVertices = true;
	settings.meshColor = 0xff0000ff;
	settings.oneTextureMaxSide = 16384;
	settings.oneTextureUsePow2TexSide = true;
	settings.settingsAnchor.x = 0.0;
	settings.settingsAnchor.y = 0.0;
	settings.settingsAnchor.z = 0.0;
	settings.settingsScalex = 1.0;
	settings.settingsScaley = 1.0;
	settings.settingsScalez = 1.0;
	settings.tileType = CapturingReality::ImportExport::UVTileType::UVTT_TILE_U1V1;

	settings.exportCoordinateSystemType = CapturingReality::CalibrationExport::ExportCoordinateSystemType::ECST_PROJECT_OUTPUT;

	wcscpy_s(settings.texturesFileType, L"png");
	wcscpy_s(settings.formatAndVersionUID, L"xyz 000 ");

	settings.exportModelByParts = false;
	settings.exportRandomPartColor = false;

	settings.exportCameras = false;
	settings.exportCamerasAsModelPart = false;

	strcpy_s(settings.numberAsciiFormatting, "%.16e");	

	HRESULT hr = S_FALSE;

	CComPtr< CapturingReality::CoordinateSystems::ICoordinateSystem > spGpsCoordSystem;	
	hr = CreateCoordinateSystem(g_epsg_2451_name, g_epsg_2451_definition, &spGpsCoordSystem);

	CComPtr< CapturingReality::ImportExport::IExportTransformationsProvider > spExportTransformationsProvider = pTransform;
	if (!spExportTransformationsProvider)
	{		
			hr = GetGridExportTransformationProviderWithOutputCoordSys(pSfmReconstruction, pMvsModel, g_epsg_2451_definition, g_epsg_2451_name, &spExportTransformationsProvider);
	}
	if (SUCCEEDED(hr))
	{
		CComPtr< CapturingReality::ImportExport::IReconstructionExporter > spExporter;
		hr = CapturingReality::ImportExport::CreateReconstructionExporterXyz(dir, name, &settings, &spExporter);
		if (SUCCEEDED(hr))
		{
			hr = CapturingReality::ModelTools::ExportModel(spExporter, pMvsModel, spExportTransformationsProvider, NULL, NULL, NULL);
		};
	};

	return hr;
};

HRESULT ExportModelXYZGPS(
	__in_z const wchar_t *dir,
	__in_z const wchar_t *name,
	__in CapturingReality::IResourceCache *pResourceCache,
	__in CapturingReality::Sfm::IStructureFromMotion *pSfm,
	__in CapturingReality::Sfm::ISfmReconstruction *pSfmReconstruction,
	__in CapturingReality::Mvs::IMvsModel *pMvsModel,
	__in_opt CapturingReality::ImportExport::IExportTransformationsProvider *pTransform)
{
	CapturingReality::ImportExport::XmlModelExportParams settings = { 0 };
	settings.pAuthorComment = L"Terra Drone Corporation Code";
	settings.exportBinary = false;
	settings.exportInfoFile = true;
	settings.exportTextureAlpha = false;
	settings.exportToOneTexture = false;
	settings.exportTriangles = true;

	settings.exportTexturing = pMvsModel->IsTextured();
	settings.exportVertexColors = CapturingReality::ImportExport::VertexColorType::PCT_COLOR_PER_VERTEX;
	
	settings.exportVertexNormals = false;
	settings.exportVertices = true;
	settings.meshColor = 0xff0000ff;
	settings.oneTextureMaxSide = 16384;
	settings.oneTextureUsePow2TexSide = true;
	settings.settingsAnchor.x = 0.0;
	settings.settingsAnchor.y = 0.0;
	settings.settingsAnchor.z = 0.0;
	settings.settingsScalex = 1.0;
	settings.settingsScaley = 1.0;
	settings.settingsScalez = 1.0;
	settings.tileType = CapturingReality::ImportExport::UVTileType::UVTT_TILE_U1V1;

	settings.exportCoordinateSystemType = CapturingReality::CalibrationExport::ExportCoordinateSystemType::ECST_GRID_PLANE;

	wcscpy_s(settings.texturesFileType, L"png");
	wcscpy_s(settings.formatAndVersionUID, L"xyz 000 ");

	settings.exportModelByParts = false;
	settings.exportRandomPartColor = false;

	settings.exportCameras = false;
	settings.exportCamerasAsModelPart = false;

	strcpy_s(settings.numberAsciiFormatting, "%.16e");

	HRESULT hr = S_FALSE;

	CComPtr< CapturingReality::CoordinateSystems::ICoordinateSystem > spGpsCoordSystem;
	hr = CreateCoordinateSystem(g_gps_name, g_gps_definition, &spGpsCoordSystem);

	CComPtr< CapturingReality::ImportExport::IExportTransformationsProvider > spExportTransformationsProvider = pTransform;
	if (!spExportTransformationsProvider)
	{
		hr = GetGridExportTransformationProvider(pSfmReconstruction, pMvsModel, &spExportTransformationsProvider);
	}
	if (SUCCEEDED(hr))
	{
		CComPtr< CapturingReality::ImportExport::IReconstructionExporter > spExporter;
		hr = CapturingReality::ImportExport::CreateReconstructionExporterXyz(dir, name, &settings, &spExporter);
		if (SUCCEEDED(hr))
		{
			hr = CapturingReality::ModelTools::ExportModel(spExporter, pMvsModel, spExportTransformationsProvider, NULL, NULL, NULL);
		};
	};

	return hr;
};

HRESULT ExportModelXYZ_old(__in_z const WCHAR *pOutputDir,
	__in_z const WCHAR *pFileName,
	__in CapturingReality::Sfm::ISfmReconstruction *pSfmReconstruction,
	__in CapturingReality::Mvs::IMvsModel *pModel)
{
	if (!pOutputDir || !pFileName || !pModel || !pSfmReconstruction)
	{
		return E_INVALIDARG;
	}

	std::wstring out_dir(pOutputDir);
	std::wstring xyz_file(pFileName);
	std::wstring xyz_fullpath(out_dir);
	xyz_fullpath.append(xyz_file);

	FILE *pFile;
	//errno_t err = _wfopen_s( &pFile, pFileName, L"w" );
	errno_t err = _wfopen_s(&pFile, xyz_fullpath.c_str(), L"w");
	if (err)
	{
		return E_FAIL;
	}

	HRESULT hr = S_FALSE;

	// model coordinate system can be shifted compared to the sfm component coordinate system if 
	// the model was calculated prior to the model geo-referencing using a rigid transform,
	// i.e., call of IStructureFromMotion::UpdateConstraints.
	// To compensate this difference a residual transformation must be applied.
	CoordinateSystemResidual transform = pModel->GetResidualTransform();

	CapturingReality::CoordinateSystemResidual rootToModel = pModel->GetResidualTransform();
	CapturingReality::CoordinateSystemResidual rootToActual = pSfmReconstruction->GetResidualTransform();
	CapturingReality::CoordinateSystemResidual modelSfmToActualSfmTransform = GetModelToActualTransform(&rootToModel, &rootToActual);

	// export all model parts
	UINT partsCount = pModel->GetPartsCount();
	for (UINT i = 0; i < partsCount; i++)
	{
		CComPtr< IMvsModelPart > spPart;
		hr = pModel->GetPart(i, &spPart);
		if (FAILED(hr))
		{
			break;
		}

		CComPtr< ISceneTriangulation > spGeometry;
		hr = spPart->LoadTriangulation(&spGeometry);
		if (FAILED(hr))
		{
			break;
		}

		CoordinateSystemAnchor *pAnchor = spGeometry->GetAnchor();
		_ASSERT(pAnchor);

		UINT pointsCount;
		CapturingReality::LocalPoint *pPoints;
		hr = spGeometry->GetPoints(&pPoints, &pointsCount);
		if (FAILED(hr))
		{
			break;
		}

		CoordinateSystemResidual modelCStoModelSfmTransform;
		modelCStoModelSfmTransform.s = 1.0f;
		modelCStoModelSfmTransform.t[0] = pAnchor->x;
		modelCStoModelSfmTransform.t[1] = pAnchor->y;
		modelCStoModelSfmTransform.t[2] = pAnchor->z;
		pModel->GetModelBBoxToGCSRotation(modelCStoModelSfmTransform.R);

		CoordinateSystemResidual modelCStoActualSfm = TransformResidualTransform(&modelSfmToActualSfmTransform, &modelCStoModelSfmTransform);

		double M[12] =
		{ modelCStoActualSfm.R[0] * modelCStoActualSfm.s, modelCStoActualSfm.R[1] * modelCStoActualSfm.s, modelCStoActualSfm.R[2] * modelCStoActualSfm.s, modelCStoActualSfm.t[0],
			modelCStoActualSfm.R[3] * modelCStoActualSfm.s, modelCStoActualSfm.R[4] * modelCStoActualSfm.s, modelCStoActualSfm.R[5] * modelCStoActualSfm.s, modelCStoActualSfm.t[1],
			modelCStoActualSfm.R[6] * modelCStoActualSfm.s, modelCStoActualSfm.R[7] * modelCStoActualSfm.s, modelCStoActualSfm.R[8] * modelCStoActualSfm.s, modelCStoActualSfm.t[2] };

		for (UINT p = 0; p < pointsCount; p++)
		{
			double x[3];
			x[0] = pPoints[p].pt.x;
			x[1] = pPoints[p].pt.y;
			x[2] = pPoints[p].pt.z;

			double y[3];
			MulM34V3(M, x, y);

			int err = fprintf(pFile, "%.6f %.6f %.6f\r\n", y[0], y[1], y[2]);
			if (err < 0)
			{
				hr = E_FAIL;
				break;
			}
		}
	}

	return hr;
}