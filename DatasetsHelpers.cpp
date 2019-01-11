//
// Capturing Reality s.r.o.
//
// This file 'DatasetsHelpers.cpp' is a part of the RC Engine Samples. You may use it freely.
//

#include "stdafx.h"
#include "SamplesCommonLib.h"

HRESULT AcquireDatasetLicense( __in IInputsLicensingManager *pLicenseManager, __in_z const UINT *pRequestEKU, __in ISfmReconstruction *pReconstruction )
{
	CComPtr< IOnlineLicensingManager >spLicensingManager;
	
	ServiceConnectionContext connectionContext;
	strcpy_s( connectionContext.appToken, SDKSAMPLE_APP_TOKEN );
	
	HRESULT hr = CreateOnlineLicensingManager( &connectionContext, NULL, &spLicensingManager );
	if ( SUCCEEDED(hr) )
	{
		ServiceCallContext callContext = {0};
		callContext.tokenType[0] = STT_GET;
		callContext.token[0] = NULL;

		InputsLicensingContext datasetContext = {0};
		datasetContext.pEku = pRequestEKU;
		datasetContext.pReconstruction = pReconstruction;
		datasetContext.pProgress = NULL;
		datasetContext.pModel = NULL;;
		datasetContext.pControlPoints = NULL;
		datasetContext.pSfm = NULL;

		InputsLicensingResultContext datasetResultContext = {0};

		hr = spLicensingManager->GetInputLicense( &callContext, &datasetContext, pLicenseManager, NULL, InputsLicensingPinType::PT_NONE, &datasetResultContext, NULL );
		if ( SUCCEEDED(hr) )
		{
			// we have obtained an interchange token
			WCHAR urlText[1024];
			hr = spLicensingManager->FormatRequestURL( &datasetResultContext.authContext, urlText, _countof(urlText) );
			if ( SUCCEEDED(hr) )
			{
				// now we have a URL which can be opened in an Internet browser component. 
				// The user will follow on-screen instructions to complete the process
				hr = OpenWebBrowser( urlText );
			}

			if ( SUCCEEDED(hr) )
			{
				// get a license, if the user successfully selected the license and finished the process,
				// otherwise GetLicense returns the interchange token again
				callContext = datasetResultContext.authContext;
				callContext.token[0] = callContext.scratchMem;
				
				// user's reaction time is not less than 5 seconds
				Sleep(5000);

				// wait for the user to respond
				while ( SUCCEEDED(hr) )
				{
					Sleep(1100);

					CComPtr< IUnknown > spDatasetCertificate;
					hr = spLicensingManager->GetInputLicense( &callContext, &datasetContext, pLicenseManager, NULL, InputsLicensingPinType::PT_NONE, &datasetResultContext, &spDatasetCertificate );

					// two options for end-of-loop testing
					//  1. test if the dataset certificate has been issued
					//  2. dataset result context will not contain the interchange token
					if ( spDatasetCertificate ) 
					{
						// finished successfully, you can work with the data within the granted EKU freely
						break;
					}
				}

				// the user has finished the task or closed the browser
			}
		}
	}

	return hr;
}