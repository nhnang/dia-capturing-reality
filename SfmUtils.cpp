//
// Capturing Reality s.r.o.
//
// This file 'SfmUtils.cpp' is a part of the RC Engine Samples. You may use it freely.
//

#include "stdafx.h"
#include "SfmUtils.h"
#include "ControlPoints.h"

#define ADD_IMAGE(imageName) hr = spSfm->AddImage((imageName), NULL, NULL, 0); if (FAILED(hr)) return hr;

///////////////////////////////////////////////////////////////////////////////
// ndef
#ifndef _ENABLE_CR_BASE_CODE_

#include <vector>
#include <cstdio>
#include <iostream>
#include <locale>
#include <vector>
#include <string>
#include <fstream>

//////////////////////////////////////////////////////////////////////////////////////////////////////
// �w�肳�ꂽ���X�g��ǂݍ��݁A���̃t�@�C���ꗗ�̑��݃`�F�b�N���s���܂��B
int loadImageList(
	__in const wchar_t* image_dir,
	__in const wchar_t* list_dir,
	__in const wchar_t* imagelist_file,
	__out wchar_t** result)
{
	// ���݃`�F�b�N���s���܂��B
	if (!PathFileExists(image_dir))
	{
		wprintf(L"\n%s dir is not found.", image_dir);
		exit(-1);
	}
	// �����₷��wstr�`���ɂ��܂�
	std::wstring imagedir_str(image_dir);

	// ���݃`�F�b�N���s���܂��B
	if (!PathFileExists(list_dir))
	{
		wprintf(L"\n%s dir is not found.", list_dir);
		exit(-1);
	}
	// �����₷��wstr�`���ɂ��܂�
	std::wstring listdir_str(list_dir);

	// ���݃`�F�b�N���s���܂��B
	std::wstring imagelist_str(imagelist_file);
	std::wstring imagelist_path = listdir_str + imagelist_str;
	if (!PathFileExists(imagelist_path.c_str()))
	{
		wprintf(L"\n%s file is not found.", imagelist_path);
		exit(-1);
	}

	// �P�s���̉��o�b�t�@
	std::wstring wline;
    std::wifstream stream(imagelist_path);
    if(stream.fail())
	{
		wprintf(L"loadImageList() failure open %s.", imagelist_path.c_str());
		exit(-2);
	}
	// �P�s���ǂݏo���܂�
	int index = 0;
    while(getline(stream, wline))
	{
		if (2 > wline.length())
		{
			OutputDebugString(L"�󔒂��X�L�b�v���܂��B\n");
			continue;
		}

		// �f�B���N�g���ƃt���p�X�𑫂��܂�
		std::wstring fullpath = imagedir_str + wline;
		std::wcout << fullpath << std::endl;
		if (!PathFileExists(fullpath.c_str()))
		{
			wprintf(L"loadImageList() %ls is not found.\n", fullpath.c_str());
			exit(-2);
		}
		// �P�v�I�ȃ��������m�ۂ��܂�
		wchar_t* buff = new wchar_t[wcslen(fullpath.c_str()) + 1];
		wcscpy_s(buff, wcslen(fullpath.c_str()) + 1, fullpath.c_str());
		// �߂�l�ɓn���܂��B
		result[index] = buff;
		index ++;
	}
	return index;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// �f�B���N�g���̃t�@�C���ꗗ�����X�g�A�b�v���܂��B
int makeFileList(
	__in const wchar_t* input,
	__out wchar_t** result)
{
	//	wchar_t *imagesList2[1000];
	int index = 0;

	HANDLE hFind;				// �n���h��
	WIN32_FIND_DATA win32fd;	// �t�@�C�����

	wchar_t dirwildpath[1024];		// �t�@�C�������܂񂾃p�X
									//	const wchar_t* base_dir = L"C:\\input\\";
									//	const wchar_t* base_dir = L"..\\Resources\\Imageset1\\";
	wcscpy_s(dirwildpath, input);		// �t�@�C���p�X���R�s�[
	wcscat_s(dirwildpath, L"*");		// ���C���h�J�[�h��ǉ�

	hFind = FindFirstFile(dirwildpath, &win32fd);

	// ����������Ȃ������Ƃ�
	if (hFind == INVALID_HANDLE_VALUE)
	{
		// �t�@�C���N���[�Y
		FindClose(hFind);
		MessageBox(NULL, L"������܂���ł���", L"sample", MB_OK);
		return -1;
		//		exit(-1);
	}
	do {
		// �J�����g�f�B���N�g���Ɛe�f�B���N�g���̂Ƃ�
		if ((wcscmp(win32fd.cFileName, L".") == 0)
			|| (wcscmp(win32fd.cFileName, L"..") == 0))
		{
			// �������Ȃ�
		}
		else if (win32fd.dwFileAttributes
			& FILE_ATTRIBUTE_DIRECTORY)
		{// �f�B���N�g���̂Ƃ��̏���
		 // �������Ȃ�
		}
		else
		{// �t�@�C���̂Ƃ�
			wprintf(L"(FILE)-%ls\n", win32fd.cFileName);
			// �o�b�t�@�쐬
			//			wchar_t* tmp = (wchar_t*)malloc(wcslen(base_dir) + wcslen(win32fd.cFileName) + 1);
			wchar_t tmp[1000];
			//			wchar_t* tmp = new wchar_t[1000];
			wcscpy_s(tmp, input);			// �t�@�C���p�X���R�s�[
			wcscat_s(tmp, win32fd.cFileName);
			//			wcscat_s(tmp, L"\0");

			size_t dir_len = wcslen(input);
			size_t file_len = wcslen(win32fd.cFileName);
			size_t buff_len = dir_len + file_len + 1;
			wchar_t* buff = (wchar_t*)malloc(buff_len * sizeof(wchar_t));
			wcscpy_s(buff, buff_len, tmp);

			wprintf(L"(PATH)-%ls\n", buff);
			result[index] = buff;
			index++;
		}

	} while (FindNextFile(hFind, &win32fd));

	// �t�@�C���N���[�Y
	FindClose(hFind);

	return index;
}


void clearFileList(__in wchar_t** result, __in int count)
{
	for (int i = 0; i < count; i++)
	{
		free(result[i]);
	}
}

#endif // _ENABLE_CR_BASE_CODE_
///////////////////////////////////////////////////////////////////////////////

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
)
{
    if ( ( pCache == NULL ) || ( pConfig == NULL ) )
    {
        return E_INVALIDARG;
    }

    // Load the sensor database if present. It is not mandatory, but
    // prior knowledge about the sensor can speed-up the alignment.

    CComPtr< CapturingReality::Sfm::ISfmSensorDatabase > spSensors;
    CreateSfmSensorDatabase( L"sensorsdb.xml", &spSensors ); // spSensorsDb == NULL on error

    HRESULT hr = S_OK;

    //
    // In order to obtain geo-referenced alignment we need
    // to provide the ground control points to the sfm pipeline.
    //

    CComPtr< CapturingReality::Sfm::IControlPoints > spControlPoints;

    // First prepare the coordinate system.

    CComPtr< CapturingReality::Sfm::ICoordinateSystemTable > spCoordinateSystemTable;

    if ( SUCCEEDED( hr ) )
    {
        hr = CreateCoordinateSystemTable( &spCoordinateSystemTable );
    }

    if ( SUCCEEDED( hr ) )
    {
        spCoordinateSystemTable->SetDefaultCoordinateSystem( spCoordSys );
    }

///////////////////////////////////////////////////////////////////////////////
// ndef
#ifndef _ENABLE_CR_BASE_CODE_
	// �摜���X�g��ǂݍ���Ńn�[�h�R�[�f�B���O�̑���Ƃ���
	// ControlPoints��
	wchar_t* imagesList[2500];
	wchar_t** pp_image_list = imagesList;
	int count = 
		loadImageList(
			inputimage_dir,
			inputparam_dir,
			L"ImageList.txt",
			imagesList);
#endif // _ENABLE_CR_BASE_CODE_
///////////////////////////////////////////////////////////////////////////////

    // Prepare our hard coded ground control points. See ControlPoints.h.

///////////////////////////////////////////////////////////////////////////////
#ifdef _ENABLE_CR_BASE_CODE_
	if ( SUCCEEDED( hr ) )
    {
        hr = ControlPoints::CreateInstance( spCoordSys, &spControlPoints );
    }
///////////////////////////////////////////////////////////////////////////////
// ndef
#else
	if ( SUCCEEDED( hr ) )
    {
        hr = 
			ControlPoints::CreateInstance(
				spCoordSys,
				&spControlPoints, 
				inputparam_dir,
				imagesList[0]
				);
    }
	if (!SUCCEEDED(hr))
	{
		OutputDebugString(L"ControlPoints::CreateInstance error.");
		printf("ControlPoints::CreateInstance error %s", hr);
		exit(-255);
	}
#endif // _ENABLE_CR_BASE_CODE_
///////////////////////////////////////////////////////////////////////////////

    //
    // Create a SFM structure in our coordinate system. We register images,
    // but do not align yet. It is done in CreateReconstruction() in case
    // there is aligned reconstruction found from previous runs.
    //

    CComPtr< CapturingReality::Sfm::IStructureFromMotion > spSfm;
    if ( SUCCEEDED( hr ) )
    {
        hr = CapturingReality::Sfm::CreateSfmPipeline( pConfig, spCoordinateSystemTable, spControlPoints, pCache, spSensors, NULL, &spSfm );
    }

///////////////////////////////////////////////////////////////////////////////
#ifdef _ENABLE_CR_BASE_CODE_

	if ( SUCCEEDED( hr ) )
    {
        // Add images.

        ADD_IMAGE( L"..\\Resources\\Imageset3\\000.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\001.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\002.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\003.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\004.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\005.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\006.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\007.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\008.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\009.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\010.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\011.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\012.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\013.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\014.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\015.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\016.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\017.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\018.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\019.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\020.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\021.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\022.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\023.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\024.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\025.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\026.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\027.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\028.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\029.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\030.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\031.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\032.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\033.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\034.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\035.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\036.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\037.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\038.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\039.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\040.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\041.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\042.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\043.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\044.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\045.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\046.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\047.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\048.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\049.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\050.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\051.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\052.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\053.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\054.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\055.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\056.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\057.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\058.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\059.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\060.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\061.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\062.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\063.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\064.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\065.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\066.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\067.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\068.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\069.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\070.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\071.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\072.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\073.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\074.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\075.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\076.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\077.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\078.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\079.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\080.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\081.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\082.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\083.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\084.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\085.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\086.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\087.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\088.jpg" );
        ADD_IMAGE( L"..\\Resources\\Imageset3\\089.jpg" );
    }
///////////////////////////////////////////////////////////////////////////////
// ndef
#else

	for (int i = 0; i < count; i ++)
	{
		wprintf(L"Addimage %s\n", imagesList[i]);
		hr = ADD_IMAGE(imagesList[i]);
		if ( !SUCCEEDED( hr ) )
		{
			OutputDebugString(L"AddImage error.");
			printf("AddImage error %s", hr);
			exit(-256);
		}
	}

#endif // _ENABLE_CR_BASE_CODE_
///////////////////////////////////////////////////////////////////////////////

	if ( SUCCEEDED( hr ) )
    {
        *ppSfm = spSfm.Detach();
    }

    return hr;
}

HRESULT CreateReconstruction(
    __in        CapturingReality::Sfm::IStructureFromMotion*    pSfm,
    __deref_out CapturingReality::Sfm::ISfmReconstruction**     ppReconstruction
)
{
    if ( pSfm == NULL )
    {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;

    //
    // Align SFM structures with registered input images to obtain a reconstruction.
    //

    if ( SUCCEEDED( hr ) )
    {
        printf( "Calculating camera parameters\n" );
        hr = pSfm->RegisterImages( NULL );
    }

    //
    // More components can be reconstructed. We select the largest one,
    // which is, most likely, the most interesting one.
    //

    UINT nReconstructions = 0;

    if ( SUCCEEDED( hr ) )
    {
        hr = UnlockReconstructionsIfNeeded( pSfm );
    }

    if ( SUCCEEDED( hr ) )
    {
        nReconstructions = pSfm->GetReconstructionsCount();
        if ( nReconstructions == 0 )
        {
            *ppReconstruction = NULL;
            return hr;
        }
    }

    UINT largestComponentIndex = 0;

    if ( SUCCEEDED( hr ) )
    {
        UINT largestComponentCameraCount = 0;

        printf( "Calculated %d components\n", nReconstructions );

        for (UINT ireconstruction = 0; ireconstruction < nReconstructions; ireconstruction++)
        {
            CComPtr< CapturingReality::Sfm::ISfmReconstruction > spReconstruction;
            UINT structureSize;

            hr = pSfm->GetReconstruction( ireconstruction, &spReconstruction );
            if ( SUCCEEDED( hr ) )
            {
                hr = spReconstruction->GetStructureSize( &structureSize );
            }
            if ( SUCCEEDED( hr ) )
            {
                UINT cameraCount = spReconstruction->GetCamerasCount();
                printf( "Component %d consists of %d cameras and %d global points\n", ireconstruction, cameraCount, structureSize );

                if ( cameraCount >= largestComponentCameraCount )
                {
                    largestComponentIndex = ireconstruction;
                    largestComponentCameraCount = cameraCount;
                }
            }
            else
            {
                break;
            }
        }
    }

    // Select the component with the largest number of aligned cameras.

    if ( SUCCEEDED( hr ) )
    {
        hr = pSfm->GetReconstruction( largestComponentIndex, ppReconstruction );
    }

    return hr;
}
