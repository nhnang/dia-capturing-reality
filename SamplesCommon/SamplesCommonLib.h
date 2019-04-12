//
// Capturing Reality s.r.o.
//
// This file 'SamplesCommonLib.h' is a part of the RC Engine Samples. You may use it freely.
//

#pragma once

#define SDKSAMPLE_APP_TOKEN "{9A683749-4AAE-4B91-A3C1-32E16794EDA1}"

using namespace CapturingReality::RealityCaptureServices;

// Call to shell-execute the default web browser to open a given URL
HRESULT OpenWebBrowser( __in_z const WCHAR *pUrl );

// Call to shell-execute the default viewer to open a given file
HRESULT OpenFile( __in_z const WCHAR *pFileName );

HRESULT ReadText( __in_z const WCHAR *pFileName, __out_ecount_z(maxTextSize) char *pText, __in size_t maxTextSize );

HRESULT WriteText( __in_z const WCHAR *pFileName, __out_ecount_z(maxKeySize) const char *pText );

// Acquire a license. The default web browser is invoked for any user interaction and the function returns
// either if the user finishes the process successfully or skip/close the browser
HRESULT GetLicense( __out_ecount_z(maxKeySize) char *pKey, __in UINT maxKeySize, __inout_ecount_z_opt(128) char *pRenewToken = NULL );

// Acquire a license using a renew token.
HRESULT RenewLicense( __out_ecount_z(maxKeySize) char *pKey, __in UINT maxKeySize, __inout_ecount_z(128) char *pRenewToken );

HRESULT GetLicenseExpirationDaysCount( __out UINT *pDaysToExpire );

// Activate the sample using the above-listed functions. Cache the license to a local file.
HRESULT ActivateSdk();

// Renew the license and activateSDK
HRESULT ReactivateSdk( __inout_ecount_z_opt(128) char *pRenewToken = NULL );

// Acquire a license for the given dataset for a PPI license
HRESULT AcquireDatasetLicense( __in IInputsLicensingManager *pLicenseManager, __in_z const UINT *pRequestEKU, __in ISfmReconstruction *pReconstruction );

// align a list of images
HRESULT AlignImages( 
    __in_z const wchar_t** ppImageList, __in UINT imagesCount, 
    __in CapturingReality::IResourceCache *pCache, __in CapturingReality::IConfig *pConfig,
    __deref_out CapturingReality::Sfm::IStructureFromMotion **ppSfm );

HRESULT Align(
	__in        CapturingReality::Sfm::IStructureFromMotion*    pSfm,
	__deref_out CapturingReality::Sfm::ISfmReconstruction**     ppReconstruction);

HRESULT SetSfmInputCalibrationPrior( 
    __in CapturingReality::Sfm::IStructureFromMotion *pSfm, 
    __in UINT inputIndex,
    __in CapturingReality::SfmCameraCalibration *pK, 
	__in CapturingReality::Sfm::SfmCameraCalibrationFlags flags,
	__in CapturingReality::SfmLensDistortionModel distortionModel );

HRESULT SetSfmInputPositionPrior( 
    __in CapturingReality::Sfm::IStructureFromMotion *pSfm, 
    __in UINT inputIndex,
    __in CapturingReality::Sfm::SfmCoordinateSystem *pPose, 
	__in CapturingReality::Sfm::SfmCoordinateSystemType type,
	__in CapturingReality::Sfm::ICoordinateSystem *pCoordinateSystem );

HRESULT ExportRCComponent( 
	__in_z const wchar_t *pFileName, 
	__in CapturingReality::Sfm::IStructureFromMotion *pSfm,
	__in CapturingReality::Sfm::ISfmReconstruction *pReconstruction );

HRESULT ImportRCComponent( 
    __in_z const wchar_t *pFileName, 
    __in CapturingReality::Sfm::IStructureFromMotion *pSfm,
    __deref_out CapturingReality::Sfm::ISfmReconstruction **ppReconstruction );

// Unlock sparse reconstruction if needed. Some SDK licenses might require this step.
HRESULT UnlockReconstructionsIfNeeded( __in CapturingReality::Sfm::IStructureFromMotion *pSfm );

void GetMeshingAlgorithmSettings( 
    __in CapturingReality::Sfm::ISfmReconstruction *pSfmReconstruction, 
    __out CapturingReality::Mvs::MeshingAlgorithmSettings *pSettings );

HRESULT ApproximateReconstructionBox(
    __out GlobalReconstructionVolume* pBox,
    __in CapturingReality::Sfm::IStructureFromMotion* pSfm,
    __in CapturingReality::Sfm::ISfmReconstruction* pReconstruction,
    __in bool bAxisAlignedBox
);

HRESULT CreateModel(
	__in UINT downscaleFactor,
	__in_opt GlobalReconstructionVolume *pReconstructionRegion,
	__in CapturingReality::IResourceCache *pResourceCache,
	__in CapturingReality::IConfig *pConfig,
	__in CapturingReality::Sfm::IStructureFromMotion *pSfm,
	__in CapturingReality::Sfm::ISfmReconstruction *pSfmReconstruction,
	__deref_out CapturingReality::Mvs::IMvsModel **ppMvsModel );

HRESULT CreateModelPreview(
	__in_opt GlobalReconstructionVolume *pReconstructionRegion,
	__in CapturingReality::IResourceCache *pResourceCache,
	__in CapturingReality::IConfig *pConfig,
	__in CapturingReality::Sfm::IStructureFromMotion *pSfm,
	__in CapturingReality::Sfm::ISfmReconstruction *pSfmReconstruction,
	__deref_out CapturingReality::Mvs::IMvsModel **ppMvsModel );

// Unlock model if needed. Some SDK licenses might requires this step.
HRESULT UnlockModelIfNeeded( 
	__in CapturingReality::Sfm::IStructureFromMotion *pSfm,
	__in CapturingReality::Sfm::ISfmReconstruction *pSfmReconstruction,
	__in CapturingReality::Mvs::IMvsModel *pModel );

HRESULT ColorizeModel( 
	__in UINT imagesDownscaleFactor,
	__in CapturingReality::IResourceCache *pResourceCache,
	__in CapturingReality::Sfm::IStructureFromMotion *pSfm,
	__in CapturingReality::Sfm::ISfmReconstruction *pSfmReconstruction,
	__in CapturingReality::Mvs::IMvsModel *pMvsModel );

HRESULT TextureModel( 
	__in UINT imagesDownscaleFactor,
	__in CapturingReality::IResourceCache *pResourceCache,
	__in CapturingReality::Sfm::IStructureFromMotion *pSfm,
	__in CapturingReality::Sfm::ISfmReconstruction *pSfmReconstruction,
	__in CapturingReality::Mvs::IMvsModel *pMvsModel );

HRESULT TextureModel(
	__in CapturingReality::ModelTools::UnwrapParameters *pUnwrapParams,
	__in CapturingReality::IResourceCache *pResourceCache,
	__in CapturingReality::Sfm::IStructureFromMotion *pSfm,
	__in CapturingReality::Sfm::ISfmReconstruction *pSfmReconstruction,
	__in CapturingReality::Mvs::IMvsModel *pMvsModel);

HRESULT Simplify(
	__in UINT targetTrianglesCount,
	__in CapturingReality::Mvs::IMvsModel *pMvsModel,
	__deref_out CapturingReality::Mvs::IMvsModel **ppMvsModel );

HRESULT SimplifyRelative(
	__in double targetTrianglesCountRelative,
	__in CapturingReality::Mvs::IMvsModel *pMvsModel,
	__deref_out CapturingReality::Mvs::IMvsModel **ppMvsModel );

HRESULT Smooth(
	__in float smoothingWeight,
	__in CapturingReality::Mvs::IMvsModel *pMvsModel,
	__deref_out CapturingReality::Mvs::IMvsModel **ppMvsModel);

//This filtering sample perform:
//1. Select largest connected component and
//2. Invert selection and
//3. Select marginal triangles and
//4. Select large triangles
//5. Invert selection and
//6. Remove unselected triangles
HRESULT FilterSample01( 
	__in float maximalEdgeLength,
	__in CapturingReality::Mvs::IMvsModel *pMvsModel,
	__deref_out CapturingReality::Mvs::IMvsModel **ppMvsModel );

HRESULT GetGridExportTransformationProvider(
	__in CapturingReality::Sfm::ISfmReconstruction *pSfmReconstruction,
	__in CapturingReality::Mvs::IMvsModel *pMvsModel,
	__deref_out CapturingReality::ImportExport::IExportTransformationsProvider **ppExportTransformationsProvider );

HRESULT GetGridExportTransformationProviderWithOutputCoordSys(
	__in CapturingReality::Sfm::ISfmReconstruction *pSfmReconstruction,
	__in CapturingReality::Mvs::IMvsModel *pMvsModel,
	__in const wchar_t* pOutputCoordSys, 
	__in const wchar_t* pOutputCoordSysName,
	__deref_out CapturingReality::ImportExport::IExportTransformationsProvider **ppExportTransformationsProvider);

HRESULT ExportModelPly(
	__in_z const wchar_t *dir,
	__in_z const wchar_t *name,
	__in CapturingReality::IResourceCache *pResourceCache,
	__in CapturingReality::Sfm::IStructureFromMotion *pSfm,
	__in CapturingReality::Sfm::ISfmReconstruction *pSfmReconstruction,
	__in CapturingReality::Mvs::IMvsModel *pMvsModel,
    __in_opt CapturingReality::ImportExport::IExportTransformationsProvider *pTransform = NULL );

HRESULT ExportModelObj(
	__in_z const wchar_t *dir,
	__in_z const wchar_t *name,
	__in CapturingReality::IResourceCache *pResourceCache,
	__in CapturingReality::Sfm::IStructureFromMotion *pSfm,
	__in CapturingReality::Sfm::ISfmReconstruction *pSfmReconstruction,
	__in CapturingReality::Mvs::IMvsModel *pMvsModel,
    __in_opt CapturingReality::ImportExport::IExportTransformationsProvider *pTransform = NULL );

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
	__deref_out CapturingReality::OrthoProjection::IOrthographicProjection **ppOrthoProjection );

HRESULT ExportWorldFile( 
	__in_z const wchar_t *pFileName, 
	__in CapturingReality::OrthoProjection::IOrthographicProjection *pOrthoProjection );

HRESULT CalculateOrthoProjectionAndExportToTiffGPS(
	__in_z wchar_t const* output_dir,
	__in CapturingReality::IResourceCache *pResourceCache,
	__in IStructureFromMotion *pSfm,
	__in ISfmReconstruction *pSfmReconstruction,
	__in IMvsModel *pMvsModel,
	__in GlobalReconstructionVolume *pReconstructionRegion,
	__in float cmPerPixel);

void LogMessageToFile( __in_z const wchar_t *logFileName, const char *format, ... );

HRESULT ComputeUnwrapParamsByFixedTexelSize(
	__in float texelSize,
	__in CapturingReality::Mvs::IMvsModel *pMvsModel,
	__out CapturingReality::ModelTools::UnwrapParameters *pUnwrapParams);

HRESULT ExportModelXYZ(
	__in_z const wchar_t *dir,
	__in_z const wchar_t *name,
	__in CapturingReality::IResourceCache *pResourceCache,
	__in CapturingReality::Sfm::IStructureFromMotion *pSfm,
	__in CapturingReality::Sfm::ISfmReconstruction *pSfmReconstruction,
	__in CapturingReality::Mvs::IMvsModel *pMvsModel,
	__in_opt CapturingReality::ImportExport::IExportTransformationsProvider *pTransform = NULL);

HRESULT ExportModelXYZGPS(
	__in_z const wchar_t *dir,
	__in_z const wchar_t *name,
	__in CapturingReality::IResourceCache *pResourceCache,
	__in CapturingReality::Sfm::IStructureFromMotion *pSfm,
	__in CapturingReality::Sfm::ISfmReconstruction *pSfmReconstruction,
	__in CapturingReality::Mvs::IMvsModel *pMvsModel,
	__in_opt CapturingReality::ImportExport::IExportTransformationsProvider *pTransform = NULL);

HRESULT ExportModelXYZ_old(
	__in_z const WCHAR *pOutputDir,
	__in_z const WCHAR *pFileName,
	__in CapturingReality::Sfm::ISfmReconstruction *pSfmReconstruction,
	__in CapturingReality::Mvs::IMvsModel *pModel);

#include "MathHelpers.h"
#include "MockupProgressReportCallback.h"
