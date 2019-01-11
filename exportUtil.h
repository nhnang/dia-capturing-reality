
#pragma once
#include "stdafx.h"
#include "define.h"

void CreateExportParams( __out CapturingReality::ImportExport::XmlModelExportParams *pSettings );

HRESULT ExportReconstructionToPly(
	__in_z const WCHAR *pFolderLocation,
	__in_z const WCHAR *pFileName,
	__in CapturingReality::Sfm::ISfmReconstruction *pRec );

HRESULT ExportReconstructionToXYZ(
	__in_z const WCHAR *pOutputDir,
	__in_z const WCHAR *pXyzFileName,
	__in CapturingReality::Sfm::ISfmReconstruction *pRec );

HRESULT ExportModelXYZ( __in_z const WCHAR *pFileName, __in CapturingReality::Sfm::ISfmReconstruction *pSfmReconstruction, __in CapturingReality::Mvs::IMvsModel *pModel );
