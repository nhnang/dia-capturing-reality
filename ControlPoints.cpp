//
// Capturing Reality s.r.o.
//
// This file 'ControlPoints.cpp' is a part of the RC Engine Samples. You may use it freely.
//

#include "stdafx.h"
#include "ControlPoints.h"

///////////////////////////////////////////////////////////////////////////////
// ndef
#ifndef _ENABLE_CR_BASE_CODE_

#include "atlimage.h"
#include "SplitString.h"
#include <vector>
#include <cstdio>
#include <iostream>
#include <locale>
#include <vector>
#include <string>
#include <fstream>

using namespace V4;

#endif	// _ENABLE_CR_BASE_CODE_

struct MyMeasurementType
{
    UINT imageIndex;
    UINT controlPointIndex;
    double x;
    double y;
};

///////////////////////////////////////////////////////////////////////////////
#ifdef _ENABLE_CR_BASE_CODE_

#define MEASUREMENT_COUNT 95

MyMeasurementType g_Measurements[MEASUREMENT_COUNT] =
{
{ 13, 0, 164.36, 1104.80  },
{ 14, 0, 775.00, 186.25   },
{ 29, 0, 504.43, 986.39   },
{ 30, 0, 331.18, 197.07   },
{ 42, 0, 783.27, 280.60   },
{ 15, 0, 637.02, 486.19   },
{ 16, 0, 650.24, 824.06   },
{ 17, 0, 650.40, 1153.62  },
{ 31, 0, 284.30, 498.09   },
{ 28, 0, 327.40, 691.30   },
{ 32, 0, 271.58, 969.09   },
{ 27, 0, 408.60, 389.76   },
{ 26, 0, 467.56, 115.39   },
{ 63, 1, 416.28, 1094.19  },
{ 48, 1, 753.55, 1161.09  },
{ 47, 1, 770.35, 833.50   },
{ 56, 1, 229.49, 340.54   },
{ 55, 1, 175.96, 83.98    },
{ 71, 1, 631.64, 320.37   },
{ 89, 1, 867.35, 1010.77  },
{ 75, 1, 232.70, 12.24    },
{ 74, 1, 767.13, 1043.07  },
{ 60, 1, 551.43, 103.49   },
{ 76, 1, 137.67, 413.67   },
{ 73, 1, 663.29, 847.55   },
{ 59, 1, 335.61, 1102.41  },
{ 45, 1, 805.63, 187.73   },
{ 58, 1, 337.23, 903.44   },
{ 43, 1, 105.05, 886.28   },
{ 46, 1, 891.44, 494.24   },
{ 70, 1, 598.24, 11.30    },
{ 57, 1, 297.41, 575.67   },
{ 62, 1, 419.54, 774.41   },
{ 78, 1, 27.75, 1042.81   },
{ 72, 1, 628.53, 536.52   },
{ 61, 1, 472.38, 472.44   },
{ 17, 2, 574.75, 292.48   },
{ 32, 2, 70.54, 72.37     },
{ 27, 2, 817.37, 1170.76  },
{ 26, 2, 749.53, 951.37   },
{ 02, 2, 891.66, 111.52   },
{ 04, 2, 809.60, 845.47   },
{ 9, 2, 213.70, 691.10    },
{ 8, 2, 207.78, 363.02    },
{ 3, 2, 815.41, 465.43    },
{ 10, 2, 271.14, 939.97   },
{ 35, 2, 57.21, 1184.82   },
{ 19, 2, 416.89, 1025.35  },
{ 18, 2, 414.52, 691.69   },
{ 34, 2, 89.68, 848.90    },
{ 24, 2, 592.51, 237.70   },
{ 25, 2, 656.18, 572.78   },
{ 41, 3, 442.18, 1147.39  },
{ 40, 3, 100.55, 931.18   },
{ 39, 3, 148.04, 659.80   },
{ 49, 3, 587.23, 597.30   },
{ 50, 3, 517.72, 860.82   },
{ 53, 3, 555.92, 294.68   },
{ 54, 3, 544.32, 595.68   },
{ 65, 3, 255.86, 839.22   },
{ 68, 3, 874.39, 301.04   },
{ 63, 3, 231.56, 250.44   },
{ 48, 3, 538.80, 319.48   },
{ 47, 3, 523.14, 32.95    },
{ 56, 3, 614.10, 1092.00  },
{ 55, 3, 554.02, 831.92   },
{ 64, 3, 114.73, 518.43   },
{ 37, 3, 135.91, 135.63   },
{ 51, 3, 441.27, 1155.62  },
{ 52, 3, 380.88, 23.32    },
{ 67, 3, 818.96, 2.25     },
{ 66, 3, 160.23, 1120.10  },
{ 36, 3, 887.96, 1106.40  },
{ 39, 4, 759.68, 1080.47  },
{ 50, 4, 35.87, 304.62    },
{ 37, 4, 684.45, 504.29   },
{ 51, 4, 31.41, 571.90    },
{ 36, 4, 431.11, 618.93   },
{ 35, 4, 505.16, 266.81   },
{ 34, 4, 629.10, 24.07    },
{ 24, 4, 215.19, 1130.83  },
{ 22, 4, 218.07, 583.05   },
{ 23, 4, 245.55, 872.15   },
{ 38, 4, 505.15, 772.47   },
{ 50, 5, 830.58, 282.30   },
{ 53, 5, 339.27, 895.27   },
{ 65, 5, 579.64, 262.99   },
{ 68, 5, 659.95, 925.06   },
{ 51, 5, 835.48, 627.40   },
{ 52, 5, 197.30, 628.31   },
{ 67, 5, 652.31, 617.91   },
{ 66, 5, 436.00, 531.86   },
{ 69, 5, 774.55, 1158.24  },
{ 80, 5, 113.71, 243.26   },
{ 81, 5, 34.25, 547.82    }
};

double g_ControlPoints[CONTROL_POINT_COUNT][3] =
{
    { 140462.593, 7068675.306999999, 103.1570000005886 },
    { 140596.592, 7068701.131999999, 91.92700000014156 },
    { 140428.317, 7068776.571000001, 108.5050000008196 },
    { 140555.647, 7068818.100999999, 85.19199999980629 },
    { 140483.575, 7068887.411000001, 99.56500000040978 },
    { 140591.647, 7068906.475000000, 89.66999999899417 },
};

const wchar_t *g_ControlPointNames[CONTROL_POINT_COUNT] = { L"GCP0", L"GCP1", L"GCP2", L"GCP3", L"GCP4", L"GCP5" };

GUID gControlPointGuids[CONTROL_POINT_COUNT] =
{
    { 0x281cf885, 0xbeeb, 0x46a3, { 0xad, 0x29, 0xbd, 0xc0, 0x59, 0xba, 0xe8, 0xa7 } }, // {281CF885-BEEB-46A3-AD29-BDC059BAE8A7}
    { 0x28100cd6, 0x63f7, 0x4a97, { 0xa3, 0xff, 0x7d, 0xb8, 0x24, 0x35, 0xbe, 0xbf } }, // {28100CD6-63F7-4A97-A3FF-7DB82435BEBF}
    { 0xa4c94b53, 0xc1b7, 0x4358, { 0xbd, 0x32, 0xfc, 0xbc, 0xa1, 0xa2, 0xf7, 0x6 } }, // {A4C94B53-C1B7-4358-BD32-FCBCA1A2F706}
    { 0xeb796df6, 0xafd5, 0x41ab, { 0xa0, 0x70, 0xb9, 0xb4, 0x5, 0x5, 0x3f, 0x93 } }, // {EB796DF6-AFD5-41AB-A070-B9B405053F93}
    { 0xe9b0895a, 0xd47d, 0x4f7e, { 0x88, 0xbe, 0x61, 0x7c, 0xe1, 0x8d, 0x5b, 0x74 } }, // {E9B0895A-D47D-4F7E-88BE-617CE18D5B74}
    { 0x2266345d, 0x764f, 0x48e8, { 0xae, 0xec, 0xc4, 0x1b, 0xd1, 0x56, 0x2c, 0x7e } } // {2266345D-764F-48E8-AEEC-C41BD1562C7E}
};

///////////////////////////////////////////////////////////////////////////////
#else	// ndef

// calc center
//CoordinateSystemPoint g_coord_center;
CoordinateSystemPoint centre3016;
double whd[3];

UINT measure_count = 0;
UINT gcp_count = 0;
USHORT pointname_count = 0;
UINT guid_count = 0;

#define MEASUREMENT_COUNT 2500
MyMeasurementType g_Measurements[MEASUREMENT_COUNT];
double g_ControlPoints[CONTROL_POINT_COUNT][3];
wchar_t *g_ControlPointNames[CONTROL_POINT_COUNT];
GUID gControlPointGuids[CONTROL_POINT_COUNT];

using namespace std;

// PNGまたはJPGからWH情報を取り出します。
// 
bool getPixelSize(
	__in const wchar_t* file_path,
	__out int* o_width,
	__out int* o_height)
{
	wstring full_path(file_path);
	CImage* img = new CImage();
	HRESULT hr = img->Load(full_path.c_str());
	if (!SUCCEEDED(hr))
	{
        wprintf(L"Failure analyze jpg/png format.\n");
		OutputDebugString(L"Failure analyze jpg/png format.\n");
		return false;
	}
	*o_width = img->GetWidth();
	*o_height = img->GetHeight();
	return true;
}

//////////////////////////////////////////////////////////////////
// 文字列から拡張子を抜き出します
wstring GetExtension(const wstring &path)
{
    wstring ext;
    size_t pos1 = path.rfind('.');
    if(pos1 != wstring::npos){
        ext = path.substr(pos1+1, path.size()-pos1);
        wstring::iterator itr = ext.begin();
        while(itr != ext.end()){
            *itr = tolower(*itr);
            itr++;
        }
        itr = ext.end()-1;
        while(itr != ext.begin()){    // パスの最後に\0やスペースがあったときの対策
            if(*itr == 0 || *itr == 32){
                ext.erase(itr--);
            }
            else{
                itr--;
            }
        }
    }
    return ext;
}

//////////////////////////////////////////////////////////////////
bool loadGcpFiles(
	__in const wchar_t* inputparam_dir,
	__in const wchar_t* measure_file, 
	__in const wchar_t* gcp_file,
	__in const wchar_t* pointname_file)
{
	//////////////////////////////////////////////////////////////////////////////////////////////////
	// Step 1

	// 引数チェックを行います
	if ((inputparam_dir == nullptr) || (measure_file == nullptr) || (gcp_file == nullptr) || (pointname_file == nullptr))
	{
		OutputDebugString(L"loadGcpFile() param is null.");
		wprintf(L"loadGcpFile() param is null.");
		exit(-100);
	}

	// 存在チェックを行います。
	if (!PathFileExists(inputparam_dir))
	{
		OutputDebugString(L"input dir is not found.");
		wprintf(L"\n%s dir is not found.", inputparam_dir);
		exit(-101);
	}
	// 扱いやすいwstr形式にします
	wstring dir_str(inputparam_dir);
	wstring measure_str(measure_file);
	measure_str = dir_str +measure_str;
	wstring gcp_str(gcp_file);
	gcp_str = dir_str + gcp_str;
	wstring pointname_str(pointname_file);
	pointname_str = dir_str + pointname_str;

	if (!PathFileExists(measure_str.c_str()))
	{
		wprintf(L"loadGcpFile() %s is not found.", measure_str);
		exit(-102);
	}
	if (!PathFileExists(gcp_str.c_str()))
	{
		wprintf(L"loadGcpFile() %s is not found.", gcp_str);
		exit(-103);
	}
	if (!PathFileExists(pointname_str.c_str()))
	{
		wprintf(L"loadGcpFile() %s is not found.", pointname_str);
		exit(-104);
	}

	// 共通で使用するテンポラリ文字列バッファ１行分
	wstring wline;

	//////////////////////////////////////////////////////////////////////////////////////////////////
	// Step 2

	// 順番変更：クラッシュ対策のためにGCP一覧を先に読み込みます。
	// gcpポイントfileを読み出します
	gcp_count = 0;
	wifstream gcp_stream(gcp_str);
    if(gcp_stream.fail())
	{
		OutputDebugString(L"loadGcpFile() failure open \n");
		wprintf(L"loadGcpFile() failure open %s.", gcp_str);
		exit(-105);
	}

	// １行ずつ読み出します
    while(getline(gcp_stream, wline))
	{
		// からっぽ改行判定の暫定版。正確な判定処理を入れるまでの繋ぎ
		if (2 > wline.length())
		{
			OutputDebugString(L"空白をスキップします。\n");
			continue;
		}

		// 余分な空白を全て取り除きます
		for(size_t c = wline.find_first_of(L" "); c != string::npos; c = c = wline.find_first_of(L" "))
		{
			wline.erase(c,1);
		}
		// 取得した１行を分解します。
		SplitString split;
		USHORT count = split.Split(wline.c_str(), L',');
		if (3 > count)
		{
			wprintf(L"loadGcpFile() failure analyze %s.", wline);
			wprintf(L"measurement file line needs, double, double, double  ex) 140462.593, 7068675.306999999, 103.1570000005886");
			exit(-106);
		}
		// 数値に変換して構造体に入れ込みます
		g_ControlPoints[gcp_count][0] = _wtof(split[0]);
		g_ControlPoints[gcp_count][1] = _wtof(split[1]);
		g_ControlPoints[gcp_count][2] = _wtof(split[2]);

		gcp_count ++;
		// 暫定予防措置
		if (gcp_count >= 2500)
		{
			wprintf(L"gcp buffer overflow.");
			break;
		}
		// 分割クラスはクリアします
		split.clear();
	}
	wprintf(L"Gcp count = %d\n", gcp_count);

	//////////////////////////////////////////////////////////////////////////////////////////////////
	// Step 3

	// measure fileを読み出します
	measure_count = 0;

    wifstream stream(measure_str);
    if(stream.fail())
	{
		wprintf(L"loadGcpFile() failure open %s.", measure_str);
		exit(-107);
	}
	// １行ずつ読み出します
    while(getline(stream, wline))
	{
		if (2 > wline.length())
		{
			OutputDebugString(L"空白をスキップします。\n");
			continue;
		}
		// 余分な空白を全て取り除きます
		for(size_t c = wline.find_first_of(L" "); c != string::npos; c = c = wline.find_first_of(L" "))
		{
			wline.erase(c,1);
		}

		// 取得した１行を分解します。
		SplitString split;
		USHORT count = split.Split(wline.c_str(), L',');
		if (4 > count)
		{
			wprintf(L"loadGcpFile() failure analyze %s.", wline);
			wprintf(L"measurement file line needs, UINT imageIndex,UINT controlPointIndex,double x,double y ex) 13, 0, 164.36, 1104.80");
			OutputDebugString(L"Measurementは４つの項目で構成される必要があります。");
			exit(-108);
		}
		// 数値に変換して構造体に入れ込みます
		g_Measurements[measure_count].imageIndex =_wtoi(split[0]);
		g_Measurements[measure_count].controlPointIndex =_wtoi(split[1]);
		g_Measurements[measure_count].x = _wtof(split[2]);
		g_Measurements[measure_count].y = _wtof(split[3]);

		wprintf(L"index=%d\n", measure_count);
		wprintf(L"imageIndex=%d\n", g_Measurements[measure_count].imageIndex);
		wprintf(L"controlPointIndex=%d\n", g_Measurements[measure_count].controlPointIndex);
		wprintf(L"x=%f\n", g_Measurements[measure_count].x);
		wprintf(L"y=%f\n", g_Measurements[measure_count].y);

		/*
		// クラッシュ防止策
		if (pp_Measurements[measure_count]->controlPointIndex > (gcp_count - 1))
		{
			wprintf(L"loadGcpFile() The index of Measurement exceeds the total number of GCPs. %s", wline.c_str());
			OutputDebugString(L"MeasurementのインデックスがGCP総数を超えています。");
			exit(-109);
		}
		*/
		measure_count ++;
		// 暫定予防措置
		if (measure_count >= 2500)
		{
			wprintf(L"measure buffer overflow.");
			break;
		}
		// 分割クラスはクリアします
		split.clear();
	}
	wprintf(L"Measurement count = %d\n", measure_count);

	//////////////////////////////////////////////////////////////////////////////////////////////////
	// Step 4

	pointname_count = 0;
	wifstream pointname_stream(pointname_str);
    if(pointname_stream.fail())
	{
		wprintf(L"loadGcpFile() failure open %s.", pointname_str);
		exit(-110);
	}
	// このファイルは１行のみです。
    if (!getline(pointname_stream, wline))
	{
		wprintf(L"loadGcpFile() failure load %s.", pointname_str);
		exit(-111);
	}
	// 余分な空白を全て取り除きます
	for(size_t c = wline.find_first_of(L" "); c != string::npos; c = c = wline.find_first_of(L" "))
	{
		wline.erase(c,1);
	}

	// 取得した１行を分解します。
	SplitString split;
	pointname_count = split.Split(wline.c_str(), L',');
	for (int i = 0; i < pointname_count; i ++)
	{
		// 全ての文字をコピーしていきます。
		size_t size = wcslen(split[i]) + 1;
		g_ControlPointNames[i] = new wchar_t[size];
		wcscpy_s(g_ControlPointNames[i], size, split[i]);
	}
	// 分割クラスはクリアします
	split.clear();

	//////////////////////////////////////////////////////////////////////////////////////////////////
	// Step 5

	// GUID構造体の配列を生成します。
	guid_count = 0;

	// これはCOMコンポーネントから生成します
	CoInitialize(NULL);
	for (int i = 0; i < pointname_count; i ++)
	{
		HRESULT result = CoCreateGuid(&gControlPointGuids[guid_count]);
		if (result != S_OK)
		{
			wprintf(L"loadGcpFile() Create Guid failure. result=%d", result);
			exit(-112);
		}
		guid_count ++;
	}

	return true;
}

#endif	// _ENABLE_CR_BASE_CODE
///////////////////////////////////////////////////////////////////////////////

static void FillControlPointTrackWithDefaultValues( __inout SfmReconstructionControlPoint *pTrack )
{
    if ( !pTrack )
    {
        return;
    }

    pTrack->X.x = pTrack->X.y = pTrack->X.z = 0.0;
    pTrack->X.w = 1.0;
    pTrack->color = 0;
    pTrack->flag = 0;
    pTrack->measurementsCount = 0;
    pTrack->pointId = (UINT)-1;
}

static void FillControlPointWithDefaultValues( __inout ControlPoint *pPoint )
{
    if ( !pPoint )
    {
        return;
    }

    FillControlPointTrackWithDefaultValues( pPoint->pTrack );

    pPoint->weight[ 0 ] = 10.; pPoint->weight[ 1 ] = 0.0; pPoint->weight[ 2 ] = 0.0;
    pPoint->weight[ 3 ] = 0.0; pPoint->weight[ 4 ] = 10.; pPoint->weight[ 5 ] = 0.0;
    pPoint->weight[ 6 ] = 0.0; pPoint->weight[ 7 ] = 0.0; pPoint->weight[ 8 ] = 10.;

    pPoint->id = (UINT)-1;
    pPoint->calibrationType = CPCT_2D;
    pPoint->csParams[ 0 ] = pPoint->csParams[ 1 ] = pPoint->csParams[ 2 ] = 0.0;
    pPoint->csParams[ 3 ] = pPoint->csParams[ 4 ] = pPoint->csParams[ 5 ] = 0.0;

    CoCreateGuid( &pPoint->identity );
}

///////////////////////////////////////////////////////////////////////////////
#ifdef _ENABLE_CR_BASE_CODE_

HRESULT ControlPoints::CreateInstance( __in CapturingReality::Sfm::ICoordinateSystem *pCoordinateSystem, __deref_out IControlPoints **ppControlPoints )
{
    if ( !ppControlPoints )
    {
        return E_POINTER;
    }

    ControlPoints *pObject = new ControlPoints();
    if ( !pObject )
    {
        return E_OUTOFMEMORY;
    }

    HRESULT hr = pObject->Initialize( pCoordinateSystem );
    if ( SUCCEEDED(hr) )
    {
        pObject->AddRef();
        *ppControlPoints = pObject;
    }
    else
    {
        delete pObject;
    }

    return hr;
}

///////////////////////////////////////////////////////////////////////////////
// ndef
#else

HRESULT ControlPoints::CreateInstance(
	__in CapturingReality::Sfm::ICoordinateSystem *pCoordinateSystem,
	__deref_out IControlPoints **ppControlPoints,
	__in const wchar_t* param_dir,
	__in const wchar_t* image_full_path
	)
{
    if ( !ppControlPoints )
    {
        return E_POINTER;
    }

	// ファイルから画像の幅と高さを抽出します。
	// pngまたはjpgにのみ対応しています。
	wstring name(image_full_path);
	wstring ext = GetExtension(name);
	int image_width = 0;
	int image_height = 0;
	if ((ext.compare(L"png") == 0) || (ext.compare(L"jpg") == 0) || (ext.compare(L"jpeg") == 0))
	{
		if (!getPixelSize(image_full_path, &image_width, &image_height))
		{
			wprintf(L"Input Image header format parse error.");
			return S_FALSE;
		}
	}
	else
	{
		wprintf(L"Fle extention must be png/jpg/jpeg. ext=%s", ext);
		return S_FALSE;
	}

	ControlPoints *pObject = new ControlPoints(image_width, image_height);
    if ( !pObject )
    {
        return E_OUTOFMEMORY;
    }

	// インスタンス生成に成功したら、初期化処理の前にGCPファイル群を読み込みます
	// TODO:
	// ファイル名の柔軟指定化
	bool is_success =
		loadGcpFiles(
			param_dir,
			L"MyMeasurementType.txt",
			L"ControlPoints.txt",
			L"ControlPointNames.txt");

	if (!is_success)
	{
		wprintf(L"GCPロードに失敗しました。");
		exit(-113);
	}

	// INFO:
	// 汚いが、構造を壊せないためここで平均値の実装
	double whd_max[3] = {0.0f,0.0f,0.0f};
	double whd_min[3] = {0.0f,0.0f,0.0f};

	bool is_first = true;
	for (int i = 0; i < gcp_count; i ++)
	{
		double* tmp = g_ControlPoints[i];
		if (is_first)
		{
			for (int j = 0; j < 3; j ++)
			{
				whd_max[j] = tmp[j];
				whd_min[j] = tmp[j];
			}
			is_first = false;
		}
		for (int j = 0; j < 3; j ++)
		{
			whd_max[j] = max(whd_max[j], tmp[j]); 
			whd_min[j] = min(whd_min[j], tmp[j]); 
		}
	}
	// calc the center point
	centre3016.x = (whd_max[0] - whd_min[0]) / 2 + whd_min[0];
	centre3016.y = (whd_max[1] - whd_min[1]) / 2 + whd_min[1];
	centre3016.z = (whd_max[2] - whd_min[2]) / 2 + whd_min[2];

	printf("avg_x = %f\n", centre3016.x);
	printf("avg_y = %f\n", centre3016.y);
	printf("avg_z = %f\n", centre3016.z);

	HRESULT hr = pObject->Initialize( pCoordinateSystem );
    if ( SUCCEEDED(hr) )
    {
        pObject->AddRef();
        *ppControlPoints = pObject;
    }
    else
    {
        delete pObject;
    }

    return hr;
}

#endif	// _ENABLE_CR_BASE_CODE_
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
#ifdef _ENABLE_CR_BASE_CODE_

ControlPoints::ControlPoints()
{
    m_RefCount = 0;
}

///////////////////////////////////////////////////////////////////////////////
// ndef
#else

ControlPoints::ControlPoints(int image_width, int image_height)
	: imageWidth(image_width),
	imageHeight(image_height)
{
    m_RefCount = 0;
}
#endif	// _ENABLE_CR_BASE_CODE_
///////////////////////////////////////////////////////////////////////////////

ControlPoints::~ControlPoints()
{
///////////////////////////////////////////////////////////////////////////////
// ndef
#ifndef _ENABLE_CR_BASE_CODE_
	for (int i = 0; i < pointname_count; i ++)
	{
		delete g_ControlPointNames[i];
	}
#endif	// _ENABLE_CR_BASE_CODE_
///////////////////////////////////////////////////////////////////////////////
	_ASSERT( m_RefCount == 0 );
}

HRESULT ControlPoints::Initialize( __in CapturingReality::Sfm::ICoordinateSystem *pCoordinateSystem )
{
    m_spCoordinateSystem = pCoordinateSystem;

///////////////////////////////////////////////////////////////////////////////
#ifdef _ENABLE_CR_BASE_CODE_
	for ( UINT controlPointIndex = 0; controlPointIndex < CONTROL_POINT_COUNT; controlPointIndex++ )
///////////////////////////////////////////////////////////////////////////////
// ndef
#else
	for ( UINT controlPointIndex = 0; controlPointIndex < gcp_count; controlPointIndex++ )
#endif	// _ENABLE_CR_BASE_CODE_
///////////////////////////////////////////////////////////////////////////////
    {
        ControlPoint *pCp = &m_controlPoints[controlPointIndex];

        pCp->pTrack = &m_tracks[controlPointIndex];

        FillControlPointWithDefaultValues( pCp );

        pCp->calibrationType = ControlPointCalibrationType::CPCT_3D;
        pCp->csParams[0] = g_ControlPoints[controlPointIndex][0];
        pCp->csParams[1] = g_ControlPoints[controlPointIndex][1];
        pCp->csParams[2] = g_ControlPoints[controlPointIndex][2];
        pCp->id = controlPointIndex;
        pCp->identity = gControlPointGuids[controlPointIndex];
        pCp->pTrack->pointId = controlPointIndex;
        pCp->pTrack->flag = TrackGlobalPointFlag::fActive;
        pCp->pTrack->color = 0;

        pCp->pTrack->measurementsCount = 0;
		wprintf(L"csParams[0] %d\n", pCp->csParams[0]);
		wprintf(L"csParams[1] %d\n", pCp->csParams[1]);
		wprintf(L"csParams[2] %d\n", pCp->csParams[2]);

		wprintf(L"id %d\n", pCp->id);
		wprintf(L"pTrack->pointId %d\n", controlPointIndex);
    }

    MyMeasurementType *pMeasurement = g_Measurements;
///////////////////////////////////////////////////////////////////////////////
#ifdef _ENABLE_CR_BASE_CODE_
    for ( UINT gMIndex = 0; gMIndex < MEASUREMENT_COUNT; gMIndex++, pMeasurement++ )
///////////////////////////////////////////////////////////////////////////////
// ndef
#else
	for ( UINT gMIndex = 0; gMIndex < measure_count; gMIndex++, pMeasurement++ )
#endif	// _ENABLE_CR_BASE_CODE_
///////////////////////////////////////////////////////////////////////////////
	{
        ControlPoint *pCp = &m_controlPoints[ pMeasurement->controlPointIndex ];

        UINT mid = pCp->pTrack->measurementsCount;
        pCp->pTrack->measurements[mid].feature  = pMeasurement->controlPointIndex;
        pCp->pTrack->measurements[mid].image    = pMeasurement->imageIndex;

		wprintf(L"mid=%d\n", mid);
		wprintf(L"feature[mid] %d\n", pMeasurement->controlPointIndex);
		wprintf(L"image[mid] %d\n", pMeasurement->imageIndex);

        // We know the dimensions of the input images in this example.

///////////////////////////////////////////////////////////////////////////////
#ifdef _ENABLE_CR_BASE_CODE_
        UINT imageWidth  = 900;
        UINT imageHeight = 1200;
#endif	// _ENABLE_CR_BASE_CODE_
///////////////////////////////////////////////////////////////////////////////

        float scale = 1.0f / ( (float)max(imageWidth, imageHeight) );
        pCp->pTrack->measurements[mid].x = ( (float)pMeasurement->x - (float)(imageWidth)  * 0.5f ) * scale;
        pCp->pTrack->measurements[mid].y = ( (float)pMeasurement->y - (float)(imageHeight) * 0.5f ) * scale;
        pCp->pTrack->measurementsCount++;

		wprintf(L"x=%f\n", pCp->pTrack->measurements[mid].x);
		wprintf(L"y=%f\n", pCp->pTrack->measurements[mid].y);
	}

    return S_OK;
}

HRESULT ControlPoints::SetControlPoint( __in_z const wchar_t *pszPointId, __in CoordinateSystemPoint *pPosition, __in_opt ICoordinateSystem *pCoordinateSystem )
{
    pszPointId;
    pPosition;
    pCoordinateSystem;
    return E_NOTIMPL;
}

UINT ControlPoints::GetPointsCount()
{
///////////////////////////////////////////////////////////////////////////////
#ifdef _ENABLE_CR_BASE_CODE_
    return CONTROL_POINT_COUNT;
///////////////////////////////////////////////////////////////////////////////
// ndef
#else
    return gcp_count;
#endif	// _ENABLE_CR_BASE_CODE_
///////////////////////////////////////////////////////////////////////////////
}

HRESULT ControlPoints::GetControlPoint( __in UINT pointIndex, __out ControlPoint **ppPoint )
{
///////////////////////////////////////////////////////////////////////////////
#ifdef _ENABLE_CR_BASE_CODE_
    if ( pointIndex >= CONTROL_POINT_COUNT )
///////////////////////////////////////////////////////////////////////////////
// ndef
#else
   if ( pointIndex >= gcp_count )
#endif	// _ENABLE_CR_BASE_CODE_
///////////////////////////////////////////////////////////////////////////////
    {
        return E_INVALIDARG;
    }
    else
    {
        *ppPoint = &m_controlPoints[pointIndex];
        return S_OK;
    }
}

HRESULT ControlPoints::GetControlPointCoordinateSystem( __in UINT pointIndex, __deref_out ICoordinateSystem **ppCoordinateSystem )
{
    pointIndex;
    return m_spCoordinateSystem.CopyTo( ppCoordinateSystem );
}

HRESULT ControlPoints::GetConstraintsCoordinateSystem( __in UINT constraintIndex, __deref_out ICoordinateSystem **ppCoordinateSystem )
{
    constraintIndex;
    return m_spCoordinateSystem.CopyTo( ppCoordinateSystem );
}

const wchar_t* ControlPoints::GetControlPointName( __in UINT pointIndex )
{
    return g_ControlPointNames[pointIndex];
}

UINT ControlPoints::GetConstraintsCount()
{
    return 0;
}

int ControlPoints::FindPoint( __in UINT id )
{
///////////////////////////////////////////////////////////////////////////////
#ifdef _ENABLE_CR_BASE_CODE_
    for ( int i = 0; i < CONTROL_POINT_COUNT; i++ )
///////////////////////////////////////////////////////////////////////////////
// ndef
#else
    for ( int i = 0; i < gcp_count; i++ )
#endif	// _ENABLE_CR_BASE_CODE_
///////////////////////////////////////////////////////////////////////////////
	{
        if ( m_controlPoints[i].id == id )
        {
            return i;
        }
    }
    return -1;
}

ControlPointConstraint *ControlPoints::GetConstraint( __in UINT index )
{
    index;
    return NULL;
}

HRESULT ControlPoints::RecalculatePoint( __in UINT pointIndex )
{
    pointIndex;
    return E_NOTIMPL;
}

void ControlPoints::Lock()
{
    //
}

void ControlPoints::Unlock()
{
    //
}

STDMETHODIMP ControlPoints::QueryInterface(REFIID riid, __deref_out_opt void **ppvObject)
{
    if ( riid == __uuidof(IControlPoints) )
    {
        *ppvObject = (void*)(IControlPoints*)this;
    }
    return E_NOTIMPL;
}

STDMETHODIMP_(ULONG) ControlPoints::AddRef()
{
    return InterlockedIncrement(&m_RefCount);
}

STDMETHODIMP_(ULONG) ControlPoints::Release()
{
    ULONG refCnt = InterlockedDecrement(&m_RefCount);
    if ( !refCnt )
    {
        delete this;
    }
    return refCnt;
}
