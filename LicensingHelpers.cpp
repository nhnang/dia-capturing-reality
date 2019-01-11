//
// Capturing Reality s.r.o.
//
// This file 'LicensingHelpers.cpp' is a part of the RC Engine Samples. You may use it freely.
//

#include "stdafx.h"
#include "SamplesCommonLib.h"
#include <time.h>
#include <stdio.h>

HRESULT GetLicense( __out_ecount_z(maxKeySize) char *pKey, __in UINT maxKeySize, __inout_ecount_z_opt(128) char *pRenewToken )
{
    if ( !pKey )
    {
        return E_POINTER;
    }

	CComPtr< IOnlineLicensingManager >spLicensingManager;
	
	ServiceConnectionContext connectionContext;
	strcpy_s( connectionContext.appToken, SDKSAMPLE_APP_TOKEN );
	
	HRESULT hr = CreateOnlineLicensingManager( &connectionContext, NULL, &spLicensingManager );
	if ( SUCCEEDED(hr) )
	{
	    UINT realKeySize = 0;
		ServiceCallContext callContext = {0};
		ServiceCallContext outContext = {0};

        int tokensCount = 0;

        // RC services support two modes: 
        //   always - services will return a token, which can be used to formulate a URL with web a request
        //   onerror - which will return a token only if any user interaction is needed
        callContext.tokenType[tokensCount] = STT_GET;
	    callContext.token[tokensCount] = "onerror";
        tokensCount++;

        if ( pRenewToken && pRenewToken[0] )
        {
            // Try the provided renew token. If it is valid, then RealityCapture service will
            // return a license right away
            callContext.tokenType[tokensCount] = STT_LICENSE_TOKEN;
	        callContext.token[tokensCount] = pRenewToken;
            tokensCount++;
        }

		hr = spLicensingManager->GetLicense( &callContext, pKey, maxKeySize, &realKeySize, &outContext );
		if ( SUCCEEDED(hr) )
		{
			if ( realKeySize > 0 )
			{
				// license was issued immediately
				return S_OK;
			}

			// We have obtained an interchange token which is stored in outContext. 
            // The next step is to create a URL where the user should follow the necessary steps.
			WCHAR urlText[1024];
			hr = spLicensingManager->FormatRequestURL( &outContext, urlText, _countof(urlText) );

			if ( SUCCEEDED(hr) )
			{
				// Now we have a URL which can be opened in an Internet browser component. 
				// The user will follow on-screen instructions to complete the process.
				hr = OpenWebBrowser( urlText );
			}

			if ( SUCCEEDED(hr) )
			{
				// get a license, if the user successfully selected the license and finished the process,
				// otherwise GetLicense returns the interchange token again
				callContext = outContext;
				callContext.token[0] = callContext.scratchMem;
				
				// user's reaction time is not less than 5 seconds
				Sleep(5000);

				// wait for the user to respond
				while ( SUCCEEDED(hr) && !realKeySize )
				{
					Sleep(1100);
					hr = spLicensingManager->GetLicense( &callContext, pKey, maxKeySize, &realKeySize, &outContext );
				}

				// user either selected a license or canceled/closed the web browser
			}
        }

        if ( SUCCEEDED(hr) && pRenewToken )
        {
            // If the user selected the renew option during the license acquisition, then a renew token for this license 
            // is created and stored in outContext. 
            pRenewToken[0] = 0;
            for ( int i = 0; i < _countof(outContext.tokenType); i++ )
            {
                if ( outContext.tokenType[i] == STT_LICENSE_TOKEN )
                {
                    // the license renew token has been found
                    strcpy_s( pRenewToken, 128, outContext.token[i] );
                    break;
                }
            }
        }
	}

	return hr;
}

HRESULT RenewLicense( __out_ecount_z(maxKeySize) char *pKey, __in UINT maxKeySize, __inout_ecount_z(128) char *pRenewToken )
{
    if ( !pKey )
    {
        return E_POINTER;
    }
    if ( !pRenewToken )
    {
        return E_INVALIDARG;
    }
    
	CComPtr< IOnlineLicensingManager >spLicensingManager;
	
	ServiceConnectionContext connectionContext;
	strcpy_s( connectionContext.appToken, SDKSAMPLE_APP_TOKEN );
	
	HRESULT hr = CreateOnlineLicensingManager( &connectionContext, NULL, &spLicensingManager );
	if ( SUCCEEDED(hr) )
	{
		ServiceCallContext callContext = {0};
    	callContext.tokenType[0] = STT_LICENSE_TOKEN;
	    callContext.token[0] = pRenewToken;
        
		UINT realKeySize;
		ServiceCallContext outContext = {0};
		hr = spLicensingManager->GetLicense( &callContext, pKey, maxKeySize, &realKeySize, &outContext );
		if ( SUCCEEDED(hr) )
		{
            // If the user selected the renew option during the license acquisition, then a renew token for this license 
            // is created and stored in outContext. 
            for ( int i = 0; i < _countof(outContext.tokenType); i++ )
            {
                if ( outContext.tokenType[i] == STT_LICENSE_TOKEN )
                {
                    // license renew token found
                    strcpy_s( pRenewToken, 128, outContext.token[i] );
                    break;
                }
            }
		}
	}

	return hr;
}

HRESULT ActivateSdk( __in_z const char *pKey ) 
{
	CComPtr< IUnknown > spActivationContext;

	// try activating the existing cached license
	HRESULT hr = CapturingReality::RealityCaptureEngine::Unlock(SDKSAMPLE_APP_TOKEN, pKey);

#ifdef RC_IMPORT_EXPORT_LIBRARY
	if (SUCCEEDED(hr))
	{
		hr = CapturingReality::ImportExport::Unlock(SDKSAMPLE_APP_TOKEN, pKey);
	}
#endif

#ifdef RC_IMAGE_CODEC_LIBRARY
	if (SUCCEEDED(hr))
	{
		hr = CapturingReality::ImageCodecPack::Unlock(SDKSAMPLE_APP_TOKEN, pKey);
	}
#endif

	return hr;
}

HRESULT ActivateSdk()
{
    char key[2048 + 1] = {0};

    // try loading the license from a file
    const WCHAR *pLicenseCacheFile = L"license.txt";
    if ( SUCCEEDED( ReadText( pLicenseCacheFile, key, _countof(key) ) ) )
    {
        HRESULT hr = ActivateSdk( key );
		if ( SUCCEEDED(hr) )
		{
			return hr;
		}

        _wremove( pLicenseCacheFile );
    }

    // Use a renew token if available instead of the user interaction. This is possible only if the user 
    // marked "enable renew" in the web-activation dialog last time.
    // The user can un-check this option in MY account anytime so that the app does not renew the license
    // again
    char keyRenewToken[128] = {0};
    const WCHAR *pLicensetokenCacheFile = L"license-token.txt";
    ReadText( pLicensetokenCacheFile, keyRenewToken, _countof(keyRenewToken) );

    // Getting the license online, since the cached license has not worked
    HRESULT hr = GetLicense( key, _countof(key), keyRenewToken );
    if ( SUCCEEDED(hr) )
    {
        // activate the app
        hr = ActivateSdk( key );

        if ( SUCCEEDED(hr) )
        {
            WriteText( pLicenseCacheFile, key );
        }

        // the renew token has been issued, cache it
        if ( keyRenewToken[0] )
        {
            WriteText( pLicensetokenCacheFile, keyRenewToken );
        }
    }

    return hr;
}

HRESULT ReactivateSdk( __inout_ecount_z_opt(128) char *pRenewToken )
{
    char strToken[128];

    HRESULT hr;
    const WCHAR *pLicensetokenCacheFile = L"license-token.txt";
    if ( !pRenewToken )
    {
        hr = ReadText( pLicensetokenCacheFile, strToken, _countof(strToken) );
        if ( FAILED(hr) )
        {
            return hr;
        }

        pRenewToken = strToken;
    }

    char key[2048 + 1] = {0};
    hr = RenewLicense( key, _countof(key), pRenewToken );
    if ( SUCCEEDED(hr) )
    {
        hr = ActivateSdk( key );
		if ( SUCCEEDED(hr) )
		{
			return hr;
		}

        // store a new renew token
        hr = WriteText( pLicensetokenCacheFile, pRenewToken );
    }

    return hr;
}

HRESULT GetLicenseExpirationDaysCount( __out UINT *pDaysToExpire )
{
    UINT day, month, year;
    HRESULT hr = CapturingReality::RealityCaptureEngine::GetLicenseExpirationDate( &day, &month, &year );
    if ( FAILED(hr) )
    {
        return hr;
    }
    
	time_t currentNow;
	time( &currentNow );

    tm now;
    errno_t err = localtime_s( &now, &currentNow );
    if ( err )
    {
        return E_FAIL;
    }

	tm tmp = now; 
	tmp.tm_year = year - 1900;
	tmp.tm_mon  = month - 1;
	tmp.tm_mday = day;

    time_t licenseTime = mktime( &tmp );

    if ( licenseTime < currentNow )
    {
        return E_ACCESSDENIED;
    }

    *pDaysToExpire = (UINT)(licenseTime - currentNow) / (60*60*24);

    return S_OK;
}