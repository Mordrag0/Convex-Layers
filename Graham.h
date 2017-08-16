#pragma once

#include <vector>
#include "Point.h"

struct GPoint
{
	Point Loc;
	double Angle;

	GPoint() : Loc(), Angle(0) {};
	GPoint(Point P, double Angle) : Loc(P), Angle(Angle) {};
	GPoint(Point P) : Loc(P), Angle(0) {};
};

class Graham
{
public:
	Graham();
	~Graham();

	// Tvorba ene izboèene lupine.
	std::vector<Point> Scan();

	std::vector<std::vector<Point>> GetConvexLayers(std::vector<Point> Points);


private:
	std::vector<GPoint> Points;
	std::vector<GPoint> Inner;

	GPoint P0;
	double LastAngle;
	
	static bool Graham::CompareLoc(GPoint A, GPoint B);
	static bool Graham::CompareAngle(GPoint A, GPoint B);
};

