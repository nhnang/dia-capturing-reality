#include "stdafx.h"
#include <iostream>
#include <locale>
#include <vector>
#include <string>
#include <fstream>
#include "exportUtil.h"
#include "CubeDistributionAnalyzer.h"


///////////////////////////////////////////////////////////////////////////////
// ndef
#ifndef _ENABLE_CR_BASE_CODE_

// computed Bounding Box in SDK
extern double bbox_min[3];
extern double bbox_max[3];

using namespace std;

// Cube reduction analyze
CubeDistributionAnalyzer* g_analyzer;

const double RAD_TO_DEG = (180 / 3.1415926535897932384626433832795);

void CreateExportParams( __out CapturingReality::ImportExport::XmlModelExportParams *pSettings )
{
    ZeroMemory( pSettings, sizeof(CapturingReality::ImportExport::XmlModelExportParams) );

	pSettings->exportBinary = true;
	pSettings->exportInfoFile = true;	
	pSettings->exportVertices = true;
	pSettings->exportVertexColors = CapturingReality::ImportExport::VertexColorType::PCT_COLOR_PER_VERTEX;
	pSettings->exportVertexNormals = false;
	pSettings->exportTriangles = true;
	pSettings->exportTexturing = false;
	pSettings->meshColor = 0xff0000ff;
	pSettings->tileType = CapturingReality::ImportExport::UVTileType::UVTT_TILE_U1V1;	
	pSettings->exportTextureAlpha = false;
	pSettings->exportToOneTexture = true;
	pSettings->oneTextureMaxSide = 16384;
	pSettings->oneTextureUsePow2TexSide = true;

    pSettings->exportCoordinateSystemType = CapturingReality::CalibrationExport::ExportCoordinateSystemType::ECST_GRID_PLANE;
	pSettings->settingsAnchor.x = 0.0;
	pSettings->settingsAnchor.y = 0.0;
	pSettings->settingsAnchor.z = 0.0;
	pSettings->settingsScalex = 1.0;
	pSettings->settingsScaley = 1.0;
	pSettings->settingsScalez = 1.0;

	wcscpy_s( pSettings->texturesFileType, L"png" );
	wcscpy_s( pSettings->formatAndVersionUID, L"ply 000 " );

	pSettings->exportModelByParts = false;
	pSettings->exportRandomPartColor = false;

	pSettings->exportCameras = false;
	pSettings->exportCamerasAsModelPart = false;

	strcpy_s( pSettings->numberAsciiFormatting, "%.16e" );
}

HRESULT ExportReconstructionToPly(
	__in_z const WCHAR *pFolderLocation,
	__in_z const WCHAR *pFileName,
	__in CapturingReality::Sfm::ISfmReconstruction *pRec )
{
    if ( !pFolderLocation || !pFileName || !pRec )
    {
        return E_INVALIDARG;
    }

    // setup PLY export defaults
	CapturingReality::ImportExport::XmlModelExportParams settings;
    CreateExportParams( &settings );
	
	CComPtr< CapturingReality::ImportExport::IReconstructionExporter > spExporter;
	HRESULT hr = CapturingReality::ImportExport::CreateReconstructionExporterPly( pFolderLocation, pFileName, &settings, &spExporter );
    if ( FAILED(hr) )
    {
        return hr;
    }

    CapturingReality::Sfm::ISfmCameraModel *pCameraModel = pRec->GetCameraModel();
    if ( !pCameraModel )
    {
        return E_UNEXPECTED;
    }

    UINT camerasCount = pRec->GetCamerasCount();

    UINT pointsCount;
    CapturingReality::Sfm::SfmReconstructionPoint *pPoints;
    hr = pRec->GetStructure( &pPoints, &pointsCount);
    if ( FAILED(hr) )
    {
        return hr;
    }

    // 
    CapturingReality::ImportExport::ReconstructionExportHeader head = {0};
    head.hasVertexColors = true;
    head.partsCount = 1;
    head.totalVertices = camerasCount + pointsCount;

	wprintf(L"base ply total vatex count =%d\n",head.totalVertices); 

    hr = spExporter->OnBeginExport( &head );
    if ( FAILED(hr) )
    {
        return hr;
    }
    
    // export camera centers
    for ( UINT i = 0; i < camerasCount; i++ )
    {
        CapturingReality::SfmCamera camera;
        hr = pRec->GetCamera( i, &camera );
        if ( FAILED(hr) )
        {
            return hr;
        }

        double C[4];
        pCameraModel->GetCameraCentre( camera, C );

        // export camera centre as a green dot
        // hint - export more cameras at once, its faster
        CapturingReality::CoordinateSystemPoint point;
        point.x = C[0] / C[3];
        point.y = C[1] / C[3];
        point.z = C[2] / C[3];
        unsigned int color = 0xff00f000;
        XMVECTOR normal = {0};
        hr = spExporter->ExportVerticesData( 1, &point, &color, &normal );
        if ( FAILED(hr) )
        {
            return hr;
        }
    }

    // export points (hint: export more points at once, not one-by-one)
	int reduce_count = 0;
    for ( UINT i = 0; i < pointsCount; i++ )
    {
        CapturingReality::CoordinateSystemPoint point;
        point.x = pPoints[i].X.x / pPoints[i].X.w;
        point.y = pPoints[i].X.y / pPoints[i].X.w;
        point.z = pPoints[i].X.z / pPoints[i].X.w;

		// reduced bbox
		if (!g_analyzer->isInsideReducedBbox(&point.x, &point.y, &point.z))
		{
			reduce_count ++;
//			continue;
		}
		
		unsigned int color = pPoints[i].color;
        XMVECTOR normal = {0};
        hr = spExporter->ExportVerticesData( 1, &point, &color, &normal );
        if ( FAILED(hr) )
        {
            return hr;
        }
    }

	hr = spExporter->OnEndExport();

    return hr;
}

HRESULT ExportReconstructionToXYZ(
	__in_z const WCHAR *pOutputDir,
	__in_z const WCHAR *pXyzFileName,
	__in CapturingReality::Sfm::ISfmReconstruction *pRec )
{
    if ( !pOutputDir || !pXyzFileName || !pRec )
    {
        return E_INVALIDARG;
    }

    CapturingReality::Sfm::ISfmCameraModel *pCameraModel = pRec->GetCameraModel();
    if ( !pCameraModel )
    {
        return E_UNEXPECTED;
    }

    UINT camerasCount = pRec->GetCamerasCount();

    UINT pointsCount;
    CapturingReality::Sfm::SfmReconstructionPoint *pPoints;
    HRESULT hr = pRec->GetStructure( &pPoints, &pointsCount);
    if ( FAILED(hr) )
    {
        return hr;
    }

	std::wstring out_dir(pOutputDir);
	std::wstring xyz_file(pXyzFileName);
	std::wstring xyz_fullpath(out_dir);
	xyz_fullpath.append(xyz_file);

	FILE *pFile;
	errno_t err = _wfopen_s( &pFile, xyz_fullpath.c_str(), L"w" );
    if ( err )
    {
        return E_FAIL;
    }

	// analyze cube class
	g_analyzer = new CubeDistributionAnalyzer(pOutputDir);
	g_analyzer->setBoundingBox(bbox_min, bbox_max);

/*	Comment out as camera position is unnecessary.
    // export camera centers
    for ( UINT i = 0; i < camerasCount; i++ )
    {
        CapturingReality::SfmCamera camera;
        hr = pRec->GetCamera( i, &camera );
        if ( FAILED(hr) )
        {
            break;
        }

        double C[4];
        pCameraModel->GetCameraCentre( camera, C );
		// xyz / scale
        C[0] /= C[3];
        C[1] /= C[3];
        C[2] /= C[3];

		// analyze cube
		if (!analyzer->analyzePointsCemara(i, &C[0], &C[1], &C[2], &C[3]))
		{
			wprintf(L"failure analyze camera points.\n");
            hr = E_FAIL;
			break;
		}

		// output xyz
        err = fprintf( pFile, "%.6f %.6f %.6f 0 255 0\r\n", C[0], C[1], C[2] );
        if ( err < 0 )
        {
            hr = E_FAIL;
            break;
        }
    }
*/
	// analyze cube
	if (!g_analyzer->analyzePoints(pPoints, pointsCount))
	{
		wprintf(L"failure analyze points.\n");
		hr = E_FAIL;
	}
	double resized_min[3];
	double resized_max[3];
	if (!g_analyzer->resizeBoundingBox(resized_min, resized_max))
	{
		wprintf(L"Failure resize bounding box.\n");
	}
	wprintf(L"[Resized Bbox] min x=%f, y=%f z=%f", resized_min[0], resized_min[1], resized_min[2]);
	wprintf(L"[Resized Bbox] max x=%f, y=%f z=%f", resized_max[0], resized_max[1], resized_max[2]);
	g_analyzer->report();

	// resized xyz
	ofstream resized_out("resized_xyz.txt");
	resized_out.precision(6);

	// xyz
    if ( SUCCEEDED(hr) )
    {
        // export points
        for ( UINT i = 0; i < pointsCount; i++ )
        {
            double C[3];
            C[0] = pPoints[i].X.x / pPoints[i].X.w;
            C[1] = pPoints[i].X.y / pPoints[i].X.w;
            C[2] = pPoints[i].X.z / pPoints[i].X.w;

			if (g_analyzer->isInsideReducedBbox(&C[0], &C[1], &C[2]))
			{
				UINT8 r = (UINT8)pPoints[i].color;
				UINT8 g = (UINT8)(pPoints[i].color>>8);
				UINT8 b = (UINT8)(pPoints[i].color>>16);
				resized_out << C[0] << " " << C[1] << " " << C[2] << " " << +r << " " << +g << " " << +b << endl;
			}

            err = fprintf( pFile, "%.6f %.6f %.6f %d %d %d\r\n", C[0], C[1], C[2], (byte)pPoints[i].color, (byte)(pPoints[i].color>>8), (byte)(pPoints[i].color>>16) );
            if ( err < 0 )
            {
                hr = E_FAIL;
                break;
            }

        }
    }
	resized_out.close();

	fclose( pFile );
//	delete analyzer;

    return hr;
}

HRESULT ExportModelXYZ( __in_z const WCHAR *pFileName, __in CapturingReality::Sfm::ISfmReconstruction *pSfmReconstruction, __in CapturingReality::Mvs::IMvsModel *pModel )
{
    if ( !pFileName || !pModel || !pSfmReconstruction )
    {
        return E_INVALIDARG;
    }

    FILE *pFile;
    errno_t err = _wfopen_s( &pFile, pFileName, L"w" );
    if ( err )
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
	CapturingReality::CoordinateSystemResidual modelSfmToActualSfmTransform = GetModelToActualTransform( &rootToModel, &rootToActual );

    // export all model parts
    UINT partsCount = pModel->GetPartsCount();
    for ( UINT i = 0; i < partsCount; i++ )
    {
        CComPtr< IMvsModelPart > spPart;
        hr = pModel->GetPart( i, &spPart );
        if ( FAILED(hr) )
        {
            break;
        }

        CComPtr< ISceneTriangulation > spGeometry;
        hr = spPart->LoadTriangulation( &spGeometry );
        if ( FAILED(hr) )
        {
            break;
        }

        CoordinateSystemAnchor *pAnchor = spGeometry->GetAnchor();
        _ASSERT( pAnchor );

        UINT pointsCount;
        CapturingReality::LocalPoint *pPoints;
        hr = spGeometry->GetPoints( &pPoints, &pointsCount );
        if ( FAILED(hr) )
        {
            break;
        }

		CoordinateSystemResidual modelCStoModelSfmTransform;
		modelCStoModelSfmTransform.s = 1.0f;
		modelCStoModelSfmTransform.t[0] = pAnchor->x;
		modelCStoModelSfmTransform.t[1] = pAnchor->y;
		modelCStoModelSfmTransform.t[2] = pAnchor->z;
		pModel->GetModelBBoxToGCSRotation( modelCStoModelSfmTransform.R );

		CoordinateSystemResidual modelCStoActualSfm = TransformResidualTransform( &modelSfmToActualSfmTransform, &modelCStoModelSfmTransform );

		double M[12] =
			{ modelCStoActualSfm.R[0] * modelCStoActualSfm.s, modelCStoActualSfm.R[1] * modelCStoActualSfm.s, modelCStoActualSfm.R[2] * modelCStoActualSfm.s, modelCStoActualSfm.t[0],
				modelCStoActualSfm.R[3] * modelCStoActualSfm.s, modelCStoActualSfm.R[4] * modelCStoActualSfm.s, modelCStoActualSfm.R[5] * modelCStoActualSfm.s, modelCStoActualSfm.t[1],
				modelCStoActualSfm.R[6] * modelCStoActualSfm.s, modelCStoActualSfm.R[7] * modelCStoActualSfm.s, modelCStoActualSfm.R[8] * modelCStoActualSfm.s, modelCStoActualSfm.t[2] };

        for ( UINT p = 0; p < pointsCount; p++ )
        {
            double x[3];
            x[0] = pPoints[p].pt.x;
            x[1] = pPoints[p].pt.y;
            x[2] = pPoints[p].pt.z;

            double y[3];
            MulM34V3( M, x, y ); 

            int err = fprintf( pFile, "%.6f %.6f %.6f\r\n", y[0], y[1], y[2] );
            if ( err < 0 )
            {
                hr = E_FAIL;
                break;
            }
        }
    }

    return hr;
}

#endif