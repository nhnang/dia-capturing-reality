//
// Capturing Reality s.r.o.
//
// This file 'ModelHelpers.cpp' is a part of the RC Engine Samples. You may use it freely.
//

#include "stdafx.h"
#include "SamplesCommonLib.h"

using namespace CapturingReality;

void GetMeshingAlgorithmSettings( __in CapturingReality::Sfm::ISfmReconstruction *pSfmReconstruction, __out CapturingReality::Mvs::MeshingAlgorithmSettings *pSettings )
{
	pSettings->SetDefaults();

	//Define a level of detail
	pSettings->settings.decimationFactor = 1.0f;

	//The maximal number of vertices in a part
	pSettings->settings.targetVertexCount = 5000000; //default value

	//This is transformation from the root SfM coordinate system to actual SfM coordinate system
	//These can be different when constraints are updated after component computation
	pSettings->residualTransform = pSfmReconstruction->GetResidualTransform();

	//The minimal sample distance for laser scan points in meters (0.002 = 2mm)
	pSettings->settings.depthDetail.laserscanMinSamplingDistance = 0.002f;
};

HRESULT CreateModel(
	__in UINT downscaleFactor,
	__in_opt GlobalReconstructionVolume *pReconstructionRegion,
	__in CapturingReality::IResourceCache *pResourceCache,
	__in CapturingReality::IConfig *pConfig,
	__in CapturingReality::Sfm::IStructureFromMotion *pSfm,
	__in CapturingReality::Sfm::ISfmReconstruction *pSfmReconstruction,
	__deref_out CapturingReality::Mvs::IMvsModel **ppMvsModel )
{
	CComPtr< CapturingReality::IImageProvider > spImageProvider;
	CComPtr< CapturingReality::Mvs::Depthmaps::ICameraList > spCameraList;
	CComPtr< CapturingReality::ISceneStructure > spSceneStructure;

	HRESULT hr = CapturingReality::Mvs::CreateUndistortedImageProvider( pSfm, pSfmReconstruction, &spImageProvider );
	if ( SUCCEEDED( hr ) )
	{
		hr = CapturingReality::Mvs::CreateCameraList( NULL, pSfm, pSfmReconstruction, &spCameraList );
		if ( SUCCEEDED( hr ) )
		{
			hr = CapturingReality::Mvs::CreateSceneStructure( pSfmReconstruction, &spSceneStructure );
			if ( SUCCEEDED( hr ) )
			{
				CapturingReality::Mvs::Depthmaps::DepthMapAlgorithmSettings depthMapAlgorithmSettings;
				depthMapAlgorithmSettings.SetDefaults();

				//Set a downscale factor for images before depth-map computation
				if ( downscaleFactor == 0 )
				{
					//in the preview mode use one
					depthMapAlgorithmSettings.downscaleFactor = 1;
				}
				else
				{
					depthMapAlgorithmSettings.downscaleFactor = downscaleFactor;
				};

				//Pair depth maps with SFM reconstruction.
				hr = pSfmReconstruction->GetComponentId( &depthMapAlgorithmSettings.componentGUID );
				if ( SUCCEEDED( hr ) )
				{
					CapturingReality::Mvs::ColoringAlgorithmSettings coloringAlgorithmSettings;
					coloringAlgorithmSettings.SetDefaults( NULL );

					CapturingReality::Mvs::MeshingAlgorithmSettings meshingAlgorithmSettings;
					GetMeshingAlgorithmSettings( pSfmReconstruction, &meshingAlgorithmSettings );

					_CrtCheckMemory();

					MockupProgressReportCallback progressReportCallback;

					CapturingReality::Mvs::CreateMvsModelParameters mvsApplicationParams = CapturingReality::Mvs::CreateMvsModelParameters();
					mvsApplicationParams.pImageProvider = spImageProvider;
					mvsApplicationParams.pCameraList = spCameraList;
					mvsApplicationParams.pSceneStructure = spSceneStructure;
					mvsApplicationParams.pColoringAlgorithmSettings = &coloringAlgorithmSettings;
					mvsApplicationParams.pProgressReportCallback = &progressReportCallback;
					mvsApplicationParams.pDepthMapAlgorithmSettings = &depthMapAlgorithmSettings;
					mvsApplicationParams.pMeshingAlgorithmSettings = &meshingAlgorithmSettings;
					mvsApplicationParams.pResourceCache = pResourceCache;

					if ( pReconstructionRegion == NULL )
					{
						mvsApplicationParams.pMeshingAlgorithmSettings->hasUserSelectedBox = false;
					}
					else
					{
						mvsApplicationParams.pMeshingAlgorithmSettings->hasUserSelectedBox = true;
						mvsApplicationParams.pMeshingAlgorithmSettings->userSelectedBox = *pReconstructionRegion;
					};

					CComPtr< CapturingReality::Mvs::IMvsModel > spMvsModel;

					if ( downscaleFactor == 0 )
					{
						//preview mode
						hr = CapturingReality::ModelTools::CreateMvsModelSfm( pSfmReconstruction, &mvsApplicationParams, pConfig, &spMvsModel );
					}
					else
					{
						LONG failedPartsCount;
						hr = CapturingReality::ModelTools::CreateMvsModelMultiPart( &mvsApplicationParams, pConfig, &spMvsModel, &failedPartsCount );
					};

					if ( SUCCEEDED( hr ) )
					{
						*ppMvsModel = spMvsModel.Detach();
					};
				};
			};
		};
	};

	return hr;
}

HRESULT CreateModelPreview(
	__in_opt GlobalReconstructionVolume *pReconstructionRegion,
	__in CapturingReality::IResourceCache *pResourceCache,
	__in CapturingReality::IConfig *pConfig,
	__in CapturingReality::Sfm::IStructureFromMotion *pSfm,
	__in CapturingReality::Sfm::ISfmReconstruction *pSfmReconstruction,
	__deref_out CapturingReality::Mvs::IMvsModel **ppMvsModel )
{
	return CreateModel( 0, pReconstructionRegion, pResourceCache, pConfig, pSfm, pSfmReconstruction, ppMvsModel );
};

HRESULT ColorizeModel(
	__in UINT imagesDownscaleFactor,
	__in CapturingReality::IResourceCache *pResourceCache,
	__in CapturingReality::Sfm::IStructureFromMotion *pSfm,
	__in CapturingReality::Sfm::ISfmReconstruction *pSfmReconstruction,
	__in CapturingReality::Mvs::IMvsModel *pMvsModel )
{
	CComPtr< CapturingReality::IImageProvider > spImageProvider;
	CComPtr< CapturingReality::Mvs::Depthmaps::ICameraList > spCameraList;
	CComPtr< CapturingReality::ISceneStructure > spSceneStructure;

	HRESULT hr = CapturingReality::Mvs::CreateUndistortedImageProvider( pSfm, pSfmReconstruction, &spImageProvider );
	if ( SUCCEEDED( hr ) )
	{
		CoordinateSystemResidual rootToModel = pMvsModel->GetResidualTransform();
		hr = CapturingReality::Mvs::CreateCameraList( &rootToModel, pSfm, pSfmReconstruction, &spCameraList );
		if ( SUCCEEDED( hr ) )
		{
			hr = CapturingReality::Mvs::CreateSceneStructure( pSfmReconstruction, &spSceneStructure );
			if ( SUCCEEDED( hr ) )
			{
				CComPtr< CapturingReality::Mvs::Depthmaps::ICameraListContext > spCameraListContext;
				hr = CapturingReality::Mvs::CreateCameraListContext( spCameraList, spSceneStructure, &spCameraListContext );
				if ( SUCCEEDED( hr ) )
				{
					CapturingReality::Mvs::ColoringAlgorithmSettings coloringAlgorithmSettings;
					coloringAlgorithmSettings.SetDefaults( NULL );
				
					//All images will be downscaled by this factor before coloring
					coloringAlgorithmSettings.coloringImagesDownScale = imagesDownscaleFactor;
					coloringAlgorithmSettings.coloringPreviewMode = false;				
					MockupProgressReportCallback progressReportCallback;

					hr = CapturingReality::Mvs::ColorMvsModel( &coloringAlgorithmSettings, spImageProvider, spCameraListContext, spSceneStructure, pMvsModel, &progressReportCallback );
				};
			};
		};
	};

	return hr;
};

HRESULT ComputeUnwrapParams( 
	__in CapturingReality::Mvs::IMvsModel *pMvsModel, 
	__in CapturingReality::Mvs::Depthmaps::ICameraList *pCameraList,
	__in CapturingReality::ISceneStructure *pSceneStructure,
	__in_opt GlobalReconstructionVolume *pBoundigBox,
	__out CapturingReality::ModelTools::UnwrapParameters *pUnwrapParams )
{
	MockupProgressReportCallback progressReportCallback;
	double area, avel;
	HRESULT hr = CapturingReality::ModelTools::CalculateAverageEdgeLength( pMvsModel, &progressReportCallback, &avel );
	if ( SUCCEEDED( hr ) )
	{
		hr = CapturingReality::ModelTools::CalculateCubeUnwrappedSurfaceArea( (float)(avel * 10.0), pMvsModel, &progressReportCallback, &area );
		if ( SUCCEEDED( hr ) )
		{
			float detail;
			hr = CapturingReality::ModelTools::EstimateDetail( pCameraList, pSceneStructure, pBoundigBox, &detail );
			if ( SUCCEEDED( hr ) )
			{
				pUnwrapParams->maximalEdgeLength = 10.0f * (float)avel;
				pUnwrapParams->gutterSize = 2;
				pUnwrapParams->textureResolution = 4096;
				pUnwrapParams->fixedTexelSize = detail;
				pUnwrapParams->minAdaptiveTexelSize = detail;
				pUnwrapParams->maxAdaptiveTexelSize = detail * 10;
				pUnwrapParams->useAdaptiveTexelSize = false;
			};
		};
	};
	return hr;
};

HRESULT TextureModel( 
	__in UINT imagesDownscaleFactor,
	__in CapturingReality::IResourceCache *pResourceCache,
	__in CapturingReality::Sfm::IStructureFromMotion *pSfm,
	__in CapturingReality::Sfm::ISfmReconstruction *pSfmReconstruction,
	__in CapturingReality::Mvs::IMvsModel *pMvsModel )
{
	CComPtr< CapturingReality::IImageProvider > spUndistortedImageProvider;
	CComPtr< CapturingReality::IImageProvider > spDistortedImageProvider;
	CComPtr< CapturingReality::Mvs::Depthmaps::ICameraList > spCameraList;
	CComPtr< CapturingReality::ISceneStructure > spSceneStructure;

	HRESULT hr = CapturingReality::Mvs::CreateUndistortedImageProvider( pSfm, pSfmReconstruction, &spUndistortedImageProvider );
	if ( SUCCEEDED( hr ) )
	{
		hr = CapturingReality::Mvs::CreateDistortedImageProvider( pSfm, pSfmReconstruction, &spDistortedImageProvider );
		if ( SUCCEEDED( hr ) )
		{
			CoordinateSystemResidual rootToModel = pMvsModel->GetResidualTransform();
			hr = CapturingReality::Mvs::CreateCameraList( &rootToModel, pSfm, pSfmReconstruction, &spCameraList );
			if ( SUCCEEDED( hr ) )
			{
				hr = CapturingReality::Mvs::CreateSceneStructure( pSfmReconstruction, &spSceneStructure );
				if ( SUCCEEDED( hr ) )
				{
					CapturingReality::Mvs::TexturingAlgorithmSettings texturingAlgorithmSettings;
					texturingAlgorithmSettings.SetDefaults();

					//All images will be downscaled with this factor before texturing
					texturingAlgorithmSettings.imagesDownscaleFactor = imagesDownscaleFactor;

					MockupProgressReportCallback progressReportCallback;

					CapturingReality::ModelTools::UnwrapParameters unwrapParams;
					hr = ComputeUnwrapParams( pMvsModel, spCameraList, spSceneStructure, NULL, &unwrapParams );
					if ( SUCCEEDED( hr ) )
					{
						//ModelTools::ModelUnwrapTextureUtilization utilization;
						//hr = CapturingReality::ModelTools::UnwrapModel( &unwrapParams, pSfmReconstruction, spCameraList, pMvsModel, &progressReportCallback, &utilization );
						double u1,u2;
						hr = CapturingReality::ModelTools::UnwrapModelLegacy( &unwrapParams, &progressReportCallback, pMvsModel, pSfmReconstruction, spCameraList, &u1, &u2 );
						if ( SUCCEEDED( hr ) )
						{
							CComPtr< CapturingReality::Mvs::Depthmaps::ICameraListContext > spCameraListContext;
							hr = Mvs::CreateCameraListContext( spCameraList, spSceneStructure, &spCameraListContext );
							if ( SUCCEEDED( hr ) )
							{
								hr = CapturingReality::ModelTools::TextureModel(
									&texturingAlgorithmSettings, 
									spUndistortedImageProvider, 
									spDistortedImageProvider, 
									spCameraListContext, 
									spSceneStructure, 
									pMvsModel, 
									&progressReportCallback );
							};			
						};
					};
				};
			};
		};
	};

	return hr;
};

HRESULT Simplify(
	__in UINT targetTrianglesCount,
	__in CapturingReality::Mvs::IMvsModel *pMvsModel,
	__deref_out CapturingReality::Mvs::IMvsModel **ppMvsModel )
{
	CapturingReality::ModelPostProcess::ModelSimplificationParams params;
	params.preserveParts = true;
	params.minimalEdgeLength = 0.0;
	params.targetTrianglesCountAbsolute = targetTrianglesCount;
	params.targetTrianglesCountRelative = 0.0;

	UINT corruptedPartsCount;
	return CapturingReality::ModelTools::Simplify( pMvsModel, &params, NULL, ppMvsModel, &corruptedPartsCount );
}

HRESULT SimplifyRelative(
	__in double targetTrianglesCountRelative,
	__in CapturingReality::Mvs::IMvsModel *pMvsModel,
	__deref_out CapturingReality::Mvs::IMvsModel **ppMvsModel )
{
	CapturingReality::ModelPostProcess::ModelSimplificationParams params;
	params.preserveParts = true;
	params.minimalEdgeLength = 0.0;
	params.targetTrianglesCountAbsolute = 0;
	params.targetTrianglesCountRelative = targetTrianglesCountRelative;

	UINT corruptedPartsCount;
	return CapturingReality::ModelTools::Simplify( pMvsModel, &params, NULL, ppMvsModel, &corruptedPartsCount );
}

HRESULT Smooth(
	__in float smoothingWeight,
	__in CapturingReality::Mvs::IMvsModel *pMvsModel,
	__deref_out CapturingReality::Mvs::IMvsModel **ppMvsModel)
{
	CapturingReality::ModelPostProcess::ModelSmoothingParams params;

	CapturingReality::Mvs::ColoringByNormalParams coloringByNormalParams;
	coloringByNormalParams.SetDefaults(NULL);

	params.doColorizeByNormals = true;
	params.coloringByNormalParams = coloringByNormalParams;

	params.type = CapturingReality::ModelPostProcess::SmoothingType::SMT_NOISE_REMOVAL;
	params.style = CapturingReality::ModelPostProcess::SmoothingStyle::SMS_JUST_BORDER;

	params.smoothingWeight = smoothingWeight;
	params.smoothingIters = 5;
	
	CComPtr< IItemSelection > spSelection;
	HRESULT hr = CapturingReality::ModelTools::SelectMarginalTriangles(pMvsModel, NULL, &spSelection);
	if (SUCCEEDED(hr))
	{
		return CapturingReality::ModelTools::Smooth(pMvsModel, &params, spSelection, NULL, ppMvsModel);
	}
	else
	{
		return hr;
	}	
}

HRESULT UnlockModelIfNeeded( 
	__in CapturingReality::Sfm::IStructureFromMotion *pSfm,
	__in CapturingReality::Sfm::ISfmReconstruction *pSfmReconstruction,
	__in CapturingReality::Mvs::IMvsModel *pModel )
{
	// various licenses models are either locked or unlocked and
	// a different unlocking mechanism applies, e.g., PPI, subscriptions,
	// cloud licenses and so on
	using namespace CapturingReality::RealityCaptureServices;

	ServiceConnectionContext connectionContext;
	strcpy_s( connectionContext.appToken, SDKSAMPLE_APP_TOKEN );

	printf( "Creating online licensing manager ... \n" );
	CComPtr< IOnlineLicensingManager > spLicensingManager;
	HRESULT hr = CreateOnlineLicensingManager( &connectionContext, NULL, &spLicensingManager );
	if ( SUCCEEDED( hr ) )
	{
		ServiceCallContext callContext = {0};

        // force a user dialog ... comment this line if you want a silent experience
        callContext.tokenType[0] = STT_GET;
        callContext.token[0] = "onerror";
		
		UINT requestEKU[] = {1, 0};
        
		InputsModelContext modelContext = {0};
		modelContext.pEku = requestEKU;
		modelContext.pReconstruction = pSfmReconstruction;
		modelContext.pProgress = NULL;
		modelContext.pSfm = pSfm;
		modelContext.pModel = pModel;



        // initialize input licenses storage
        static CComPtr< IInputsLicensingManager > spPPIManager;
        if ( !spPPIManager )
        {
            // the licensing manager is static and hence all
            // gained licenses
            hr = CreateInputsLicensingManager( &spPPIManager );
        }

		InputsLicensingResultContext outContext = {0};
		if ( SUCCEEDED(hr) )
        {
			printf( "Unlocking model ... \n" );
			hr = spLicensingManager->UnlockModel( &callContext, &modelContext, spPPIManager, "", InputsLicensingPinType::PT_NONE, &outContext );
		};

		if ( SUCCEEDED(hr) )
        {
            if ( outContext.authContext.tokenType[0] == STT_INTERCHANGE_TOKEN )
			{
				// user interaction / authorization is requested
				// this is done through the secured web-based interface
				WCHAR urlText[1024];
				hr = spLicensingManager->FormatRequestURL( &outContext.authContext, urlText, _countof(urlText) );
				if ( SUCCEEDED(hr) )
				{
					// Now we have a URL which can be opened in an Internet browser component. 
					// The user will follow on-screen instructions to complete the process
					printf( "Opening browser ... \n" );
					hr = OpenWebBrowser( urlText );
				}

				if ( SUCCEEDED(hr) )
				{
					// wait for the user to finish
					callContext = outContext.authContext;
					callContext.token[0] = callContext.scratchMem;

					// wait for the user's response
					while ( SUCCEEDED(hr) && (outContext.authContext.tokenType[0] == STT_INTERCHANGE_TOKEN) )
					{
						printf( "Waiting ... \n" );
						Sleep(1000);
						printf( "Unlocking model ... \n" );
						hr = spLicensingManager->UnlockModel( &callContext, &modelContext, spPPIManager, "", InputsLicensingPinType::PT_NONE, &outContext );
					}

					// the user has finished the task or closed the browser
				}
			}
        }
    }

    return hr;
}

HRESULT FilterSample01( 
	__in float maximalEdgeLength,
	__in CapturingReality::Mvs::IMvsModel *pMvsModel,
	__deref_out CapturingReality::Mvs::IMvsModel **ppMvsModel )
{
	CComPtr< CapturingReality::ModelTools::IConnectedComponentsMetadata > spCcMetadata;
	HRESULT hr = CapturingReality::ModelTools::CreateConnectedComponentsMetadata( pMvsModel, NULL, &spCcMetadata );
	
	CComPtr< IItemSelection > spSelection;
	if ( SUCCEEDED( hr ) )
	{
		hr = CapturingReality::ModelTools::SelectMaximalConnectedComponent( pMvsModel, spCcMetadata, NULL, &spSelection );
	};

	if ( SUCCEEDED( hr ) )
	{
		hr = CapturingReality::ModelTools::InvertExactSelection( spSelection );
	};

	if ( SUCCEEDED( hr ) )
	{
		CComPtr< IItemSelection > spSelection2;
		hr = CapturingReality::ModelTools::SelectMarginalTriangles( pMvsModel, NULL, &spSelection2 );
		if ( SUCCEEDED( hr ) )
		{
			CComPtr< IItemSelection > spSelection3;
			hr = spSelection->Union( spSelection2, &spSelection3 );
			if ( SUCCEEDED( hr ) )
			{
				spSelection.Release();
				spSelection = NULL;
				spSelection = spSelection3;
			};
		};
	};

	if ( SUCCEEDED( hr ) )
	{
		double averageEdgeLength;
		hr = CapturingReality::ModelTools::CalculateAverageEdgeLength( pMvsModel, NULL, &averageEdgeLength );
		if ( SUCCEEDED( hr ) )
		{
			double edgeLengthThr = averageEdgeLength * (double)maximalEdgeLength;
			CComPtr< IItemSelection > spSelection2;
			hr = CapturingReality::ModelTools::SelectTrianglesByEdgeSize( 
					edgeLengthThr, 
					CapturingReality::ModelTools::SelectTrianglesByEdgeSizeType::STBES_LARGE_TRIANGLES, 
					pMvsModel, 
					NULL, 
					&spSelection2 );
			if ( SUCCEEDED( hr ) )
			{
				CComPtr< IItemSelection > spSelection3;
				hr = spSelection->Union( spSelection2, &spSelection3 );
				if ( SUCCEEDED( hr ) )
				{
					spSelection.Release();
					spSelection = NULL;
					spSelection = spSelection3;
				};
			};
		};
	};

	if ( SUCCEEDED( hr ) )
	{
		hr = CapturingReality::ModelTools::InvertExactSelection( spSelection );
	};


	if ( SUCCEEDED( hr ) )
	{
		hr = CapturingReality::ModelTools::RemoveUnselectedTriangles( pMvsModel, spSelection, NULL, ppMvsModel );
	};

	return hr;
}


HRESULT ComputeUnwrapParamsByFixedTexelSize(
	__in float texelSize,
	__in CapturingReality::Mvs::IMvsModel *pMvsModel,
	__out CapturingReality::ModelTools::UnwrapParameters *pUnwrapParams)
{
	MockupProgressReportCallback progressReportCallback;
	double avel;
	HRESULT hr = CapturingReality::ModelTools::CalculateAverageEdgeLength(pMvsModel, &progressReportCallback, &avel);
	if (SUCCEEDED(hr))
	{
		pUnwrapParams->maximalEdgeLength = 10.0f * (float)avel;
		pUnwrapParams->gutterSize = 2;
		pUnwrapParams->textureResolution = 4096;
		pUnwrapParams->fixedTexelSize = texelSize;
		pUnwrapParams->minAdaptiveTexelSize = texelSize;
		pUnwrapParams->maxAdaptiveTexelSize = texelSize * 10;
		pUnwrapParams->useAdaptiveTexelSize = false;
	};
	return hr;
};


HRESULT TextureModel(
	__in CapturingReality::ModelTools::UnwrapParameters *pUnwrapParams,
	__in CapturingReality::IResourceCache *pResourceCache,
	__in CapturingReality::Sfm::IStructureFromMotion *pSfm,
	__in CapturingReality::Sfm::ISfmReconstruction *pSfmReconstruction,
	__in CapturingReality::Mvs::IMvsModel *pMvsModel)
{
	CComPtr< CapturingReality::IImageProvider > spUndistortedImageProvider;
	CComPtr< CapturingReality::IImageProvider > spDistortedImageProvider;
	CComPtr< CapturingReality::Mvs::Depthmaps::ICameraList > spCameraList;
	CComPtr< CapturingReality::ISceneStructure > spSceneStructure;

	HRESULT hr = CapturingReality::Mvs::CreateUndistortedImageProvider(pSfm, pSfmReconstruction, &spUndistortedImageProvider);
	if (SUCCEEDED(hr))
	{
		hr = CapturingReality::Mvs::CreateDistortedImageProvider(pSfm, pSfmReconstruction, &spDistortedImageProvider);
		if (SUCCEEDED(hr))
		{
			CoordinateSystemResidual rootToModel = pMvsModel->GetResidualTransform();
			hr = CapturingReality::Mvs::CreateCameraList(&rootToModel, pSfm, pSfmReconstruction, &spCameraList);
			if (SUCCEEDED(hr))
			{
				hr = CapturingReality::Mvs::CreateSceneStructure(pSfmReconstruction, &spSceneStructure);
				if (SUCCEEDED(hr))
				{
					CapturingReality::Mvs::TexturingAlgorithmSettings texturingAlgorithmSettings;
					texturingAlgorithmSettings.SetDefaults();

					//All images will be downscaled with this factor before texturing
					texturingAlgorithmSettings.imagesDownscaleFactor = 1;

					MockupProgressReportCallback progressReportCallback;

					//ModelTools::ModelUnwrapTextureUtilization utilization;
					//hr = CapturingReality::ModelTools::UnwrapModel( &unwrapParams, pSfmReconstruction, spCameraList, pMvsModel, &progressReportCallback, &utilization );
					double u1, u2;
					hr = CapturingReality::ModelTools::UnwrapModelLegacy(pUnwrapParams, &progressReportCallback, pMvsModel, pSfmReconstruction, spCameraList, &u1, &u2);
					if (SUCCEEDED(hr))
					{
						CComPtr< CapturingReality::Mvs::Depthmaps::ICameraListContext > spCameraListContext;
						hr = Mvs::CreateCameraListContext(spCameraList, spSceneStructure, &spCameraListContext);
						if (SUCCEEDED(hr))
						{
							hr = CapturingReality::ModelTools::TextureModel(
								&texturingAlgorithmSettings,
								spUndistortedImageProvider,
								spDistortedImageProvider,
								spCameraListContext,
								spSceneStructure,
								pMvsModel,
								&progressReportCallback);
						};
					};
				};
			};
		};
	};

	return hr;
};