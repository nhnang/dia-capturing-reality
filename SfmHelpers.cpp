//
// Capturing Reality s.r.o.
//
// This file 'SfmHelpers.cpp' is a part of the RC Engine Samples. You may use it freely.
//

#include "stdafx.h"
#include "SamplesCommonLib.h"                             

HRESULT AlignImages( __in_z const wchar_t** ppImageList, __in UINT imagesCount, 
                     __in CapturingReality::IResourceCache *pCache, __in CapturingReality::IConfig *pConfig,
                     __deref_out CapturingReality::Sfm::IStructureFromMotion **ppSfm )
{
    HRESULT hr;

	//the Pipeline can use a database with camera sensors to get rough prior information.
    //We can ignore missing database of sensors, the pipeline works ok without exif focal but is faster with it.
	//You can find this database file included with RealityCapture application executables.
    CComPtr< CapturingReality::Sfm::ISfmSensorDatabase > spSensorsDb;
    CreateSfmSensorDatabase( L"..\\Resources\\metadata\\sensorsdb.xml", &spSensorsDb );
    
    // This is optional if you do not plan to use geo-referenced cameras and you can simply pass NULL
    // to CreateSfmPipeline creator.
    // Coordinates table servers as a container for storing definitions of particular coordinate systems, e.g., WGS 84 (GPS)
    // If table was provided, system will be able to understand GPS coordinates and read them from EXIF.
    // If table was not provided, GPS stored in the image EXIF will be ignored
    CComPtr<CapturingReality::Sfm::ICoordinateSystemTable> spCoordinateSystemTable;
    hr = CreateCoordinateSystemTable( &spCoordinateSystemTable );
    if (FAILED(hr))
        return hr;

    CComPtr<CapturingReality::Sfm::IStructureFromMotion> spSfm;
    hr = CapturingReality::Sfm::CreateSfmPipeline( pConfig, spCoordinateSystemTable, NULL, pCache, spSensorsDb, NULL, &spSfm);
    if (FAILED(hr))
        return hr;

    //Add images
    for ( UINT i = 0; i < imagesCount; i++ )
    {
        hr = spSfm->AddImage(ppImageList[i], NULL, NULL, 0); 
        if (FAILED(hr)) 
            return hr;
    }

    printf("Calculating camera parameters\n");

    //If you wish to receive progress information or add an ability to abort the registration process, implement IProgressReport and pass it to RegisterImages.
    hr = spSfm->RegisterImages(NULL);
    if (FAILED(hr) )
        return hr;

	hr = UnlockReconstructionsIfNeeded(spSfm);
    if (FAILED(hr))
        return hr;

    return spSfm.CopyTo(ppSfm);
}

HRESULT SetSfmInputCalibrationPrior( 
    __in CapturingReality::Sfm::IStructureFromMotion *pSfm, 
    __in UINT inputIndex,
    __in CapturingReality::SfmCameraCalibration *pK, 
	__in CapturingReality::Sfm::SfmCameraCalibrationFlags flags,
	__in CapturingReality::SfmLensDistortionModel distortionModel )
{
    CComPtr< ISfmStreamInput > spInput;
    HRESULT hr = pSfm->GetImage( inputIndex, &spInput );
	if ( FAILED(hr) )
    {
		return hr;
    }

    // cast to image as it can be something else, e.g., laser scan input
    // whose calibration is fixed
    CComQIPtr< ISfmImage > spImate = spInput;
    if ( spImate )
    {
        return E_NOINTERFACE;
    }

    return spImate->SetCalibration( pK, flags, distortionModel );
}

HRESULT SetSfmInputPositionPrior( 
    __in CapturingReality::Sfm::IStructureFromMotion *pSfm, 
    __in UINT inputIndex,
    __in CapturingReality::Sfm::SfmCoordinateSystem *pPose, 
	__in CapturingReality::Sfm::SfmCoordinateSystemType type,
	__in CapturingReality::Sfm::ICoordinateSystem *pCoordinateSystem )
{
    CComPtr< ISfmStreamInput > spInput;
    HRESULT hr = pSfm->GetImage( inputIndex, &spInput );
	if ( FAILED(hr) )
    {
		return hr;
    }

    return spInput->SetPriorPoseAbsolute( pPose, type, pCoordinateSystem ); 
}

HRESULT UnlockReconstructionsIfNeeded( __in CapturingReality::Sfm::IStructureFromMotion *pSfm )
{
	// various licenses scenes are either locked or unlocked and
	// a different unlocking mechanism applies, e.g., PPI, subscriptions,
	// cloud licenses and so on

	using namespace CapturingReality::RealityCaptureServices;

	// unlock reconstructions
	ServiceConnectionContext connectionContext;
	strcpy_s( connectionContext.appToken, SDKSAMPLE_APP_TOKEN );

	CComPtr< IOnlineLicensingManager > spLicensingManager;
	HRESULT hr = CreateOnlineLicensingManager( &connectionContext, NULL, &spLicensingManager );

	if ( SUCCEEDED(hr) )
	{
		ServiceCallContext callContext = {0};

        // uncomment following line if you want UX with external browser
        // which informs user what happens.
        callContext.tokenType[0] = STT_GET;
        callContext.token[0] = "onerror";

		UINT requestEKU[] = {1, 0};
        
		InputsLicensingContext datasetContext = {0};
		datasetContext.pEku = requestEKU;
		datasetContext.pReconstruction = NULL;
		datasetContext.pProgress = NULL;
		datasetContext.pModel = NULL;
		datasetContext.pControlPoints = NULL;
		datasetContext.pSfm = pSfm;

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
    		hr = spLicensingManager->UnlockReconstructions( &callContext, &datasetContext, spPPIManager, "", InputsLicensingPinType::PT_NONE, &outContext );
        }
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
						Sleep(1100);
						hr = spLicensingManager->UnlockReconstructions( &callContext, &datasetContext, spPPIManager, "", InputsLicensingPinType::PT_NONE, &outContext );
					}

					// the user has finished the task or closed the browser
				}
			}
		}
	}
	
	return hr;
}

HRESULT ExportRCComponent( 
	__in_z const wchar_t *pFileName, 
	__in CapturingReality::Sfm::IStructureFromMotion *pSfm,
	__in CapturingReality::Sfm::ISfmReconstruction *pReconstruction )
{
	UINT nImages = pSfm->GetImagesCount();

	UINT *pImages = new UINT[nImages];
    if ( !pImages )
    {
        return E_OUTOFMEMORY;
    }

	for ( UINT i = 0; i < nImages; i++ )
	{
		pImages[i] = i;
	};

	CComPtr<ISfmReconstructionExported> spComponent;
	HRESULT hr = pSfm->CreateSfmReconstructionExported(
			pReconstruction, 
			0,
			NULL,
			nImages, 
			pImages,
			NULL,
			&spComponent);

	if ( SUCCEEDED( hr ) )
	{
        CComPtr< IStream > spStream;
        hr = SHCreateStreamOnFile( pFileName, STGM_CREATE | STGM_WRITE, &spStream );
		if ( SUCCEEDED( hr ) )
		{
			hr = spComponent->SaveToStream( spStream );
		};
	};

	delete[] pImages;

	return hr;
}

HRESULT ImportRCComponent( __in_z const wchar_t *pComponentFileName, __in CapturingReality::Sfm::IStructureFromMotion *pSfm,
                     __deref_out CapturingReality::Sfm::ISfmReconstruction **ppReconstruction )
{
    CComPtr< CapturingReality::Sfm::ISfmReconstructionExported > spComponent;
    HRESULT hr = CapturingReality::Sfm::CreateSfmReconstructionImporter( pComponentFileName, &spComponent );
    if ( FAILED(hr) )
        return hr;

    CComPtr<ISfmReconstruction> spRec;
    hr = spComponent->QueryInterface(&spRec);
    if ( FAILED(hr) )
        return hr;

    // register images
	CapturingReality::SfmCamera *pCameras;
    UINT cameraCount;
    hr = spRec->GetCameras(&pCameras, &cameraCount);
    if ( SUCCEEDED(hr) )
    {
        for (UINT i = 0; i < cameraCount; i++)
        {
            const SfmReconstructionImageInfo *pImageInfo;
            hr = spComponent->GetImageInfo(i, &pImageInfo);
            if ( SUCCEEDED(hr) )
            {
			    const WCHAR *imageFileName = spComponent->GetString(pImageInfo->fileNameIndex);
			    if ( imageFileName )
			    {
					CComPtr< ISfmStreamInput > spStreamInput;
					hr = pSfm->AddImage( imageFileName, NULL, 0, &spStreamInput );
                    if ( FAILED(hr) )
                        return hr;
                    
                    //relocate the sfm index
					pCameras[i].sfmImage = spStreamInput->GetSfmIndex();

                    //optionally, you should copy flags stored in pImageInfo into the sfm image
                }
            }
        }
    }

    if ( SUCCEEDED(hr) )
    {
        hr = spRec.CopyTo( ppReconstruction );
    }

    return hr;
}


HRESULT Align(
	__in        CapturingReality::Sfm::IStructureFromMotion*    pSfm,
	__deref_out CapturingReality::Sfm::ISfmReconstruction**     ppReconstruction)
{
	if (pSfm == NULL)
	{
		return E_INVALIDARG;
	}

	HRESULT hr = S_OK;

	//
	// Align SFM structures with registered input images to obtain a reconstruction.
	//

	if (SUCCEEDED(hr))
	{
		printf("Calculating camera parameters\n");
		hr = pSfm->RegisterImages(NULL);
	}

	//
	// More components can be reconstructed. We select the largest one,
	// which is, most likely, the most interesting one.
	//

	UINT nReconstructions = 0;

	if (SUCCEEDED(hr))
	{
		printf("Unlocking reconstructions\n");
		hr = UnlockReconstructionsIfNeeded(pSfm);
	}

	if (SUCCEEDED(hr))
	{
		nReconstructions = pSfm->GetReconstructionsCount();
		if (nReconstructions == 0)
		{
			*ppReconstruction = NULL;
			return hr;
		}
	}

	UINT largestComponentIndex = 0;

	if (SUCCEEDED(hr))
	{
		UINT largestComponentCameraCount = 0;

		printf("Calculated %d components\n", nReconstructions);

		for (UINT ireconstruction = 0; ireconstruction < nReconstructions; ireconstruction++)
		{
			CComPtr< CapturingReality::Sfm::ISfmReconstruction > spReconstruction;
			UINT structureSize;

			hr = pSfm->GetReconstruction(ireconstruction, &spReconstruction);
			if (SUCCEEDED(hr))
			{
				hr = spReconstruction->GetStructureSize(&structureSize);
			}
			if (SUCCEEDED(hr))
			{
				UINT cameraCount = spReconstruction->GetCamerasCount();
				printf("Component %d consists of %d cameras and %d global points\n", ireconstruction, cameraCount, structureSize);

				if (cameraCount >= largestComponentCameraCount)
				{
					largestComponentIndex = ireconstruction;
					largestComponentCameraCount = cameraCount;
				}
			}
			else
			{
				break;
			}
		}
	}

	// Select the component with the largest number of aligned cameras.

	if (SUCCEEDED(hr))
	{
		hr = pSfm->GetReconstruction(largestComponentIndex, ppReconstruction);
	}

	return hr;
}