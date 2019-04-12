//
// Capturing Reality s.r.o.
//
// This file 'VariousHelpers.cpp' is a part of the RC Engine Samples. You may use it freely.
//

#include "stdafx.h"
#include "SamplesCommonLib.h"

HRESULT OpenWebBrowser( __in_z const WCHAR *pUrl )
{
	SHELLEXECUTEINFO ShExecInfo = {0};

	ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
	ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	ShExecInfo.hwnd = NULL;
	ShExecInfo.lpVerb = L"open";
	ShExecInfo.lpFile = pUrl;
	ShExecInfo.lpParameters = L"";   
	ShExecInfo.lpDirectory = NULL;
	ShExecInfo.nShow = SW_SHOW;
	ShExecInfo.hInstApp = NULL; 
	if (! ShellExecuteEx(&ShExecInfo) )
	{
		return E_UNEXPECTED;
	}

	// modern browsers with tabs will not wait
	WaitForSingleObject( ShExecInfo.hProcess, INFINITE );
	return S_OK;
}

HRESULT OpenFile( __in_z const WCHAR *pFileName )
{
	SHELLEXECUTEINFO ShExecInfo = {0};

	ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
	ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	ShExecInfo.hwnd = NULL;
	ShExecInfo.lpVerb = L"open";
	ShExecInfo.lpFile = pFileName;
	ShExecInfo.lpParameters = L"";   
	ShExecInfo.lpDirectory = NULL;
	ShExecInfo.nShow = SW_SHOW;
	ShExecInfo.hInstApp = NULL; 
	if (! ShellExecuteEx( &ShExecInfo ) )
	{
		return E_UNEXPECTED;
	}

	return S_OK;
}

HRESULT ReadText( __in_z const WCHAR *pFileName, __out_ecount_z(maxTextSize) char *pText, __in size_t maxTextSize )
{
    if ( PathFileExists(pFileName) )
    {
        FILE *file;
        errno_t err = _wfopen_s( &file, pFileName, L"r" ); 
        if ( !err )
        {
            char *strRes = fgets( pText, (int)maxTextSize, file );
            fclose( file );

            return strRes ? S_OK : E_FAIL;
        }
    }

    return E_FAIL;
}

HRESULT WriteText( __in_z const WCHAR *pFileName, __out_ecount_z(maxKeySize) const char *pText )
{
    // store the license for later
    FILE *file;
    errno_t err = _wfopen_s( &file, pFileName, L"w" ); 
    if ( !err )
    {
        int res = fputs( pText, file );
        fclose( file );

        return res > 0 ? S_OK : E_FAIL;
    }

    return E_FAIL;
}

void LogMessageToFile( __in_z const wchar_t *logFileName, const char *format, ... )
{
    va_list args;
    va_start (args, format);

    FILE *file;
    errno_t err = _wfopen_s( &file, logFileName, L"a" );

	if ( !err && file )
	{
		vfprintf(file, format, args);

		fclose(file);
	}

	vprintf(format, args);

    va_end (args);
}