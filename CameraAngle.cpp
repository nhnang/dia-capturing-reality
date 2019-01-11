#include "stdafx.h"
#include "CameraAngle.h"

extern FILE* fp;

const double RAD_TO_DEG = (180 / 3.1415926535897932384626433832795);

// res = a + b 
template <class T> inline void Vect3Add( _In_reads_(3) const T *a, _In_reads_(3) const T* b, _Out_writes_(3) T* res)
{
    res[0] = a[0] + b[0];
    res[1] = a[1] + b[1];
    res[2] = a[2] + b[2];
}

void PrintGpsLatitude(__in double src)
{
	double angle, min, sec;
	sec = fabs(src) * RAD_TO_DEG;
	angle = floor(sec);
	sec = (sec - angle) * 60;
	min = floor(sec);
	sec = (sec - min) * 60;
    fprintf(fp, "%c%.0f,%.0f,%.2f\n", src >= 0 ? 'N' : 'S' , angle, min, sec );
}

void PrintGpsLongitude(__in double src)
{
    double angle, min, sec;
	sec = fabs(src) * RAD_TO_DEG;
	angle = floor(sec);
	sec = (sec - angle) * 60;
	min = floor(sec);
	sec = (sec - min) * 60;
    fprintf(fp, "%c%.0f,%.0f,%.2f\n", src >= 0 ? 'E' : 'W', angle, min, sec );
}

//TODO verify correctness
HRESULT PrintCalculatedCameraPositionsInGps( __in ISfmReconstruction *pReconstruction, __in IStructureFromMotion *pSfm )
{
    using namespace CapturingReality;
    using namespace CapturingReality::Sfm;

	HRESULT hr = S_OK;

	// AW: test
	// WARNING code
	UINT flag = pReconstruction->GetReconstructionFlags();
	UINT SCCF_FG = SCCF_GROUND_CONTROL | SCCF_METRIC;
	// ‚±‚Ì‡¬”»’è‚Å‚Íí‚É‹U‚É‚È‚é‰Â”\«‚ª‚ ‚é‚Ì‚Å‚ÍH
	// &‚Å‚Í‚PŒ…–Ú‚É‚P‚ª—ˆ‚éƒtƒ‰ƒO‚ÆAã‚ÌŒ…‚É—ˆ‚éƒtƒ‰ƒO‚ð‡¬‚µ‚Ä‚à•K‚¸ƒ[ƒi‹Uj‚É‚È‚é
	UINT if_fg = ( pReconstruction->GetReconstructionFlags() & (SCCF_GROUND_CONTROL | SCCF_METRIC) );

	// if ( pReconstruction->GetReconstructionFlags() & (SCCF_GROUND_CONTROL | SCCF_METRIC) )
	if (true)
    {
		CComPtr< CapturingReality::CoordinateSystems::ICoordinateSystem > spGpsCoordSystem;
		const wchar_t* g_gps_name = L"GPS (WGS 84)";
		const wchar_t* g_gps_definition = L"+proj=longlat +datum=WGS84 +no_defs";
		hr = CapturingReality::CoordinateSystems::CreateCoordinateSystem( g_gps_name, g_gps_definition, &spGpsCoordSystem );
        if ( SUCCEEDED(hr) )
        {
			ISfmCameraModel *pCameraModel = pReconstruction->GetCameraModel();
			CoordinateSystemAnchor anchor = pReconstruction->GetAnchor();
			CoordinateSystemGroundPlane groundPlane = pReconstruction->GetGroundPlane();

			UINT count;
			SfmCamera *pCameras;
			hr = pReconstruction->GetCameras( &pCameras, &count );
			if ( SUCCEEDED(hr) )
			{
				for ( UINT i = 0; i < count; i++, pCameras++ )
				{
					double cameraCentre[4];
					pCameraModel->GetCameraCentre( *pCameras, cameraCentre );

					CoordinateSystemPoint point;
					MulV3M33( cameraCentre, groundPlane.R, &point.x );
					Vect3Add( &point.x, &anchor.x, &point.x );

					hr = spGpsCoordSystem->ToModel( 1, &point );
       
					if ( SUCCEEDED(hr) )
					{
						double longitude = ( point.x / XM_PI ) * 0.5;
						double latitude  = -( log( tan( point.y ) + ( 1.0 / cos( point.y ) ) ) / XM_PI ) * 0.5;
						double altitude  = point.z;

						fprintf(fp, " Calculated position latitude: " );
						PrintGpsLatitude( latitude );
                    
						fprintf(fp, " longitude: ");
						PrintGpsLongitude( latitude );

						fprintf(fp, " altitude:%.2f\n", altitude );
					}

					// ANGEWORK
					// get rotation test
					RigCameraTransform transform;
				    CComPtr< ISfmRigInstance > rigInstance;
					for (int i = 0; i < 10; i ++)
					{
						pSfm->GetRigInstance(i, &rigInstance);
						if (rigInstance)
						{
							rigInstance->GetAbsoluteTransform(&transform);
						}
						else
						{
							printf("instance == null\n");
						}
					}
					if ( FAILED( hr ) )
					{
						break;
					};
				}
			}
		}
    }
	else
	{
		hr = S_FALSE; //the component is not geo-refrerenced
	};

    return hr;
}

HRESULT PrintReconstructionStats( __in IStructureFromMotion *pSfm )
{
    using namespace CapturingReality;
    using namespace CapturingReality::Sfm;

    UINT recCount = pSfm->GetReconstructionsCount();
    fprintf(fp, "Calculated %d components\n", recCount );
    for ( UINT i = 0; i < recCount; i++)
    {
        CComPtr< ISfmReconstruction > spReconstruction;
		pSfm->GetReconstruction( i, &spReconstruction );
		
		UINT structureSize;
		HRESULT hr = spReconstruction->GetStructureSize( &structureSize );
		if ( FAILED(hr) )
        {
			return hr;
        }

		// ANGEWORK:
		// Test for CR new func.
		HRESULT tmp_hr = PrintCalculatedCameraPositionsInGps(spReconstruction, pSfm);

		UINT cameraCount = spReconstruction->GetCamerasCount();
        fprintf(fp, "Component %d consists of %d cameras and %d global points\n", i, cameraCount, structureSize );

        // now let us print some camera information
        if ( cameraCount > 0 )
        {
            int cameraIndex = 0;

            fprintf(fp, "Here is some info about Camera %d\n", cameraIndex );

            SfmCamera camera;
            hr = spReconstruction->GetCamera( cameraIndex, &camera );
		    if ( FAILED(hr) )
            {
			    return hr;
            }

            // get camera input file; it is not necessary an image
            // following information are available even prior to registration
            // and are can be read the same way

            CComPtr< ISfmStreamInput > spInput;
            hr = pSfm->GetImage( camera.sfmImage, &spInput );
		    if ( FAILED(hr) )
            {
			    return hr;
            }

            // Uncomment to setup calibration groups, i.e., cameras with the same
            // group ID will share the same calibrations/lens parameters
            // CComQIPtr< ISfmImage > spImage = spInput;
            // if ( spImage )
            // {
            //     SfmImageParametersGroups p;
            //     p.calibrationGroup = 1;
            //     p.distortionGroup = 1;
            //     spImage->SetParameterGrouping( p );
            // }

            UINT width, height;
            hr = spInput->GetImageSize( &width, &height );
		    if ( FAILED(hr) )
            {
			    return hr;
            }

            const wchar_t *pInputFileName = spInput->GetFileName();
            fprintf(fp, " Source file name: \"%S\"\n", pInputFileName );
            fprintf(fp, " Width: %d pixels\n", width );
            fprintf(fp, " Height: %d pixels\n", height );

            // check if the input is geo-referenced
            // it can be in any coordinate system
            SfmCoordinateSystem csPose;
            SfmCoordinateSystemType csType;
            CComPtr< ICoordinateSystem > spCS;
            hr = spInput->GetPriorPoseAbsolute( &csPose, &csType, &spCS );
		    if ( FAILED(hr) )
            {
			    return hr;
            }

            if ( csType & SFMCST_REGISTERED )
            {
                fprintf(fp, " Image is geo-referenced\n" );

                if ( spCS->IsLatitudeLongitude() )
                {
                    double longitude = csPose.position[0];
                    double latitude  = csPose.position[1];
                    double altitude  = csPose.position[2];

                    fprintf(fp, " Prior position latitude: " );
                    PrintGpsLatitude( latitude );
                    
                    fprintf(fp, " longitude: ");
                    PrintGpsLongitude( latitude );

                    fprintf(fp, " altitude:%.2f\n", altitude );
                }
            }

            // now print some registration outcomes
            fprintf(fp, " Focal length: %.1fmm\n", camera.K.K.focalLength * 36.0 );

            // all information are stored relative to the unit-sized image (the bigger side of image is 1)
            // this way image resolution is not important and you can freely downscale/upscale images in 
            // processes
            double unit2Pixel = max( width,height );
            fprintf(fp, " Principal point: [%.1fpx, %.1fpx]\n", camera.K.K.principalU * unit2Pixel + width / 2.0, camera.K.K.principalU * unit2Pixel + height / 2.0 );

            // distortion values have meaning only w.r.t. selected camera model; this is defined in config prior to 
            // registration and by default it is set to Brown model

            // image is distorted in general and camera model serves to project
            // between image and 3D space
            ISfmCameraModel *pModel = spReconstruction->GetCameraModel();
            _ASSERT( pModel );

            UINT cameraModelId = pModel->GetModelId();
            
            fprintf(fp, " Distortion model: " );
            switch ( cameraModelId & 0xff ) 
            {
                case SFMLDM_DIVISION:
                    fprintf(fp, "Division Model\n" );
                    break;
                case SFMLDM_BROWN_R3:
                    fprintf(fp, "Brown 3 param\n" );
                    break;
                case SFMLDM_BROWN_R3_T2:
                    fprintf(fp, "Brown 3 param with tangential distortion\n" );
                    break;
                case SFMLDM_BROWN_R4:
                    fprintf(fp, "Brown 4 param\n" );
                    break;
                case SFMLDM_BROWN_R4_T2:
                    fprintf(fp, "Brown 4 param with tangential distortion\n" );
                    break;
                    
            }

            fprintf(fp, " Radial distortion: [%.1f, %.1f, %.1f, %.1f] tangential distortion: [%.1f, %.1f]\n", camera.K.rd.radial1, camera.K.rd.radial2, camera.K.rd.radial3, camera.K.rd.radial4, camera.K.rd.tangential1, camera.K.rd.tangential2 );

            // let us project some 3D point to the image space

            UINT pointsCount;
            SfmReconstructionPoint *pPoints;
            hr = spReconstruction->GetStructure( &pPoints, &pointsCount );
            _ASSERT( SUCCEEDED(hr) );
            
            WorldPoint X;
            X.x = pPoints[0].X.x;
            X.y = pPoints[0].X.y;
            X.z = pPoints[0].X.z;
            X.w = pPoints[0].X.w;

            double projection[3];
            pModel->GetProjection( camera, X, projection );

            // de-homogenize 3D point 
            double x = X.x / X.w;
            double y = X.y / X.w;
            double z = X.z / X.w;

            if ( projection[2] < 0 )
            {
                fprintf(fp, " The projected 3D point [%.2f, %.2f, %.2f] is behind the camera\n", x, y, z );
            }
            else
            {
                fprintf(fp, " The point [%.2f, %.2f, %.2f] projects to [%.2fpx, %.2fpx] \n", x, y, z, projection[0] * unit2Pixel + width / 2, projection[1] * unit2Pixel + height / 2 );
            }
        }
    }

    return S_OK;
}
