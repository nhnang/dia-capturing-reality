#include "stdafx.h"
#include "param.h"
#include <string>
#include <cstdio>
#include <iostream>
#include <locale>
#include <vector>
#include <string>
#include <fstream>

using namespace std;

///////////////////////////////////////////////////////////////////////////////
// ndef
#ifndef _ENABLE_CR_BASE_CODE_

//////////////////////////////////////////////////////////////////////////////////////////////////////
// load Ortho GPS
bool loadOrthoGps(
	__in const wchar_t* input_dir,
	__in const wchar_t* ortho_file,
	CoordinateSystemPoint& centreGps,
	double& north,
	double& south,
	double& east,
	double& west
	)
{
	// 存在チェックを行います。
	if (!PathFileExists(input_dir))
	{
		wprintf(L"\n%s dir is not found.", input_dir);
		exit(-70);
	}
	// 扱いやすいwstr形式にします
	wstring dir_str(input_dir);
	wstring ortho_str(ortho_file);
	wstring ortho_path = dir_str + ortho_str;

	// 存在チェックを行います。
	if (!PathFileExists(ortho_path.c_str()))
	{
		wprintf(L"\nOrtho Input: %s file is not found.", ortho_path);
		exit(-71);
	}
	// １行分の仮バッファ
	wstring wline;
    wifstream stream(ortho_path);
    if(stream.fail())
	{
		wprintf(L"loadOrthoGps() failure open %s.", ortho_path);
		exit(-72);
	}

	///////////////////////////////////////////
	// １行ずつ読み出します
    if (!getline(stream, wline))
	{
		wprintf(L"loadOrthoGps() need centreGps.x");
		exit(-73);
	}
	centreGps.x = _wtof(wline.c_str());

	if (!getline(stream, wline))
	{
		wprintf(L"loadOrthoGps() need centreGps.y");
		exit(-74);
	}
	centreGps.y = _wtof(wline.c_str());
    if (!getline(stream, wline))
	{
		wprintf(L"loadOrthoGps() need centreGps.z");
		exit(-75);
	}
	centreGps.z = _wtof(wline.c_str());

	// NSEW 
	if (!getline(stream, wline))
	{
		wprintf(L"loadOrthoGps() need north.");
		exit(-75);
	}
	north = _wtof(wline.c_str());

	if (!getline(stream, wline))
	{
		wprintf(L"loadOrthoGps() need north.");
		exit(-76);
	}
	south = _wtof(wline.c_str());

	if (!getline(stream, wline))
	{
		wprintf(L"loadOrthoGps() need north.");
		exit(-77);
	}
	east = _wtof(wline.c_str());

	if (!getline(stream, wline))
	{
		wprintf(L"loadOrthoGps() need north.");
		exit(-78);
	}
	west = _wtof(wline.c_str());

	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// EPSGリストを読み込み、中身をワイド文字配列で返します。
int loadEpsgList(
	__in const wchar_t* input_dir,
	__in const wchar_t* epsglist_file,
	__out wchar_t** result,
	int epsg_count)
{
	// 存在チェックを行います。
	if (!PathFileExists(input_dir))
	{
		wprintf(L"\n%s dir is not found.", input_dir);
		exit(-41);
	}
	// 扱いやすいwstr形式にします
	wstring dir_str(input_dir);
	wstring epsg_str(epsglist_file);
	wstring epsg_path = dir_str + epsg_str;

	// 存在チェックを行います。
	if (!PathFileExists(epsg_path.c_str()))
	{
		wprintf(L"\n%s file is not found.", epsg_path);
		exit(-42);
	}

	// １行分の仮バッファ
	wstring wline;
    wifstream stream(epsg_path);
    if(stream.fail())
	{
		wprintf(L"loadEpsgList() failure open %s.", epsg_path);
		exit(-43);
	}
	// １行ずつ読み出します
	int index = 0;
    while(getline(stream, wline))
	{
		if (index > epsg_count)
		{
			wprintf(L"loadEpsgList() too much lines.");
			OutputDebugString(L"10行以上存在しているため残りは無視します。");
			break;
		}
		// 恒久的なメモリを確保します
		size_t len = wcslen(wline.c_str()) + 1;
		wchar_t* buff = (wchar_t*)malloc((wcslen(wline.c_str()) + 1) * sizeof(wchar_t));
		wcscpy_s(buff, wcslen(wline.c_str()) + 1, wline.c_str());
		// 戻り値に渡します。
		result[index] = buff;
		index ++;
	}
	return index;
}

///////////////////////////////////////////////////////////////////////////////
// common func
void HrToStr(HRESULT hr) {
    LPVOID string;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                  FORMAT_MESSAGE_FROM_SYSTEM,
                  NULL,
                  hr,
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  (LPTSTR)&string,
                  0,
                  NULL);
    if (string != NULL)
	{
        OutputDebugString((LPCWSTR)string);
	}

    LocalFree(string);
}


#endif
///////////////////////////////////////////////////////////////////////////////
