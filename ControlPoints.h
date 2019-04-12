//
// Capturing Reality s.r.o.
//
// This file 'ControlPoints.h' is a part of the RC Engine Samples. You may use it freely.
//

#pragma once

#include "define.h"

///////////////////////////////////////////////////////////////////////////////
#ifdef _ENABLE_CR_BASE_CODE_

#define CONTROL_POINT_COUNT 6

///////////////////////////////////////////////////////////////////////////////
#else	// ndef

#define CONTROL_POINT_COUNT 2500

#endif	// _ENABLE_CR_BASE_CODE_
///////////////////////////////////////////////////////////////////////////////

using namespace CapturingReality;
using namespace CapturingReality::Sfm;
using namespace CapturingReality::CoordinateSystems;

//
// Note that this is a hard coded implementation of the IControlPoints editor
// specifically for this example. We do not provide a general implementation.
//

class ControlPoints : public IControlPoints
{
public:

///////////////////////////////////////////////////////////////////////////////
#ifdef _ENABLE_CR_BASE_CODE_

    static HRESULT CreateInstance( __in CapturingReality::Sfm::ICoordinateSystem *pCoordinateSystem, __deref_out IControlPoints **ppControlPoints );

    ControlPoints();

///////////////////////////////////////////////////////////////////////////////
#else	// ndef

	// @param	param_dir		オプションが格納されたディレクトリ名を渡します。
	// @param	file_name		画像ファイルのフルパスを渡します。先頭の１つ目で問題ありません。幅高さ解析用です。
	static HRESULT ControlPoints::CreateInstance(
		__in CapturingReality::Sfm::ICoordinateSystem *pCoordinateSystem,
		__deref_out IControlPoints **ppControlPoints,
		__in const wchar_t* param_dir,
		__in const wchar_t* image_full_path
		);

	ControlPoints(int image_width, int image_height);

#endif	// _ENABLE_CR_BASE_CODE_
///////////////////////////////////////////////////////////////////////////////

    ~ControlPoints();

    HRESULT Initialize( __in CapturingReality::Sfm::ICoordinateSystem *pCoordinateSystem );

    // IControlPoints

    HRESULT SetControlPoint( __in_z const wchar_t *pszPointId, __in CoordinateSystemPoint *pPosition, __in_opt ICoordinateSystem *pCoordinateSystem );

    UINT    GetPointsCount();

    HRESULT GetControlPoint( __in UINT pointIndex, __out ControlPoint **ppPoint );

    HRESULT GetControlPointCoordinateSystem( __in UINT pointIndex, __deref_out ICoordinateSystem **ppCoordinateSystem );

    HRESULT GetConstraintsCoordinateSystem( __in UINT constraintIndex, __deref_out ICoordinateSystem **ppCoordinateSystem );

    const wchar_t* GetControlPointName( __in UINT pointIndex );

    UINT    GetConstraintsCount();

    int     FindPoint( __in UINT id );

    ControlPointConstraint *GetConstraint( __in UINT index );

    HRESULT RecalculatePoint( __in UINT pointIndex );

    void    Lock();

    void    Unlock();

    // CComObject interface methods.

    STDMETHOD(QueryInterface)(REFIID riid, __deref_out_opt void **ppvObject);

    STDMETHOD_(ULONG, AddRef)();

    STDMETHOD_(ULONG, Release)();

private:

    CComPtr< ICoordinateSystem >    m_spCoordinateSystem;
    ControlPoint                    m_controlPoints[CONTROL_POINT_COUNT];
    SfmReconstructionControlPoint   m_tracks[CONTROL_POINT_COUNT];
    volatile LONG m_RefCount;

///////////////////////////////////////////////////////////////////////////////
#ifndef _ENABLE_CR_BASE_CODE_

	int imageWidth;
	int imageHeight;

#endif	// _ENABLE_CR_BASE_CODE_
///////////////////////////////////////////////////////////////////////////////

};

///////////////////////////////////////////////////////////////////////////////
#ifndef _ENABLE_CR_BASE_CODE_

bool loadGcpFiles(
		__in const wchar_t* inputparam_dir,
		__in const wchar_t* measure_file, 
		__in const wchar_t* gcp_file,
		__in const wchar_t* pointname_file);

#endif
///////////////////////////////////////////////////////////////////////////////
