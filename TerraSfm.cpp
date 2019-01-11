//
// Capturing Reality s.r.o.
//
// This file 'OrthoExample.cpp' is a part of the RC Engine Samples. You may use it freely.
//

#include "stdafx.h"
#include <vector>
#include <cstdio>
#include <iostream>
#include <locale>
#include <vector>
#include <string>
#include <fstream>
#include <direct.h>
#include <string.h>  
#include <stdlib.h>  
#include <stdio.h>  
#include <errno.h> 
#include <Windows.h>

#include "define.h"
#include "SfmUtils.h"
#include "ControlPoints.h"
#include "..\SamplesCommon\TransformationHelpers.h"

#include "w84gps.h"
#include "CubeDistributionAnalyzer.h"
#include "CameraAngle.h"
#include "exportUtil.h"
#include "param.h"
#include "CrsUtils.h"

using namespace std;

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
//          * filter and simply the model
//          * colorize and calculate the model texture
//          * create and export an ortho projection (epsg:2451)
//          * create and export the ortho projection (gps)
//
//
// Inline definitions of coordinate systems used in this example.
//
///////////////////////////////////////////////////////////////////////////////

//Define Macro
#define BUFFER_SIZE 2048

/* Define folder */
 const wchar_t* densification = L"2_densification";
 const wchar_t* point_cloud = L"\\point_cloud\\";
 const wchar_t* quarry_group1_densified_point_cloud = L"quarry_group1_densified_point_cloud";
 const wchar_t* dsm_ortho = L"3_dsm_ortho";
 const wchar_t* mosaic = L"\\2_mosaic\\";
 const wchar_t* dsm = L"\\1_dsm\\";
 const wchar_t* quarry_mosaic_group1 = L"quarry_mosaic_group1";


///////////////////////////////////////////////////////////////////////////////
// global
FILE* fp;

// Global import val
const wchar_t* g_epsg_2451_name;
const wchar_t* g_epsg_2451_definition;

const wchar_t* g_gps_name = L"GPS (WGS 84)";
const wchar_t* g_gps_definition = L"+proj=longlat +datum=WGS84 +no_defs";


// Global import val
double north;
double south;
double east;
double west;

// Global export Image detail
UINT projectionWidth = 4000;
UINT projectionHeight = 4000;
double projectionDepth = 100.0;

const int EPSG_LIST_LINE_COUNT = 10;

// computed Bounding Box in SDK
double bbox_min[3];
double bbox_max[3];

// Mode GCP or not GCP 
 bool f_mode_GCP = FALSE;

///////////////////////////////////////////////////////////////////////////////
//
//

 int _tmain(int argc, _TCHAR* argv[])
 {
	 int wgs_number = 0;

	 HRESULT hr = ActivateSdk();
	 if (!SUCCEEDED(hr))
	 {
		return 0;
	 }
	 // 引数チェック
	 if ((argc < 4) && (argc > 6))
	 {
		 wprintf(L"\nusage:  >TerraSfm.exe c:\\input\\ c:\\output\\ c:\\param\\ -nocache [-GCP]\n");
		 exit(-1);
	 }
	 if (!PathFileExists(argv[1]))
	 {
		 wprintf(L"\nInput Image dir %ls is not found.\n", argv[1]);
		 wprintf(L"\nusage:  >TerraSfm.exe c:\\input\\ c:\\output\\ c:\\param\\ -nocache [-GCP]\n");
		 exit(-2);
	 }
	 if (!PathFileExists(argv[2]))
	 {
		 wprintf(L"\nOutput Sfm dir %ls is not found.\n", argv[2]);
		 wprintf(L"\nusage:  >TerraSfm.exe c:\\input\\ c:\\output\\ c:\\param\\ -nocache [-GCP]\n");
		 exit(-3);
	 }
	 if (!PathFileExists(argv[3]))
	 {
		 wprintf(L"\nParam dir %ls is not found.\n", argv[3]);
		 wprintf(L"\nusage:  >TerraSfm.exe c:\\input\\ c:\\output\\ c:\\param\\ -nocache [-GCP]\n");
		 //exit(-4);
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
			 wprintf(L"\nusage:  >TerraSfm.exe c:\\input\\ c:\\output\\ c:\\param\\ -nocache [-GCP]\n");
			 wprintf(L"is image width and height.");
			 wprintf(L"-nocache is option.");
			 exit(-5);
		 }
	 }
	 else if (argc == 6)
	 {
		 wstring gcp_str(argv[5]);
		 if ((gcp_str.compare(L"-GCP") == 0) || (gcp_str.compare(L"-gcp") == 0))
		 {
			 f_mode_GCP = TRUE;
		 }
		 else
		 {
			 f_mode_GCP = FALSE;
		 }
	 }

	 // Get CRSDK Engine Version
	 ServiceConnectionContext connectionContext;
	 strcpy_s(connectionContext.appToken, SDKSAMPLE_APP_TOKEN);
	 CComPtr< IEngineOnlineServices > spOnlineServices;
	 hr = CreateEngineOnlineServices(&connectionContext, &spOnlineServices);
	 EngineVersionInfo newVersionInfo = { 0 };
	 hr = spOnlineServices->GetEngineUpdateInfo(&newVersionInfo);
	 if (SUCCEEDED(hr))
	 {
		 UINT major, minor, sub, build;
		 CapturingReality::RealityCaptureEngine::GetEngineVersion(&major, &minor, &sub, &build);
		 printf("[Info] CRSDK Version %d.%d.%d build=%d\n", major, minor, sub, build);

		 bool bApiChanged = (newVersionInfo.major > major) || (newVersionInfo.minor > minor) || (newVersionInfo.sub > sub);
		 if (bApiChanged || (newVersionInfo.build > build))
		 {
			 printf("A new version of RealityCapture Engine is available.\n\n");
		 }
		 else
		 {
			 printf("Your RealityCapture Engine libraries are up to date.\n\n");
		 }
		 if (bApiChanged)
		 {
			 printf("[ You will need to re-compile your application since there are changes in API which might made DLL files incompatible with this version. ]\n\n");
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
	 errno = fopen_s(&fp, "SfmExample_log.txt", "w");

	 //
	 // __ create folder ouput __
	 //
	 wchar_t p_output_dir1[MAX_PATH];
	 char* output = (char*)malloc(MAX_PATH);

	 wcscpy_s(p_output_dir1, param_strlen, p_output_dir);
	 wcscat_s(p_output_dir1, densification);
	 if (!PathFileExists(p_output_dir1))
	 {
		 wcstombs(output, p_output_dir1, MAX_PATH);
		 _mkdir(output);
	 }

	 wcscat_s(p_output_dir1, point_cloud);
	 if (!PathFileExists(p_output_dir1))
	 {
		 wcstombs(output, p_output_dir1, MAX_PATH);
		 _mkdir(output);
	 }

	 wchar_t p_output_dir2[MAX_PATH];
	 char* output1 = (char*)malloc(MAX_PATH);

	 wcscpy_s(p_output_dir2, param_strlen, p_output_dir);
	 wcscat_s(p_output_dir2, dsm_ortho);
	 if (!PathFileExists(p_output_dir2))
	 {
		 wcstombs(output1, p_output_dir2, MAX_PATH);
		 _mkdir(output1);
	 }

	 wcscat_s(p_output_dir2, mosaic);
	 if (!PathFileExists(p_output_dir2))
	 {
		 wcstombs(output1, p_output_dir2, MAX_PATH);
		 _mkdir(output1);
	 }

	 wchar_t p_output_dir3[MAX_PATH];
	 wcscpy_s(p_output_dir3, param_strlen, p_output_dir);
	 wcscat_s(p_output_dir3, dsm_ortho);
	 wcscat_s(p_output_dir3, dsm);
	 if (!PathFileExists(p_output_dir3))
	 {
		 wcstombs(output1, p_output_dir3, MAX_PATH);
		 _mkdir(output1);
	 }

	 //
	 // __ Get EPSG infor frome EpsgList.txt __
	 //
	 wchar_t* epsg_list[EPSG_LIST_LINE_COUNT];
	 extern double whd[3];
	 int epsg_count = 0;
	 epsg_count =
		loadEpsgList(
			p_param_dir,
			L"EpsgList.txt",
			epsg_list,
			EPSG_LIST_LINE_COUNT);

	 if (epsg_count == EPSG_LIST_LINE_COUNT)
	 {			 
		wgs_number = _wtoi(epsg_list[1]);
		const wchar_t* pp_w84 = nullptr;
		if (!toWgs84(wgs_number, &pp_w84))
		{
			wprintf(L"WGS number is invalid val=%d", wgs_number);
			exit(-11);
		}

		g_epsg_2451_name = epsg_list[0];
		g_epsg_2451_definition = pp_w84;

		projectionWidth = _wtoi(epsg_list[7]);
		projectionHeight = _wtoi(epsg_list[8]);
		projectionDepth = _wtof(epsg_list[9]);

		whd[0] = _wtof(epsg_list[4]);
		whd[1] = _wtof(epsg_list[5]);
		whd[2] = _wtof(epsg_list[6]);
	 }
	 else
	 {
		 wgs_number = 4326;
		 g_epsg_2451_name = L"GPS (WGS 84)";
		 g_epsg_2451_definition = L"+proj=longlat +datum=WGS84 +no_defs";		 
	 }
	 
	 //
	 // __ create cache __
	 //
	 // Creating cache is optional for Sfm creation process but required for Mvs / Multipart meshing.
	 //
	 CComPtr< CapturingReality::IConfig > spConfig;
	 CComPtr< CapturingReality::IResourceCache > spCache;

	 if (SUCCEEDED(hr))
	 {
		 hr = CapturingReality::Utilities::CreateResourceCache(CapturingReality::RCL_CUSTOM, L"_crTmp", &spCache);
	 }

	 if (SUCCEEDED(hr))
	 {
		 hr = CapturingReality::Utilities::CreateConfig(&spConfig);
	 }

	 //
	 // __ obtain a component __
	 //
	 // The component has to be geo-referenced.
	 //
	 // Create epsg:2451 coordinate system. The definition can be found in ..\Resources\metadata\epsg.xml.
	 //
	 CComPtr< CapturingReality::CoordinateSystems::ICoordinateSystem > spCoordinateSystem2451;

	 if (wgs_number != 4326)
	 {
		 if (SUCCEEDED(hr))
		 {
			 hr = CapturingReality::CoordinateSystems::CreateCoordinateSystem(g_epsg_2451_name, g_epsg_2451_definition, &spCoordinateSystem2451);
		 }
	 }

	 // Import the one from previous run if present or create a new one.
	 CComPtr< CapturingReality::Sfm::IStructureFromMotion > spSfm;
	 CComPtr< CapturingReality::Sfm::ISfmReconstruction > spSfmReconstruction;

	 if (SUCCEEDED(hr))
	 {
		 if (f_mode_GCP) {
			 printf("Create SFM GCP... \n");
			 hr =
				 CreateOrthoExampleSfm(
					 p_input_dir,
					 p_param_dir,
					 spCache,
					 spConfig,
					 spCoordinateSystem2451,
					 &spSfm);
		 }
		 else
		 {
			 printf("Create SFM none GCP... \n");
			 hr = CreateSfmFromDirectory(p_input_dir, spCache, spConfig, &spSfm);
		 }
	 }
	 if (!SUCCEEDED(hr))
	 {
		 printf("CreateOrthoExampleSfm error HRESULT=%d \n", hr);
		 OutputDebugString(L"CreateOrthoExampleSfm error.\n");
		 HrToStr(hr);
		 exit(-12);
	 }

	 //
	 // __ get value coordinate center GPS, North, South, East, West __
	 //
	 CoordinateSystemPoint centerGps;
	 hr = GetInforCoordGPS(spSfm, &centerGps, &west, &east, &south, &north);
	 if (SUCCEEDED(hr))
	 {
		 if (wgs_number == 4326)
		 {
			 const wchar_t* pp_w84 = nullptr;

			 if (!toSearchCode(centerGps.x, centerGps.y, &wgs_number))
			 {
				 wprintf(L"Code number is invalid val=%d", wgs_number);
				 exit(-1000);
			 }

			 if (!toWgs84(wgs_number, &pp_w84))
			 {
				 wprintf(L"WGS number is invalid val=%d", wgs_number);
				 exit(-1001);
			 }
			 g_epsg_2451_definition = pp_w84;

			 pp_w84 = nullptr;
			 if (!toWgs84Name(wgs_number, &pp_w84))
			 {
				 wprintf(L"WGS name is invalid val=%d", wgs_number);
				 exit(-1002);
			 }
			 g_epsg_2451_name = pp_w84;

			 printf("CoordinateSystems =%d \n", wgs_number);
			 if (SUCCEEDED(hr) && ( !spCoordinateSystem2451 ))
			 {
				 hr = CapturingReality::CoordinateSystems::CreateCoordinateSystem(g_epsg_2451_name, g_epsg_2451_definition, &spCoordinateSystem2451);
			 }
			
			 //write info coordinates to file epsg.txt			 
			wchar_t s_wgs_number[256];
			swprintf_s(s_wgs_number, L"%d", wgs_number);

			bool err = writeFileTxt(p_param_dir, L"EpsgList.txt", g_epsg_2451_name, true);
			err = writeFileTxt(p_param_dir, L"EpsgList.txt", s_wgs_number, false);
			err = writeFileTxt(p_param_dir, L"EpsgList.txt", g_gps_name, false);
			err = writeFileTxt(p_param_dir, L"EpsgList.txt", g_gps_definition, false);

			if (!err)
			{
				printf("writeFileTxt =%d \n", err);
				OutputDebugString(L"writeFileTxt error.\n");
				HrToStr(err);
				exit(-1004);
			}
			 
		 }
	 }

	 if (!SUCCEEDED(hr))
	 {
		 printf("Get Infor Coordinates GPS HRESULT=%d \n", hr);
		 OutputDebugString(L"Get Infor Coordinates GPS error.\n");
		 HrToStr(hr);
		 exit(-13);
	 }

	 //
	 // __ Create Reconstruction __
	 //
	 if (SUCCEEDED(hr))
	 {
		 const wchar_t *pFileName = L"component0.rcalign";
		 if (is_use_cache)
		 {
			 if (PathFileExists(pFileName) == TRUE)
			 {
				 printf("Importing component ... \n");
				 ImportRCComponent(pFileName, spSfm, &spSfmReconstruction);
			 }
			 else
			 {
				 printf("Aligning ... \n");
				 hr = CreateReconstruction(spSfm, &spSfmReconstruction);
				 if (SUCCEEDED(hr))
				 {
					 printf("Exporting component ... \n");
					 hr = ExportRCComponent(pFileName, spSfm, spSfmReconstruction);
				 }
			 }
		 }
		 else
		 {			 
			 if (f_mode_GCP)
			 {
				 printf("Aligning GCP... \n");
				 hr = CreateReconstruction(spSfm, &spSfmReconstruction);
			 }
			 else
			 {
				 printf("Aligning none GCP... \n");
				 hr = Align(spSfm, &spSfmReconstruction);
			 }

			 if (SUCCEEDED(hr))
			 {
				 printf("Exporting component ... \n");
				 hr = ExportRCComponent(pFileName, spSfm, spSfmReconstruction);
			 }
		 }
	 }
	 if (!SUCCEEDED(hr))
	 {
		 printf("Create Reconstruction HRESULT=%d \n", hr);
		 OutputDebugString(L"Create Reconstruction error.\n");
		 HrToStr(hr);
		 exit(-400);
	 }

	 //
	 // __ create a model __
	 //
	 //
	 // Create a model within a reconstruction region. The reconstruction region is in the epsg:2451 coordinate system.
	 //
	 // Create a reconstruction region.
	 //
	 GlobalReconstructionVolume region;
	 extern CoordinateSystemPoint centre2451;

	 if (SUCCEEDED(hr))
	 {
		 // convert from global coordinate system to sfm coordinate system. The Sfm must be geo-referenced.
		 if (f_mode_GCP) 
		 {
			 printf("Create region GCP... \n");
			 hr =
				 ConvertGcsBoxToSfmBox(
					 centre2451,
					 whd,
					 spCoordinateSystem2451,
					 spSfmReconstruction,
					 &region);
		 }
		 else
		 {
			 printf("Create region none GCP... \n");
			 hr = ApproximateReconstructionBox(&region, spSfm, spSfmReconstruction, false);
		 }
	 }
	 if (!SUCCEEDED(hr))
	 {
		 printf("Create region error HRESULT=%d \n", hr);
		 OutputDebugString(L"Create region error.\n");
		 HrToStr(hr);
		 exit(-500);
	 }

	 //
	 // __ create a model.
	 //
	 CComPtr< CapturingReality::Mvs::IMvsModel > spMvsModel;

	 if (SUCCEEDED(hr))
	 {
		 printf("Computing a model ... \n");
		 hr = CreateModel(2, &region, spCache, spConfig, spSfm, spSfmReconstruction, &spMvsModel);
	 }

	 if (!SUCCEEDED(hr))
	 {
		 printf("Create Model error HRESULT=%d \n", hr);
		 OutputDebugString(L"Create Model error.\n");
		 HrToStr(hr);
		 exit(-510);
	 }

	 if (SUCCEEDED(hr))
	 {
		 printf("Unlocking the model ... \n");
		 hr = UnlockModelIfNeeded(spSfm, spSfmReconstruction, spMvsModel);
	 }

	 //
	 // __ colorize and calculate model the texture __
	 //
	 if (SUCCEEDED(hr))
	 {
		 printf("Calculating the model vertex colors ... \n");
		 hr = ColorizeModel(1, spCache, spSfm, spSfmReconstruction, spMvsModel);
	 } 

	 //
	 // __ texture tool __
	 //	
	 float cmPerPixel = 100.0f;

	 if (SUCCEEDED(hr))
	 {
		 if (SUCCEEDED(hr) && (spSfmReconstruction->GetReconstructionFlags() & (SCCF_GROUND_CONTROL | SCCF_METRIC)))
		 {
			 //convert from centimeter per pixel to meter per texel ( ~pixel )
			 float texelSize = cmPerPixel / 100.0f;

			 CapturingReality::ModelTools::UnwrapParameters unwrapParams;
			 hr = ComputeUnwrapParamsByFixedTexelSize(texelSize, spMvsModel, &unwrapParams);
			 if (SUCCEEDED(hr))
			 {
				 printf("Texturing model unwrapParams... \n");
				 hr = TextureModel(&unwrapParams, spCache, spSfm, spSfmReconstruction, spMvsModel);
			 }
		 }
		 else
		 {
			 printf("Texturing model ... \n");
			 hr = TextureModel(1, spCache, spSfm, spSfmReconstruction, spMvsModel);
		 }
	 }

	 //
	 // __ filter model if requested __
	 //
	 if (SUCCEEDED(hr))
	 {
		 printf("Filtering model ... \n");
		 CComPtr< CapturingReality::Mvs::IMvsModel > spMvsModelFiltered;
		 hr = FilterSample01(8.0f, spMvsModel, &spMvsModelFiltered);
		 if (SUCCEEDED(hr))
		 {
			 spMvsModel.Release();
			 spMvsModel = NULL;
			 spMvsModel = spMvsModelFiltered;
		 }
	 }	 

	 if (SUCCEEDED(hr))
	 {
		 printf("Exporting model to obj ... \n");
		 hr =
			 ExportModelObj(
				 p_output_dir1,
				 L"quarry_group1_densified_point_cloud",
				 spCache,
				 spSfm,
				 spSfmReconstruction,
				 spMvsModel);
	 }
	 if (!SUCCEEDED(hr))
	 {
		 printf("Obj export error HRESULT=%d \n", hr);
		 OutputDebugString(L"Obj export error.\n");
		 HrToStr(hr);
		 exit(-530);
	 }

	 //
	 // __ simplify relative tool __
	 //		 
	 if (SUCCEEDED(hr))
	 {
		 printf("Simplifing relative the model ... \n");
		 CComPtr< CapturingReality::Mvs::IMvsModel > spMvsModelSimplifiedRelative;
		 hr = SimplifyRelative(0.1, spMvsModel, &spMvsModelSimplifiedRelative);
		 if (SUCCEEDED(hr))
		 {
			 spMvsModel.Release();
			 spMvsModel = NULL;
			 spMvsModel = spMvsModelSimplifiedRelative;
		 }
	 }

	 //
	 // __ smooth tool __
	 //		 
	 if (SUCCEEDED(hr))
	 {
		 printf("Smooth the model ... \n");
		 CComPtr< CapturingReality::Mvs::IMvsModel > spMvsModelSmooth;
		 hr = Smooth(0.500000, spMvsModel, &spMvsModelSmooth);
		 if (SUCCEEDED(hr))
		 {
		 spMvsModel.Release();
		 spMvsModel = NULL;
		 spMvsModel = spMvsModelSmooth;
		 }
	 }

	 //
	 // __ export model XYZ__
	 //
	 if (SUCCEEDED(hr))
	 {
		 printf("Exporting model to xyz ... \n");
		 hr = ExportModelXYZ(p_output_dir1,
			 L"quarry_group1_densified_point_cloud",
			 spCache,
			 spSfm,
			 spSfmReconstruction,
			 spMvsModel);
	 }

	 if (!SUCCEEDED(hr))
	 {
		 printf("XYZ Model export error HRESULT=%d \n", hr);
		 OutputDebugString(L"XYZ Model error.\n");
		 HrToStr(hr);
		 exit(-526);
	 }

	 //
	 // __ simplify tool __
	 //	
	 CComPtr< CapturingReality::Mvs::IMvsModel > spMvsModelSimplified;
	 if (SUCCEEDED(hr))
	 {
		 
		printf("Simplifing model... \n");
		hr = Simplify(3000000, spMvsModel, &spMvsModelSimplified);
		 
	 }	 
	 //
	 // __ export model PLY__
	 //
	 if (SUCCEEDED(hr))
	 {
		 printf("Exporting model to ply ... \n");
		 hr =
			 ExportModelPly(
				 p_output_dir1,
				 L"quarry_group1_densified_point_cloud",
				 spCache,
				 spSfm,
				 spSfmReconstruction,
				 spMvsModelSimplified);
	 }

	 if (!SUCCEEDED(hr))
	 {
		 printf("Ply export error HRESULT=%d \n", hr);
		 OutputDebugString(L"Ply export error.\n");
		 HrToStr(hr);
		 exit(-535);
	 }	
	 //
	 // __ create and export an ortho projection __
	 //	
	 if (SUCCEEDED(hr))
	 {	
		 printf("\nExport an orthographic projection... \n");
		 if (!f_mode_GCP)
		 {
			 //export GTiff GPS			 			 
			 hr = CalculateOrthoProjectionAndExportToTiffGPS(p_output_dir2, spCache, spSfm, spSfmReconstruction, spMvsModel, &region, cmPerPixel);
			 
		 }
		 else
		 {
			 //export GTiff EPSG:2451
			 CapturingReality::CoordinateSystemPoint pCentre;
			 pCentre = centre2451;		 

			 //pixel size in meters.
			 const double pixelSizeInMetersX = 2 * whd[0] / projectionWidth;
			 const double pixelSizeInMetersY = 2 * whd[1] / projectionHeight;

			 CComPtr< CapturingReality::OrthoProjection::IOrthographicProjection > spOrthoProjection;
			 CComPtr< CapturingReality::OrthoProjection::IOrthoProjectionDataProvider > spOrthoProjectionDataProvider;

			 if (SUCCEEDED(hr))
			 {
				 printf("\nCalculating an orthographic projection normal... \n");

				 hr = CalculateAxisAlignedOrthoProjection(
					 &pCentre, projectionWidth, projectionHeight,
					 projectionDepth, pixelSizeInMetersX, pixelSizeInMetersY,
					 g_epsg_2451_name, g_epsg_2451_definition,
					 spCache, spSfmReconstruction, spMvsModel, &spOrthoProjection
				 );
			 }
			 if (!SUCCEEDED(hr))
			 {
				 printf("CalculateAxisAlignedOrthoProjection error HRESULT=%d \n", hr);
				 OutputDebugString(L"CalculateAxisAlignedOrthoProjection error.\n");
				 exit(-650);
			 }

			 if (SUCCEEDED(hr))
			 {
				 hr = CapturingReality::ModelTools::CreateOrthoPhotoDataProvider(spOrthoProjection, NULL, &spOrthoProjectionDataProvider);
			 }
			 if (!SUCCEEDED(hr))
			 {
				 printf("CreateOrthoPhotoDataProvider error HRESULT=%d \n", hr);
				 OutputDebugString(L"CreateOrthoPhotoDataProvider error.\n");
				 exit(-660);
			 }

			 if (SUCCEEDED(hr))
			 {
				 bool bGeoTaggsWritten;

				 wstring full_path(p_output_dir2);
				 full_path.append(L"quarry_mosaic_group1.tif");

				 hr = CapturingReality::ModelTools::ExportOrthoProjectionToGeoTiff(
					 CapturingReality::OrthoProjection::WorldCoordSystemCentreType::WCSCT_GLOBAL,
					 CapturingReality::OrthoProjection::WorldCoordSystemAxesType::WCSAT_GLOBAL_COORDINATE_SYSTEM,
					 CapturingReality::OrthoProjection::OrthoPixelType::OPT_COLOR,
					 CapturingReality::OrthoProjection::CompressionType::CT_NONE,
					 spOrthoProjectionDataProvider,
					 full_path.c_str(),
					 false,
					 NULL,
					 &bGeoTaggsWritten
				 );

				 if (SUCCEEDED(hr))
				 {
					 wstring full_path(p_output_dir2);
					 full_path.append(L"quarry_mosaic_group1.tfw");

					 hr = ExportWorldFile(full_path.c_str(), spOrthoProjection);
				 }
			 }
			 if (!SUCCEEDED(hr))
			 {
				 printf("ExportOrthoProjectionToGeoTiff error HRESULT=%d \n", hr);
				 OutputDebugString(L"ExportOrthoProjectionToGeoTiff error.\n");
				 exit(-670);
			 }
		 }
	}


	printf("Project execute to finish!!!\n", hr);
    return 0;
}
