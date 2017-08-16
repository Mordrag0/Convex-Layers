#include "QuickHull.h"
#include <algorithm>

QuickHull::QuickHull()
{
}

QuickHull::~QuickHull()
{
}

std::vector<Point> QuickHull::QHull()
{
	if (Points.size() <= 3)
	{
		std::vector<Point> Hull = Points;
		Points.clear();
		NotHull.clear();
		return Hull;
	}
	auto Pair = std::minmax_element(Points.begin(), Points.end());
	Point A = *Pair.first;
	Point B = *Pair.second;

	std::vector<Point> S1, S2;
	for (std::vector<Point>::const_iterator It = Points.begin(); It != Points.end(); ++It)
	{
		if (It == Pair.first || It == Pair.second)
		{
			continue;
		}
		
		double S = Side(A, B, *It);
		if ( S > 0)
		{
			S1.push_back(*It);
		}
		else
		{
			S2.push_back(*It);
		}
	}

	NotHull.clear();
	S1 = FindHull(S1, A, B);
	S2 = FindHull(S2, B, A);

	std::vector<Point> Hull;
	Hull.reserve(S1.size() + S2.size() + 2);
	Hull.push_back(A);
	Hull.insert(Hull.end(), S1.begin(), S1.end());
	Hull.push_back(B);
	Hull.insert(Hull.end(), S2.begin(), S2.end());


	Points = NotHull;
	return Hull;
}

std::vector<Point> QuickHull::FindHull(std::vector<Point> SK, Point P, Point Q)
{
	if (SK.size() == 0)
	{
		return std::vector<Point>();
	}
	double MaxDist = -std::numeric_limits<double>::infinity();
	std::vector<Point>::iterator ItMax;
	for (auto It = SK.begin(); It != SK.end(); ++It)
	{
		double D = Distance(P, Q, *It);
		if (D > MaxDist)
		{
			ItMax = It;
			MaxDist = D;
		}
	}
	Point C = *ItMax;
	SK.erase(ItMax);
	
	std::vector<Point> S1, S2;
	for (auto It = SK.begin(); It != SK.end(); ++It)
	{
		bool bIsOnLeftSide = Side(P, C, *It) > 0;
		bool bIsOnRightSide = Side(C, Q, *It) > 0;
		if (bIsOnLeftSide)
		{
			S1.push_back(*It);
		}
		else if (bIsOnRightSide)
		{
			S2.push_back(*It);
		}
		else
		{
			NotHull.push_back(*It);
		}
	}

	std::vector<Point> HS1 = FindHull(S1, P, C);
	std::vector<Point> HS2 = FindHull(S2, C, Q);

	std::vector<Point> Hull;
	Hull.reserve(HS1.size() + HS2.size() + 1);
	Hull.insert(Hull.end(), HS1.begin(), HS1.end());
	Hull.push_back(C);
	Hull.insert(Hull.end(), HS2.begin(), HS2.end());

	return Hull;
}

std::vector<std::vector<Point>> QuickHull::GetConvexLayers(std::vector<Point> CPoints)
{
	Points = CPoints;
	std::vector<std::vector<Point>> Layers;
	while (Points.size() > 0)
	{
		std::vector<Point> Layer = QHull();
		Layers.push_back(Layer);
	}
	return Layers;
}

double QuickHull::Distance(Point L1, Point L2, Point T) const
{
	return std::abs((L2.Y - L1.Y) * T.X - (L2.X - L1.X) * T.Y + L2.X * L1.Y - L2.Y * L1.X);
}

double QuickHull::Side(Point L1, Point L2, Point T) const
{
	return (L2.X - L1.X) * (T.Y - L1.Y) - (L2.Y - L1.Y) * (T.X - L1.X);
}
