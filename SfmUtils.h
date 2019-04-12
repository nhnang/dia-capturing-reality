//
// RealityCapture s.r.o.
//
// This file 'SfmUtils.h' is part of an RC Engine Samples.
//

#pragma once

#include "define.h"

enum MyResourceType
{
    MRT_COMPONENT = 0xF000,
};

HRESULT CreateOrthoExampleSfm(
///////////////////////////////////////////////////////////////////////////////
// ndef
#ifndef _ENABLE_CR_BASE_CODE_
	__in		const wchar_t* inputimage_dir,
	__in		const wchar_t* inputparam_dir,
#endif // _ENABLE_CR_BASE_CODE_
///////////////////////////////////////////////////////////////////////////////
	__in        CapturingReality::IResourceCache*                       pCache,
    __in        CapturingReality::IConfig*                              pConfig,
    __in        CapturingReality::CoordinateSystems::ICoordinateSystem* spCoordSys,
    __deref_out CapturingReality::Sfm::IStructureFromMotion**           ppSfm
);

HRESULT CreateReconstruction(
    __in        CapturingReality::Sfm::IStructureFromMotion*    pSfm,
    __deref_out CapturingReality::Sfm::ISfmReconstruction**     ppReconstruction
);

///////////////////////////////////////////////////////////////////////////////
// ndef
#ifndef _ENABLE_CR_BASE_CODE_

int makeFileList(
	__in const wchar_t* input,
	__out wchar_t** result);

void clearFileList(__in wchar_t** result, __in int count);

int loadImageList(
	__in const wchar_t* input_dir,
	__in const wchar_t* imagelist_file,
	__out wchar_t** result);

#endif // _ENABLE_CR_BASE_CODE_
///////////////////////////////////////////////////////////////////////////////
