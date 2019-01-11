//
// Capturing Reality s.r.o.
//
// This file 'MockupProgressReportCallback.h' is a part of the RC Engine Samples. You may use it freely.
//

#pragma once 

class MockupProgressReportCallback : public CapturingReality::IProgressReportCallback
{
public:

	MockupProgressReportCallback()
	{
	};

	~MockupProgressReportCallback()
	{
	};

	HRESULT Initialize()
	{
		return S_OK;
	};

	ULONG AddRef()
	{
		return 0;
	};

	HRESULT QueryInterface( REFIID riid, void **ppvObject )
	{
        return E_NOINTERFACE;
	};

	ULONG Release()
	{
		return 0;
	};

	void BeginSection( __in CapturingReality::ProgressAlgorithmId algId, __in float sectionFactor )
	{
		//Called by workers at the beginning of each work block
		//The sectionFactor specifies what part of the <0,1> interval of the parent section this section takes
		printf( "Begin section %i, sectionFactor %f\n", algId, sectionFactor );
		algId;
		sectionFactor;
	};

	HRESULT OnProgress( __in UINT position, __in UINT totalSteps )
	{
		//Called by workers to notify a caller about the current algorithm progress
		position;
		totalSteps;
		return S_OK;
	};

	HRESULT OnEvent( __in CapturingReality::ProgressEventId eventId, __in_opt void* reserved )
	{
		eventId;
		reserved;
		return S_OK;
	};

	HRESULT LogMessage( __in_z const wchar_t *pMessage )
	{
		pMessage;
		return S_OK;
	};

	bool IsAborted()
	{
		//Return true to abort the actual algorithm.
		return false;
	};

	void EndSection( __in HRESULT hr, __in_opt void* reserved )
	{
		//Called by workers at the end of each work block
		printf( "End section ( hr: %i ) \n", hr );
		hr;
		reserved;
	};

	HRESULT SetLastError( __in UINT code, __in_z_opt const WCHAR *pMessage, __in_z_opt const WCHAR *pErrorStack )
	{
		code;
		pMessage;
		pErrorStack;
		return S_OK;
	};

	HRESULT CreateWorkerProgressReport(__deref_out CapturingReality::IProgressReportCallback **ppWorkerReport)
	{
		//Return a new progress report for a newly created thread. As progress sections are hierarchical, each thread has to have its own worker, so 
		//section states do not mix. In this example, we return NULL as we do not want to track thread workers' states.
		*ppWorkerReport = NULL;
		return S_OK;
	};
};