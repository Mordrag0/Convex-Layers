#pragma once

#include <vector>
#include "Point.h"

class QuickHull
{
public:
	QuickHull();
	~QuickHull();

	// Tvorba konveksne lupine. 
	std::vector<Point> QHull();

	std::vector<std::vector<Point>> GetConvexLayers(std::vector<Point> Points);


private:
	std::vector<Point> Points;
	std::vector<Point> NotHull;

	// Funkcija poi��e to�ke konveksne lupine na podani podmno�ici to�k.
	std::vector<Point> FindHull(std::vector<Point> SK, Point P, Point Q);

	// Izra�un razdalje med daljico L1-L2 in to�ko T. 
	double Distance(Point L1, Point L2, Point T) const;
	// Predznak rezultata nam pove na kateri strani daljice L1-L2 je to�ka T.
	double Side(Point L1, Point L2, Point T) const;
};

