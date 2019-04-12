#pragma once
#include "stdafx.h"
#include "define.h"

using namespace CapturingReality;

bool loadOrthoGps(
	__in const wchar_t* input_dir,
	__in const wchar_t* ortho_file,
	CoordinateSystemPoint& centreGps,
	double& north,
	double& south,
	double& east,
	double& west
	);

int loadEpsgList(
	__in const wchar_t* input_dir,
	__in const wchar_t* epsglist_file,
	__out wchar_t** result,
	int epsg_count);

void HrToStr(HRESULT hr);
