#pragma once

#include <vector>
#include "Point.h"

class Jarvis
{
public:
	Jarvis();
	~Jarvis();

	// Tvorba ene izboèene lupine. 
	std::vector<Point> March();

	std::vector<std::vector<Point>> GetConvexLayers(std::vector<Point> Points);

private:
	std::vector<Point> Points;

	Point P0;
	Point START;

	// Poišèe prvi P0. 
	void Init();

	float LastAngle;

	Point GetNextPointCCW(Point Current, bool bErase);
};

