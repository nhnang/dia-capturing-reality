
#pragma once
#include "stdafx.h"
#include "define.h"

// from CR support 

HRESULT PrintCalculatedCameraPositionsInGps( __in ISfmReconstruction *pReconstruction, __in IStructureFromMotion *pSfm );

HRESULT PrintReconstructionStats( __in IStructureFromMotion *pSfm );

