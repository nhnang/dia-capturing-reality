//
// Capturing Reality s.r.o.
//
// This file 'OrthoExample.cpp' is a part of the RC Engine Samples. You may use it freely.
//

#include "stdafx.h"
#include "define.h"
#include "SfmUtils.h"
#include "ControlPoints.h"
#include "..\SamplesCommon\TransformationHelpers.h"

// ndef
#ifndef _ENABLE_CR_BASE_CODE_

#include "w84gps.h"
#include "CubeDistributionAnalyzer.h"
#include "CameraAngle.h"
#include "exportUtil.h"
#include "param.h"
#include <vector>
#include <cstdio>
#include <iostream>
#include <locale>
#include <vector>
#include <string>
#include <fstream>

using namespace std;

#endif _ENABLE_CR_BASE_CODE_

//
// This example shows how to obtain an orthographic projection from input images using the RC Engine.
//
// A geo-referenced component is required. We use ground control points (see ControlPoints.cpp)
// to geo-reference the component in this example. Another way of geo-referencing a component
// is to use a flight log or images in exif format containing GPS coordinates. Note, however,
// that these are not shown in this example.
//
//      Scheme:
//          * activate a license
//          * create cache
//          * obtain a component
//          * create a model
//          * colorize and calculate the model texture
//          * create and export an ortho projection (epsg:3016)
//          * create and export the ortho projection (gps)
//

//
// Inline definitions of coordinate systems used in this example.
//

///////////////////////////////////////////////////////////////////////////////
#ifdef _ENABLE_CR_BASE_CODE_

const wchar_t* g_epsg_3016_name = L"SWEREF99 20 15";
const wchar_t* g_epsg_3016_definition = L"+proj=tmerc +lat_0=0 +lon_0=20.25 +k=1 +x_0=150000 +y_0=0 +ellps=GRS80 +towgs84=0,0,0,0,0,0,0 +units=m +no_defs";

const wchar_t* g_gps_name = L"GPS (WGS 84)";
const wchar_t* g_gps_definition = L"+proj=longlat +datum=WGS84 +no_defs";

#endif // _ENABLE_CR_BASE_CODE_
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
// ndef
#ifndef _ENABLE_CR_BASE_CODE_

// global
FILE* fp;

// Global import val
const wchar_t* g_epsg_3016_name;
const wchar_t* g_epsg_3016_definition;
const wchar_t* g_gps_name;
const wchar_t* g_gps_definition;

// Global import val
double north;
double south;
double east;
double west;

// Global export Image detail
UINT projectionWidth = 1400;
UINT projectionHeight = 1400;
double projectionDepth = 100.0;

const int EPSG_LIST_LINE_COUNT = 10;

// computed Bounding Box in SDK
double bbox_min[3];
double bbox_max[3];

#endif // _ENABLE_CR_BASE_CODE_
///////////////////////////////////////////////////////////////////////////////

//
//

int _tmain(int argc, _TCHAR* argv[])
{
    HRESULT hr = ActivateSdk();

///////////////////////////////////////////////////////////////////////////////
// ndef
#ifndef _ENABLE_CR_BASE_CODE_

	// 引数チェック
	if ((argc != 4) && (argc != 5))
	{
		wprintf(L"\nusage:  >MeshingExample.exe c:\\input\\ c:\\output\\ c:\\param\\ -nocache\n");
		exit(-1);
	}
	if (!PathFileExists(argv[1]))
	{
		wprintf(L"\nInput Image dir %ls is not found.\n", argv[1]);
		wprintf(L"\nusage:  >MeshingExample.exe c:\\input\\ c:\\output\\ c:\\param\\ -nocache\n");
		exit(-2);
	}
	if (!PathFileExists(argv[2]))
	{
		wprintf(L"\nOutput Sfm dir %ls is not found.\n", argv[2]);
		wprintf(L"\nusage:  >MeshingExample.exe c:\\input\\ c:\\output\\ c:\\param\\ -nocache\n");
		exit(-3);
	}
	if (!PathFileExists(argv[3]))
	{
		wprintf(L"\nParam dir %ls is not found.\n", argv[3]);
		wprintf(L"\nusage:  >MeshingExample.exe c:\\input\\ c:\\output\\ c:\\param\\ -nocache\n");
		exit(-4);
	}

	bool is_use_cache = true;
	if (argc == 5)
	{
		wstring cache_str(argv[4]);
		if (cache_str.compare(L"-nocache") == 0)
		{
			is_use_cache = false;
		}
		else
		{
			wprintf(L"\nusage:  >MeshingExample.exe c:\\input\\ c:\\output\\ c:\\param\\ -nocache\n");
			wprintf(L"is image width and height.");
			wprintf(L"-nocache is option.");
			exit(-5);
		}
	}

	// Get CRSDK Engine Version
    ServiceConnectionContext connectionContext;
    strcpy_s( connectionContext.appToken, SDKSAMPLE_APP_TOKEN );
    CComPtr< IEngineOnlineServices > spOnlineServices;
    hr = CreateEngineOnlineServices( &connectionContext, &spOnlineServices );
    EngineVersionInfo newVersionInfo = {0};
    hr = spOnlineServices->GetEngineUpdateInfo( &newVersionInfo );
    if ( SUCCEEDED(hr) )
    {
        UINT major, minor, sub, build;
        CapturingReality::RealityCaptureEngine::GetEngineVersion( &major, &minor, &sub, &build );
		printf("[Info] CRSDK Version %d.%d.%d build=%d\n", major, minor, sub, build);

        bool bApiChanged = (newVersionInfo.major > major) || (newVersionInfo.minor > minor) || (newVersionInfo.sub > sub);
        if ( bApiChanged || (newVersionInfo.build > build) )
        {
            printf( "A new version of RealityCapture Engine is available.\n\n" );
        }
        else
        {
            printf( "Your RealityCapture Engine libraries are up to date.\n\n" );
        }
        if ( bApiChanged )
        {
            printf( "[ You will need to re-compile your application since there are changes in API which might made DLL files incompatible with this version. ]\n\n" );
        }
    }

	size_t input_strlen = wcslen(argv[1]);
	input_strlen = (input_strlen + 2) * sizeof(wchar_t);
	wchar_t p_input_dir[MAX_PATH];
	wcscpy_s(p_input_dir, input_strlen, argv[1]);
	PathAddBackslashW(p_input_dir);

	// output dir BackSlash
	size_t output_strlen = wcslen(argv[1]);
	output_strlen = (input_strlen + 2) * sizeof(wchar_t);
	wchar_t p_output_dir[MAX_PATH];
	wcscpy_s(p_output_dir, output_strlen, argv[2]);
	PathAddBackslashW(p_output_dir);

	// param dir BackSlash
	size_t param_strlen = wcslen(argv[1]);
	param_strlen = (input_strlen + 2) * sizeof(wchar_t);
	wchar_t p_param_dir[MAX_PATH];
	wcscpy_s(p_param_dir, param_strlen, argv[3]);
	PathAddBackslashW(p_param_dir);

	// SfmExample log out
	errno = fopen_s(&fp, "SfmExample_log.txt","w");

	//////////////////////////////////////////////////////////////
	// epsg データロード

	// 測地系の情報を読み込みます。
	wchar_t* epsg_list[EPSG_LIST_LINE_COUNT];
	int epsg_count = 
		loadEpsgList(
			p_param_dir,
			L"EpsgList.txt",
			epsg_list,
			EPSG_LIST_LINE_COUNT);

	// EPSGファイルは必ずEPSG_LIST_LINE_COUNT行である必要があります。
	if (epsg_count != EPSG_LIST_LINE_COUNT)
	{
		wprintf(L"EpsgList.txt file needs %d lines data.", EPSG_LIST_LINE_COUNT);
		exit(-10);
	}

	// 2番目のデータは直書きではなく番号指定になりました。
	// 番号から変換します。
	int wgs_number = _wtoi(epsg_list[1]);
	const wchar_t* pp_w84 = nullptr;
	if (!toWgs84(wgs_number, &pp_w84))
	{
		wprintf(L"WGS number is invalid val=%d", wgs_number);
		exit(-11);
	}

	// EPSGデータに取り込みます
	g_epsg_3016_name = epsg_list[0];

	// OrthoExample recover test
	g_epsg_3016_definition = pp_w84;

	g_gps_name = epsg_list[2];
	g_gps_definition = epsg_list[3];

	// 出力画像サイズを設定します。
	projectionWidth = _wtoi(epsg_list[7]);
    projectionHeight = _wtoi(epsg_list[8]);
    projectionDepth = _wtof(epsg_list[9]);

	// WARNING!
	// 下の方でこの変数をまだ使っています。破棄は最後で。

#endif // _ENABLE_CR_BASE_CODE_
///////////////////////////////////////////////////////////////////////////////

    //
    // __ create cache __
    //

    // Creating cache is optional for Sfm creation process but required for Mvs / Multipart meshing.

    CComPtr< CapturingReality::IConfig > spConfig;
    CComPtr< CapturingReality::IResourceCache > spCache;

    if ( SUCCEEDED( hr ) )
    {
        hr = CapturingReality::Utilities::CreateResourceCache( CapturingReality::RCL_CUSTOM, L"_crTmp", &spCache );
    }

    if ( SUCCEEDED( hr ) )
    {
        hr = CapturingReality::Utilities::CreateConfig( &spConfig );
    }

    //
    // __ obtain a component __
    //

    // The component has to be geo-referenced.

    // Create epsg:3016 coordinate system. The definition can be found in ..\Resources\metadata\epsg.xml.

    CComPtr< CapturingReality::CoordinateSystems::ICoordinateSystem > spCoordinateSystem3016;

    if ( SUCCEEDED( hr ) )
    {
        hr = CapturingReality::CoordinateSystems::CreateCoordinateSystem( g_epsg_3016_name, g_epsg_3016_definition, &spCoordinateSystem3016 );
    }

    // Import the one from previous run if present or create a new one.

    CComPtr< CapturingReality::Sfm::IStructureFromMotion > spSfm;
    CComPtr< CapturingReality::Sfm::ISfmReconstruction > spSfmReconstruction;

///////////////////////////////////////////////////////////////////////////////
#ifdef _ENABLE_CR_BASE_CODE_

	if ( SUCCEEDED( hr ) )
    {
        hr = CreateOrthoExampleSfm( spCache, spConfig, spCoordinateSystem3016, &spSfm );
    }

///////////////////////////////////////////////////////////////////////////////
// ndef
#else

	if ( SUCCEEDED( hr ) )
    {
        hr = 
			CreateOrthoExampleSfm( 
				p_input_dir,
				p_param_dir,
				spCache, 
				spConfig, 
				spCoordinateSystem3016, 
				&spSfm );
    }
	if ( !SUCCEEDED( hr ) )
	{
        printf( "CreateOrthoExampleSfm error HRESULT=%d \n", hr );
		OutputDebugString(L"CreateOrthoExampleSfm error.\n");
		HrToStr(hr);
		exit(-11);
	}

#endif // _ENABLE_CR_BASE_CODE_
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
#ifdef _ENABLE_CR_BASE_CODE_

    if ( SUCCEEDED( hr ) )
    {
        const wchar_t *pFileName = L"component0.rcalign";

        if ( PathFileExists( pFileName ) == TRUE )
        {
            printf( "Importing a component ... \n" );
            ImportRCComponent( pFileName,  spSfm, &spSfmReconstruction );
        }
        else
        {
            printf( "Aligning ... \n" );
            hr = CreateReconstruction( spSfm, &spSfmReconstruction );
            if ( SUCCEEDED( hr ) )
            {
                printf( "Exporting a component ... \n" );
                hr = ExportRCComponent( pFileName, spSfm, spSfmReconstruction );
            }
        }
    }

///////////////////////////////////////////////////////////////////////////////
#else	// ndef

    if ( SUCCEEDED( hr ) )
    {
		const wchar_t *pFileName = L"component0.rcalign";
		if (is_use_cache)
		{
	        if ( PathFileExists( pFileName ) == TRUE )
	        {
	            printf( "Importing component ... \n" );
	            ImportRCComponent( pFileName,  spSfm, &spSfmReconstruction );
	        }
			else
			{
				printf( "Aligning ... \n" );
				hr = CreateReconstruction( spSfm, &spSfmReconstruction );
				if ( SUCCEEDED( hr ) )
				{
					printf( "Exporting component ... \n" );
					hr = ExportRCComponent( pFileName, spSfm, spSfmReconstruction );
				}
			}
		}
        else
        {
            printf( "Aligning ... \n" );
            hr = CreateReconstruction( spSfm, &spSfmReconstruction );
            if ( SUCCEEDED( hr ) )
            {
                printf( "Exporting component ... \n" );
                hr = ExportRCComponent( pFileName, spSfm, spSfmReconstruction );
            }
        }
    }

#endif // _ENABLE_CR_BASE_CODE_
///////////////////////////////////////////////////////////////////////////////

	//
    // __ create a model __
    //

    // Create a model within a reconstruction region. The reconstruction region is in the epsg:3016 coordinate system.

    // Create a reconstruction region.

    GlobalReconstructionVolume region;

///////////////////////////////////////////////////////////////////////////////
#ifdef _ENABLE_CR_BASE_CODE_

	CoordinateSystemPoint centre3016;
    double whd[3]; // {width,height,depth} in global coordinate system

    if ( SUCCEEDED( hr ) )
    {
        // Center the reconstruction region directly in the epsg:3016 coordinate system.
        centre3016.x = 140519.981561353;
        centre3016.y = 7068782.89967679;
        centre3016.z = 97.4389834348112;

        // In native units of epsg:3016 (i.e. in meters).
        whd[0] = 100.0;
        whd[1] = 100.0;
        whd[2] = 25.0;
    }

    if ( SUCCEEDED( hr ) )
    {
        // Convert from the global coordinate system to the sfm coordinate system. The Sfm must be geo-referenced.
        hr = ConvertGcsBoxToSfmBox( centre3016, whd, spCoordinateSystem3016, spSfmReconstruction, &region );
    }

///////////////////////////////////////////////////////////////////////////////
// ndef
#else

	extern CoordinateSystemPoint centre3016;
	extern double whd[3];

	// whdデータもここで取り込みます
	whd[0] = _wtof(epsg_list[4]);
	whd[1] = _wtof(epsg_list[5]);
	whd[2] = _wtof(epsg_list[6]);

	if ( SUCCEEDED( hr ) )
    {
        // Convert from global coordinate system to sfm coordinate system. The Sfm must be geo-referenced.
        hr = 
			ConvertGcsBoxToSfmBox( 
				centre3016, 
				whd, 
				spCoordinateSystem3016, 
				spSfmReconstruction, 
				&region );
    }
    if ( !SUCCEEDED( hr ) )
	{
        printf( "ConvertGcsBoxToSfmBox error HRESULT=%d \n", hr );
		OutputDebugString(L"ConvertGcsBoxToSfmBox error.\n");
		HrToStr(hr);
		exit(-500);
	}

#endif	// _ENABLE_CR_BASE_CODE_
///////////////////////////////////////////////////////////////////////////////

    // Create a model.

    CComPtr< CapturingReality::Mvs::IMvsModel > spMvsModel;

    if ( SUCCEEDED( hr ) )
    {
        printf( "Computing a model ... \n" );
        hr = CreateModel( 2, &region, spCache, spConfig, spSfm, spSfmReconstruction, &spMvsModel );
    }

    if ( SUCCEEDED( hr ) )
    {
        printf( "Unlocking the model ... \n" );
        hr = UnlockModelIfNeeded( spSfm, spSfmReconstruction, spMvsModel );
    }

    //
    // __ colorize and calculate model the texture __
    //

    if (SUCCEEDED(hr))
    {
        printf( "Calculating the model vertex colors ... \n" );
        hr = ColorizeModel( 1, spCache, spSfm, spSfmReconstruction, spMvsModel );
    }

///////////////////////////////////////////////////////////////////////////////
#ifdef _ENABLE_CR_BASE_CODE_
    if ( SUCCEEDED( hr ) )
    {
        printf( "Exporting the model to ply ... \n" );
        hr = ExportModelPly( L"", L"model", spCache, spSfm, spSfmReconstruction, spMvsModel );
    }
///////////////////////////////////////////////////////////////////////////////
// ndef
#else
	if ( SUCCEEDED( hr ) )
    {
        printf( "Exporting model to ply ... \n" );
        hr = 
			ExportModelPly(
				p_output_dir,
				L"model",
				spCache, 
				spSfm, 
				spSfmReconstruction, 
				spMvsModel );
    }
	if (!SUCCEEDED(hr))
	{
        printf( "Ply export error HRESULT=%d \n", hr );
		OutputDebugString(L"Ply export error.\n");
		HrToStr(hr);
		exit(-520);
	}
#endif	// _ENABLE_CR_BASE_CODE_
///////////////////////////////////////////////////////////////////////////////
    CComPtr< CapturingReality::Mvs::IMvsModel > spMvsModelSimplified;
    if ( SUCCEEDED( hr ) )
    {
        printf( "Simplifying the model ... \n" );
        hr = SimplifyRelative( 0.1, spMvsModel, &spMvsModelSimplified );
    }

    if ( SUCCEEDED( hr ) )
    {
        printf( "Texturing the model ... \n" );
        hr = TextureModel( 1, spCache, spSfm, spSfmReconstruction, spMvsModelSimplified );
    }
///////////////////////////////////////////////////////////////////////////////
#ifdef _ENABLE_CR_BASE_CODE_
    if ( SUCCEEDED( hr ) )
    {
        printf( "Exporting the model to obj ... \n" );
        hr = ExportModelObj( L"", L"modelSimpl", spCache, spSfm, spSfmReconstruction, spMvsModelSimplified );
    }
///////////////////////////////////////////////////////////////////////////////
// ndef
#else
    if ( SUCCEEDED( hr ) )
    {
        printf( "Exporting model to obj ... \n" );
        hr = 
			ExportModelObj( 
				p_output_dir,
				L"modelSimpl",
				spCache, 
				spSfm, 
				spSfmReconstruction, 
				spMvsModelSimplified );
    }
	if (!SUCCEEDED(hr))
	{
        printf( "Obj export error HRESULT=%d \n", hr );
		OutputDebugString(L"Obj export error.\n");
		HrToStr(hr);
		exit(-530);
	}
#endif	// _ENABLE_CR_BASE_CODE_
///////////////////////////////////////////////////////////////////////////////

    //
    // __ create and export ortho projection (epsg:3016) __
    //

///////////////////////////////////////////////////////////////////////////////
#ifdef _ENABLE_CR_BASE_CODE_
	const UINT projectionWidth  = 1400; // in pixels
    const UINT projectionHeight = 1400; // in pixels
    const double projectionDepth = 100.0; // in meters
#endif	// _ENABLE_CR_BASE_CODE_
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// ndef
#ifndef _ENABLE_CR_BASE_CODE_
	// XYZ export code from ExportExmaple proj.
	if (SUCCEEDED(hr))
	{
		printf( "Exporting model to xyz ... \n" );
		// TODO:
		// literal filename
		hr =
			ExportReconstructionToXYZ(
				p_output_dir,
				L"model.xyz",
				spSfmReconstruction);
	}
	if (!SUCCEEDED(hr))
	{
        printf( "Xyz export error HRESULT=%d \n", hr );
		OutputDebugString(L"Xyz export error.\n");
		HrToStr(hr);
		exit(-540);
	}
#endif	// _ENABLE_CR_BASE_CODE_
///////////////////////////////////////////////////////////////////////////////
	if ( SUCCEEDED( hr ) )
    {
        const double pixelSizeInMetersX = ( 2.0 * whd[0] ) / projectionWidth;
        const double pixelSizeInMetersY = ( 2.0 * whd[1] ) / projectionHeight;

        CComPtr< CapturingReality::OrthoProjection::IOrthographicProjection > spOrthoProjection;
        CComPtr< CapturingReality::OrthoProjection::IOrthoProjectionDataProvider > spOrthoProjectionDataProvider;

        if ( SUCCEEDED( hr ) )
        {
            printf( "Calculating an orthographic projection... \n" );

            hr = CalculateAxisAlignedOrthoProjection(
                &centre3016, projectionWidth, projectionHeight,
                projectionDepth, pixelSizeInMetersX, pixelSizeInMetersY,
                g_epsg_3016_name, g_epsg_3016_definition,
                spCache, spSfmReconstruction, spMvsModelSimplified, &spOrthoProjection
            );
        }
///////////////////////////////////////////////////////////////////////////////
// ndef
#ifndef _ENABLE_CR_BASE_CODE_
		if (!SUCCEEDED(hr))
		{
			printf( "CalculateAxisAlignedOrthoProjection error HRESULT=%d \n", hr );
			OutputDebugString(L"CalculateAxisAlignedOrthoProjection error.\n");
			HrToStr(hr);
			exit(-551);
		}
#endif	// _ENABLE_CR_BASE_CODE_
///////////////////////////////////////////////////////////////////////////////

        // Export as geotiff.

        if ( SUCCEEDED( hr ) )
        {
            hr = CapturingReality::ModelTools::CreateOrthoPhotoDataProvider( spOrthoProjection, NULL, &spOrthoProjectionDataProvider );
        }
///////////////////////////////////////////////////////////////////////////////
// ndef
#ifndef _ENABLE_CR_BASE_CODE_
		if (!SUCCEEDED(hr))
		{
			printf( "CreateOrthoPhotoDataProvider error HRESULT=%d \n", hr );
			OutputDebugString(L"CreateOrthoPhotoDataProvider error.\n");
			HrToStr(hr);
			exit(-552);
		}
#endif	// _ENABLE_CR_BASE_CODE_
///////////////////////////////////////////////////////////////////////////////

        if ( SUCCEEDED( hr ) )
        {
///////////////////////////////////////////////////////////////////////////////
// ndef
#ifndef _ENABLE_CR_BASE_CODE_
			wstring full_path(p_output_dir);
			full_path.append(L"orthoPhoto.tiff");
#endif	// _ENABLE_CR_BASE_CODE_
///////////////////////////////////////////////////////////////////////////////
			bool bGeoTaggsWritten;
            hr = CapturingReality::ModelTools::ExportOrthoProjectionToGeoTiff(
                CapturingReality::OrthoProjection::WorldCoordSystemCentreType::WCSCT_GLOBAL,
                CapturingReality::OrthoProjection::WorldCoordSystemAxesType::WCSAT_GLOBAL_COORDINATE_SYSTEM,
                CapturingReality::OrthoProjection::OrthoPixelType::OPT_COLOR,
                CapturingReality::OrthoProjection::CompressionType::CT_NONE,
                spOrthoProjectionDataProvider,
///////////////////////////////////////////////////////////////////////////////
#ifdef _ENABLE_CR_BASE_CODE_
                L"orthoPhoto.tiff",
///////////////////////////////////////////////////////////////////////////////
// ndef
#else
				full_path.c_str(),
#endif	// _ENABLE_CR_BASE_CODE_
///////////////////////////////////////////////////////////////////////////////
                false,
                NULL,
                &bGeoTaggsWritten
            );
        }
///////////////////////////////////////////////////////////////////////////////
// ndef
#ifndef _ENABLE_CR_BASE_CODE_
		if (!SUCCEEDED(hr))
		{
			printf( "ExportOrthoProjectionToGeoTiff error HRESULT=%d \n", hr );
			OutputDebugString(L"ExportOrthoProjectionToGeoTiff error.\n");
			HrToStr(hr);
			exit(-553);
		}
#endif	// _ENABLE_CR_BASE_CODE_
///////////////////////////////////////////////////////////////////////////////

        if ( SUCCEEDED( hr ) )
        {
///////////////////////////////////////////////////////////////////////////////
#ifdef _ENABLE_CR_BASE_CODE_
            hr = ExportWorldFile( L"orthoPhoto.tfw", spOrthoProjection );
///////////////////////////////////////////////////////////////////////////////
// ndef
#else
			wstring full_path(p_output_dir);
			full_path.append(L"orthoPhoto.tfw");
            hr = 
				ExportWorldFile(
					full_path.c_str(),
					spOrthoProjection );
#endif	// _ENABLE_CR_BASE_CODE_
///////////////////////////////////////////////////////////////////////////////
        }
///////////////////////////////////////////////////////////////////////////////
// ndef
#ifndef _ENABLE_CR_BASE_CODE_
		if (!SUCCEEDED(hr))
		{
			printf( "tfw export error HRESULT=%d \n", hr );
			OutputDebugString(L"tfw export error.\n");
			HrToStr(hr);
			exit(-554);
		}
#endif	// _ENABLE_CR_BASE_CODE_
///////////////////////////////////////////////////////////////////////////////

	}
///////////////////////////////////////////////////////////////////////////////
// ndef
#ifndef _ENABLE_CR_BASE_CODE_
	if (!SUCCEEDED(hr))
	{
        printf( "Xyz export error HRESULT=%d \n", hr );
		OutputDebugString(L"Xyz export error.\n");
		HrToStr(hr);
		exit(-550);
	}
#endif	// _ENABLE_CR_BASE_CODE_
///////////////////////////////////////////////////////////////////////////////

	//
    // __ create and export an ortho projection (gps) __
    //

    if ( SUCCEEDED( hr ) )
    {
        CoordinateSystemPoint centreGps; // degrees in radians

///////////////////////////////////////////////////////////////////////////////
#ifdef _ENABLE_CR_BASE_CODE_
        // Region centre on Earth.
        centreGps.x = 20.0581695625 * DEG_TO_RAD; // 0.350081100788936
        centreGps.y = 63.7211447956 * DEG_TO_RAD; // 1.11214377982249
        centreGps.z = 97.4389834320173;

        // Earth's region to cover.
        const double north = 63.7220418742507 * DEG_TO_RAD;
        const double south = 63.7204966958123 * DEG_TO_RAD;
        const double east  = 20.0596314452294 * DEG_TO_RAD;
        const double west  = 20.0561460717712 * DEG_TO_RAD;

///////////////////////////////////////////////////////////////////////////////
// ndef
#else
		// get Ortho Data from file.
		if (!loadOrthoGps(
				p_param_dir,
				L"OrthoGps.txt",
				centreGps,
				north,
				south,
				east,
				west))
		{
			printf( "loadOrthoGps error HRESULT=%d \n", hr );
			OutputDebugString(L"loadOrthoGps error.\n");
			HrToStr(hr);
			exit(-560);
		}

		// calc ortho gps
		centreGps.x *= DEG_TO_RAD;
		centreGps.y *= DEG_TO_RAD;
//		centreGps.z *= DEG_TO_RAD;		no touch z.

        north *= DEG_TO_RAD;
        south *= DEG_TO_RAD;
        east  *= DEG_TO_RAD;
        west  *= DEG_TO_RAD;
#endif	// _ENABLE_CR_BASE_CODE_
///////////////////////////////////////////////////////////////////////////////

        const double pixelSizeInRadiansX = ( east  -  west ) / 1024.0;
        const double pixelSizeInRadiansY = ( north - south ) / 1024.0;

        CComPtr< CapturingReality::OrthoProjection::IOrthographicProjection > spOrthoProjection;
        CComPtr< CapturingReality::OrthoProjection::IOrthoProjectionDataProvider > spOrthoProjectionDataProvider;

        if ( SUCCEEDED( hr ) )
        {
            printf( "Calculating orthographic projection... \n" );

            hr = CalculateAxisAlignedOrthoProjection(
                &centreGps, projectionWidth, projectionHeight,
                projectionDepth, pixelSizeInRadiansX, pixelSizeInRadiansY,
                g_gps_name, g_gps_definition,
                spCache, spSfmReconstruction, spMvsModelSimplified, &spOrthoProjection
            );
        }
///////////////////////////////////////////////////////////////////////////////
// ndef
#ifndef _ENABLE_CR_BASE_CODE_
		if (!SUCCEEDED(hr))
		{
			printf( "CalculateAxisAlignedOrthoProjection error HRESULT=%d \n", hr );
			OutputDebugString(L"CalculateAxisAlignedOrthoProjection error.\n");
			HrToStr(hr);
			exit(-561);
		}
#endif	// _ENABLE_CR_BASE_CODE_
///////////////////////////////////////////////////////////////////////////////

        // Export into kzm.

        if ( SUCCEEDED( hr ) )
        {
            hr = CapturingReality::ModelTools::CreateOrthoPhotoDataProvider( spOrthoProjection, NULL, &spOrthoProjectionDataProvider );
        }
///////////////////////////////////////////////////////////////////////////////
// ndef
#ifndef _ENABLE_CR_BASE_CODE_
		if (!SUCCEEDED(hr))
		{
			printf( "CreateOrthoPhotoDataProvider error HRESULT=%d \n", hr );
			OutputDebugString(L"CreateOrthoPhotoDataProvider error.\n");
			HrToStr(hr);
			exit(-562);
		}
#endif	// _ENABLE_CR_BASE_CODE_
///////////////////////////////////////////////////////////////////////////////

        if ( SUCCEEDED( hr ) )
        {
			CapturingReality::OrthoProjection::OrthoImageExportParameters parameters;
///////////////////////////////////////////////////////////////////////////////
#ifdef _ENABLE_CR_BASE_CODE_
            parameters.filename = L"orthoPhoto.kmz";
///////////////////////////////////////////////////////////////////////////////
// ndef
#else
			wstring full_path(p_output_dir);
			full_path.append(L"orthoPhoto.kmz");
			parameters.filename = full_path.c_str();
#endif	// _ENABLE_CR_BASE_CODE_
///////////////////////////////////////////////////////////////////////////////
            parameters.gcpsCount = 0;
            parameters.pGcps = NULL;
            parameters.imageQuality = 90;
            parameters.kmlExportCompressed = true;
            parameters.kmlTileExtension = L"png";
            parameters.kmlTileSide = 1024;
            parameters.kmlTileWicCodecGUID = GUID_ContainerFormatPng;

            hr = CapturingReality::ModelTools::ExportOrthoPhotoToKml( &parameters, spOrthoProjectionDataProvider, NULL, NULL );
        }
///////////////////////////////////////////////////////////////////////////////
// ndef
#ifndef _ENABLE_CR_BASE_CODE_
		if (!SUCCEEDED(hr))
		{
			printf( "kml export error HRESULT=%d \n", hr );
			OutputDebugString(L"kml export error.\n");
			HrToStr(hr);
			exit(-563);
		}

#endif	// _ENABLE_CR_BASE_CODE_
///////////////////////////////////////////////////////////////////////////////
    }

    //
    //
#ifdef _ENABLE_CR_BASE_CODE_

    // View results.

    if ( SUCCEEDED( hr ) )
    {
        hr = OpenFile( L"model.ply" );
    }

    if ( SUCCEEDED( hr ) )
    {
        hr = OpenFile( L"modelSimpl.obj" );
    }

    if ( SUCCEEDED( hr ) )
    {
        hr = OpenFile( L"orthoPhoto.tiff" );
    }

    if ( SUCCEEDED( hr ) )
    {
        hr = OpenFile( L"orthoPhoto.kmz" );
    }

    printf( "Completed with code %#X\n", hr );
#endif _ENABLE_CR_BASE_CODE_

    return 0;
}
