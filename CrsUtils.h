//
// RealityCapture s.r.o.
//
// CrsUtils.h
//

#ifndef CRSUTILS_H_INCLUDED
#define CRSUTILS_H_INCLUDED

struct crsBoundType
{
	double South_Latitude;
	double North_Latitude;
	double Left_Longitude;
	double Right_Longitude;
};

bool toSearchCode(
	__in	double x,
	__in	double y,
	__out	int* o_wgs_info
);

#endif
