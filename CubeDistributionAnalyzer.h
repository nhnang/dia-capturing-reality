/////////////////////////////////////////
// Cube Distribution Analyzer 

#ifndef CUBE_ANALYZER_H_INCLUDED
#define CUBE_ANALYZER_H_INCLUDED

#include "stdafx.h"
#include <string>
#include <CapturingReality.Sfm.h>

struct CapturingReality::Sfm::SfmReconstructionPoint;

#define		CUBE_SLICE			16
#define		CUBE_BLOCKS			CUBE_SLICE * CUBE_SLICE * CUBE_SLICE
#define		AREA_THERESHOLD		100

// this count is from SDK sample
#define		MAX_POINTS			5000000

struct ST_POINT
{
	double x;
	double y;
	double z;
};

// Bounding Box Struct
struct ST_BBOX 
{
	ST_POINT min;
	ST_POINT max;
};

struct ST_CUBE_BBOX
{
	ST_BBOX top_parts[4];
	ST_BBOX low_parts[4];
};

using namespace CapturingReality;
using namespace CapturingReality::Sfm;

class ostream;

// 
class CubeDistributionAnalyzer
{
public:
	CubeDistributionAnalyzer(const wchar_t* output);
	~CubeDistributionAnalyzer();

public:
	// set the maximum area
	bool setBoundingBox(const double* p_min, const double* p_max);
	// file
	bool openDistFile();
	bool closeDistFile();
	// analyze
	bool analyzePointsCemara(UINT camera_index, const double* px, const double* py, const double* pz, const double* pw);
	bool analyzePoints(const CapturingReality::Sfm::SfmReconstructionPoint* pPoints, UINT count);

public:

	// cut slice cube area
	bool resizeBoundingBox(double* o_min, double* o_max);
	const ST_BBOX* getResizedBbox();
	bool isInsideReducedBbox(const double* x, const double* y, const double* z);

public:

	void report();

private:

	// default ctor
	CubeDistributionAnalyzer();
	// check func
	bool isPointInside(const WorldPoint* point, const ST_BBOX* box);
	bool isPointInside(const double* px, const double* py, const double* pz, const ST_BBOX* box);

	// innner method
	bool cutSurface(double* o_min, double* o_max, const int* resize_info);

private:

	std::wstring* m_output_dir;
	std::ofstream* m_distribution_stream;

	// buf
	ST_BBOX m_org_bbox;
	ST_BBOX m_resized_bbox;

	// all
	float m_x_width;
	float m_y_width;
	float m_z_width;
	// xyz split parts
	float m_x_part_width;
	float m_y_part_width;
	float m_z_part_width;
	// area vertex
	ST_BBOX m_cube_area[CUBE_SLICE][CUBE_SLICE][CUBE_SLICE];

	// 
//	double m_point_cloud[MAX_POINTS][3];
	const SfmReconstructionPoint* m_tmp_points;

	// 
	UINT m_point_count[CUBE_SLICE][CUBE_SLICE][CUBE_SLICE];

	// theresold fg
	bool m_is_under_there[CUBE_SLICE][CUBE_SLICE][CUBE_SLICE];

	// x-ud y-ud z-ud
	int m_cut_surface_limit[6];
};

#endif