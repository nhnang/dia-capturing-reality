#include "stdafx.h"
#include "CubeDistributionAnalyzer.h"
#include<iostream>
#include<fstream>

using namespace std;

CubeDistributionAnalyzer::CubeDistributionAnalyzer(const wchar_t* output)
{
	m_output_dir = new std::wstring(output);
	if (!openDistFile())
	{
		assert(false);
	}

	for (int x = 0; x < CUBE_SLICE; x ++)
	{
		for (int y = 0; y < CUBE_SLICE; y ++)
		{
			for (int z = 0; z < CUBE_SLICE; z ++)
			{
				m_point_count[x][y][z] = 0;
				m_is_under_there[x][y][z] = false;
			}
		}
	}
	// 
	m_cut_surface_limit[0] = -1;
	m_cut_surface_limit[1] = CUBE_SLICE;
	m_cut_surface_limit[2] = -1;
	m_cut_surface_limit[3] = CUBE_SLICE;
	m_cut_surface_limit[4] = -1;
	m_cut_surface_limit[5] = CUBE_SLICE;
}

CubeDistributionAnalyzer::CubeDistributionAnalyzer()
{
	wprintf(L"this ctor is not recommendation.\n");
	assert(false);
}

CubeDistributionAnalyzer::~CubeDistributionAnalyzer()
{
	if (m_output_dir)
	{
		closeDistFile();
		delete m_output_dir;
	}
}

bool CubeDistributionAnalyzer::openDistFile()
{
    m_distribution_stream = new ofstream("cloud_points.txt");
	if (!m_distribution_stream)
	{
		wprintf(L"failure open to cloud_points.txt\n");
		return false;
	}
	return true;
}

bool CubeDistributionAnalyzer::closeDistFile()
{
	if (!m_distribution_stream)
	{
		wprintf(L"cloud_points.txt is not opend.\n");
		return false;
	}
	m_distribution_stream->close();
	return true;
}

bool CubeDistributionAnalyzer::analyzePointsCemara(
		UINT camera_index, 
		const double* px, 
		const double* py,
		const double* pz,
		const double* pw)
{
	if (!m_distribution_stream)
	{
		wprintf(L"output file is not open.\n");
		return false;
	}
	double cx = *px / *pw;
	double cy = *py / *pw;
	double cz = *pz / *pw;

	*m_distribution_stream << camera_index << " x:" << *px << " y:" << *py << " z:" << *pz << " w:"  << *pw << endl;
	for (int x = 0; x < CUBE_SLICE; x ++)
	{
		for (int y = 0; y < CUBE_SLICE; y ++)
		{
			for (int z = 0; z < CUBE_SLICE; z ++)
			{
				if (isPointInside(&cx, &cx, &cx, &m_cube_area[x][y][z]))
				{
					m_point_count[x][y][z] ++;
					wprintf(L"count[%d][%d][%d]=%d\n", x, y, z, m_point_count[x][y][z]);
				}
			}
		}
	}
	return true;
}

bool CubeDistributionAnalyzer::analyzePoints(const SfmReconstructionPoint* pPoints, UINT count)
{
	if (!m_distribution_stream)
	{
		wprintf(L"output file is not open.\n");
		return false;
	}
	// memory points;
	m_tmp_points = pPoints;

	// analyze
	for (int i = 0; i < count; i ++)
	{
		WorldPoint current_point = pPoints[i].X;
		current_point.x /= current_point.w;
		current_point.y /= current_point.w;
		current_point.z /= current_point.w;

		*m_distribution_stream << i << " x:" << pPoints->X.x << " y:" << pPoints->X.y << " z:" << pPoints->X.z << " w:"  << pPoints->X.w;
		*m_distribution_stream << i << " [calc] x:" << current_point.x << " y:" << current_point.y << " z:" << current_point.z << endl;

		for (int x = 0; x < CUBE_SLICE; x ++)
		{
			for (int y = 0; y < CUBE_SLICE; y ++)
			{
				for (int z = 0; z < CUBE_SLICE; z ++)
				{
					// count point cloud
					if (isPointInside(&current_point, &m_cube_area[x][y][z]))
					{
						m_point_count[x][y][z] ++;
						//wprintf(L"count[%d][%d][%d]=%d\n", x, y, z, m_point_count[x][y][z]);
					}
				}
			}
		}
	}

	// Under there fg;
	for (int x = 0; x < CUBE_SLICE; x ++)
	{
		for (int y = 0; y < CUBE_SLICE; y ++)
		{
			for (int z = 0; z < CUBE_SLICE; z ++)
			{
				// check delete candidate fg
				if (AREA_THERESHOLD > m_point_count[x][y][z])
				{
					//wprintf(L"under check [%d][%d][%d] = true. count=%d \n", x,y,z, m_point_count[x][y][z]);
					m_is_under_there[x][y][z] = true;
				}
			}
		}
	}
	return true;
}

bool CubeDistributionAnalyzer::setBoundingBox(const double* p_min, const double* p_max)
{
	if (!p_min || !p_max)
	{
		wprintf(L"CubeDistributionAnalyzer::setBoundingBox Input param is invalid.\n");
		return false;
	}
	// tmp
	m_org_bbox.min.x = p_min[0];
	m_org_bbox.min.y = p_min[1];
	m_org_bbox.min.z = p_min[2];
	m_org_bbox.max.x = p_max[0];
	m_org_bbox.max.y = p_max[1];
	m_org_bbox.max.z = p_max[2];
	// copy buff not ANSI mode.
	m_resized_bbox = m_org_bbox;

	// 
	ofstream outputfile("slice_info.txt");

	// calc size
	m_x_width = p_max[0] - p_min[0];
	m_y_width = p_max[1] - p_min[1];
	m_z_width = p_max[2] - p_min[2];
	// calc part
	m_x_part_width = m_x_width / CUBE_SLICE;
	m_y_part_width = m_y_width / CUBE_SLICE;
	m_z_part_width = m_z_width / CUBE_SLICE;

	// calc cube blocks
	for (int x = 0; x < CUBE_SLICE; x ++)
	{
		for (int y = 0; y < CUBE_SLICE; y ++)
		{
			for (int z = 0; z < CUBE_SLICE; z ++)
			{
				ST_BBOX* current = &m_cube_area[x][y][z];
				ST_POINT* min = &current->min;
				ST_POINT* max = &current->max;
				// calc
				float min_x = p_min[0];
				float min_y = p_min[1];
				float min_z = p_min[2];
				min_x += (x * m_x_part_width);
				min_y += (y * m_y_part_width);
				min_z += (z * m_z_part_width);
				// max
				float max_x = min_x + m_x_part_width;
				float max_y = min_y + m_y_part_width;
				float max_z = min_z + m_z_part_width;

				// set
				min->x = min_x;
				min->y = min_y;
				min->z = min_z;

				max->x = max_x;
				max->y = max_y;
				max->z = max_z;

				// file
				outputfile << x << ":" << y << ":" << z << " ";
				outputfile << "min " << min_x << ":" << min_y << ":" << min_z;
				outputfile << " max " << max_x << ":" << max_y << ":" << max_z << endl;
			}
		}
	}
    outputfile.close();
	return true;
}

bool CubeDistributionAnalyzer::resizeBoundingBox(double* o_min, double* o_max)
{
	if (!o_min || !o_max)
	{
		wprintf(L"CubeDistributionAnalyzer::resizeBoundingBox() Invalid output param.");
		return false;
	}

	/////////////////////////////////////////////////////////////////////

	// XUP
	for (int x = 0; x < CUBE_SLICE; x ++)
	{
		bool is_apply = true;
		for (int y = 0; y < CUBE_SLICE; y ++)
		{
			for (int z = 0; z < CUBE_SLICE; z ++)
			{
				// ‚P‚Â‚Å‚àã‰ñ‚é‚à‚Ì‚ª‚ ‚é‚Ì‚È‚çƒLƒƒƒ“ƒZƒ‹
				if (m_point_count[x][y][z] > AREA_THERESHOLD)
				{
					is_apply = false;
					break;
				}
			}
			if (!is_apply)
			{
				break;
			}
		}
		if (is_apply)
		{
			wprintf(L"XUP cut: x=%d\n", x);
			// cut suface
			if (x > m_cut_surface_limit[0])
			{
				m_cut_surface_limit[0] = x;
			}
		}
		else
		{
			break;
		}
	}

	// XDOWN
	for (int x = CUBE_SLICE - 1; x >= 0; x --)
	{
		bool is_apply = true;
		for (int y = 0; y < CUBE_SLICE; y ++)
		{
			for (int z = 0; z < CUBE_SLICE; z ++)
			{
				// ‚P‚Â‚Å‚àã‰ñ‚é‚à‚Ì‚ª‚ ‚é‚Ì‚È‚çƒLƒƒƒ“ƒZƒ‹
				if (m_point_count[x][y][z] > AREA_THERESHOLD)
				{
					is_apply = false;
					break;
				}
			}
			if (!is_apply)
			{
				break;
			}
		}
		if (is_apply)
		{
			wprintf(L"XDOWN cut: x=%d\n", x);
			// cut suface
			if (m_cut_surface_limit[1] > x)
			{
				m_cut_surface_limit[1] = x;
			}
		}
		else
		{
			break;
		}
	}

	/////////////////////////////////////////////////////////////////////

	// YUP
	for (int y = 0; y < CUBE_SLICE; y ++)
	{
		bool is_apply = true;
		for (int x = 0; x < CUBE_SLICE; x ++)
		{
			for (int z = 0; z < CUBE_SLICE; z ++)
			{
				// ‚P‚Â‚Å‚àã‰ñ‚é‚à‚Ì‚ª‚ ‚é‚Ì‚È‚çƒLƒƒƒ“ƒZƒ‹
				if (m_point_count[x][y][z] > AREA_THERESHOLD)
				{
					is_apply = false;
					break;
				}
			}
			if (!is_apply)
			{
				break;
			}
		}
		if (is_apply)
		{
			// cut suface
			wprintf(L"YUP cut: y=%d\n", y);
			if (y > m_cut_surface_limit[2])
			{
				m_cut_surface_limit[2] = y;
			}
		}
		else
		{
			break;
		}
	}

	// YDOWN
	for (int y = CUBE_SLICE - 1; y >= 0; y --)
	{
		bool is_apply = true;
		for (int x = 0; x < CUBE_SLICE; x ++)
		{
			for (int z = 0; z < CUBE_SLICE; z ++)
			{
				// ‚P‚Â‚Å‚àã‰ñ‚é‚à‚Ì‚ª‚ ‚é‚Ì‚È‚çƒLƒƒƒ“ƒZƒ‹
				if (m_point_count[x][y][z] > AREA_THERESHOLD)
				{
					is_apply = false;
					break;
				}
			}
			if (!is_apply)
			{
				break;
			}
		}
		if (is_apply)
		{
			wprintf(L"YDOWN cut: y=%d\n", y);
			// cut suface
			if (m_cut_surface_limit[3] > y)
			{
				m_cut_surface_limit[3] = y;
			}
		}
		else
		{
			break;
		}
	}

	/////////////////////////////////////////////////////////////////////

	// ZUP
	for (int z = 0; z < CUBE_SLICE; z ++)
	{
		bool is_apply = true;
		for (int x = 0; x < CUBE_SLICE; x ++)
		{
			for (int y = 0; y < CUBE_SLICE; y ++)
			{
				// ‚P‚Â‚Å‚àã‰ñ‚é‚à‚Ì‚ª‚ ‚é‚Ì‚È‚çƒLƒƒƒ“ƒZƒ‹
				if (m_point_count[x][y][z] > AREA_THERESHOLD)
				{
					is_apply = false;
					break;
				}
			}
			if (!is_apply)
			{
				break;
			}
		}
		if (is_apply)
		{
			// cut suface
			wprintf(L"ZUP cut: z=%d\n", z);
			if (z > m_cut_surface_limit[4])
			{
				m_cut_surface_limit[4] = z;
			}
		}
		else
		{
			break;
		}
	}

	// ZDOWN
	for (int z = CUBE_SLICE - 1; z >= 0; z --)
	{
		bool is_apply = true;
		for (int x = 0; x < CUBE_SLICE; x ++)
		{
			for (int y = 0; y < CUBE_SLICE; y ++)
			{
				if (m_point_count[x][y][z] > AREA_THERESHOLD)
				{
					is_apply = false;
					break;
				}
			}
			if (!is_apply)
			{
				break;
			}
		}
		if (is_apply)
		{
			wprintf(L"ZDOWN cut: z=%d\n", z);
			// cut suface
			if (m_cut_surface_limit[5] > z)
			{
				m_cut_surface_limit[5] = z;
			}
		}
		else
		{
			break;
		}
	}

    ofstream outputfile("resize.txt");

	// write
	for (int i = 0; i < 6; i ++)
	{
		outputfile << m_cut_surface_limit[i] << endl;
	}

    outputfile.close();
	return cutSurface(o_min, o_max, m_cut_surface_limit);
}

const ST_BBOX* CubeDistributionAnalyzer::getResizedBbox()
{
	return &m_resized_bbox;
}

bool CubeDistributionAnalyzer::isInsideReducedBbox(const double* x, const double* y, const double* z)
{
	return isPointInside(x, y, z, &m_resized_bbox);
}

bool CubeDistributionAnalyzer::cutSurface(double* o_min, double* o_max, const int* resize_info)
{
	if (!o_min || !o_max || !resize_info)
	{
		return false;
	}

	// TODO: to Structured func.

	///////////////////////////////////
	// X min
	int x_up = resize_info[0];
	if (x_up == -1)
	{
		o_min[0] = m_org_bbox.min.x;
	}
	else
	{
		o_min[0] = m_resized_bbox.min.x = m_org_bbox.min.x + (x_up * m_x_part_width);
	}

	// X max
	int x_down = resize_info[1];
	if (x_down == CUBE_SLICE)
	{
		o_max[0] = m_org_bbox.max.x;
	}
	else
	{
		int x_down_diff = CUBE_SLICE - x_down;
		o_max[0] = m_resized_bbox.max.x = m_org_bbox.max.x - (x_down_diff * m_x_part_width);
	}

	///////////////////////////////////
	// Y min
	int y_up = resize_info[2];
	if (y_up == -1)
	{
		o_min[1] = m_org_bbox.min.y;
	}
	else
	{
		o_min[1] = m_resized_bbox.min.y = m_org_bbox.min.y + (y_up * m_y_part_width);
	}

	// Y max
	int y_down = resize_info[3];
	if (y_down == CUBE_SLICE)
	{
		o_max[1] = m_org_bbox.max.y;
	}
	else
	{
		int y_down_diff = CUBE_SLICE - y_down;
		o_max[1] = m_resized_bbox.max.y = m_org_bbox.max.y - (y_down_diff * m_y_part_width);
	}

	///////////////////////////////////
	// Z min
	int z_up = resize_info[4];
	if (z_up == -1)
	{
		o_min[2] = m_org_bbox.min.z;
	}
	else
	{
		o_min[2] = m_resized_bbox.min.z = m_org_bbox.min.z + (z_up * m_z_part_width);
	}

	// Z max
	int z_down = resize_info[5];
	if (z_down == CUBE_SLICE)
	{
		o_max[2] = m_org_bbox.max.z;
	}
	else
	{
		int z_down_diff = CUBE_SLICE - z_down;
		o_max[2] = m_resized_bbox.max.z = m_org_bbox.max.z - (z_down_diff * m_z_part_width);
	}

	// 
	return true;
}

void CubeDistributionAnalyzer::report()
{
	{
		ofstream outputfile("cube.txt");

		for (int x = 0; x < CUBE_SLICE; x ++)
		{
			for (int y = 0; y < CUBE_SLICE; y ++)
			{
				for (int z = 0; z < CUBE_SLICE; z ++)
				{
					outputfile << x << ":" << y << ":" << z << " ";
					outputfile << m_point_count[x][y][z] << endl;
				}
			}
		}
		outputfile.close();
	}
	{
		ofstream outputfile("bbox.txt");

		outputfile << "original bbox" << endl;
		outputfile << "min " << m_org_bbox.min.x << ":" << m_org_bbox.min.y << ":" << m_org_bbox.min.z << endl;
		outputfile << "max " << m_org_bbox.max.x << ":" << m_org_bbox.max.y << ":" << m_org_bbox.max.z << endl;
	
		outputfile << "resize bbox" << endl;
		outputfile << "min " << m_resized_bbox.min.x << ":" << m_resized_bbox.min.y << ":" << m_resized_bbox.min.z << endl;
		outputfile << "max " << m_resized_bbox.max.x << ":" << m_resized_bbox.max.y << ":" << m_resized_bbox.max.z << endl;

		outputfile.close();
	}
}

bool CubeDistributionAnalyzer::isPointInside(const WorldPoint* point, const ST_BBOX* box)
{
	return (point->x >= box->min.x && point->x <= box->max.x) &&
			(point->y >= box->min.y && point->y <= box->max.y) &&
			(point->z >= box->min.z && point->z <= box->max.z);
}

bool CubeDistributionAnalyzer::isPointInside(const double* px, const double* py, const double* pz, const ST_BBOX* box)
{
	return (*px >= box->min.x && *px <= box->max.x) &&
			(*py >= box->min.y && *py <= box->max.y) &&
			(*pz >= box->min.z && *pz <= box->max.z);
}
