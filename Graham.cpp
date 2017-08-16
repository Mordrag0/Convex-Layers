#include "Graham.h"
#include <algorithm>

Graham::Graham()
{
}


Graham::~Graham()
{
}

std::vector<std::vector<Point>> Graham::GetConvexLayers(std::vector<Point> CPoints)
{
	// Vsem toèkam dodaj kot.
	Points.resize(CPoints.size());
	std::transform(CPoints.begin(), CPoints.end(), Points.begin(), [](Point P) { return GPoint(P); });

	std::vector<std::vector<Point>> Layers;
	while (Points.size() > 0)
	{
		if (Points.size() <= 3)
		{
			std::vector<Point> LastLayer;
			for (const GPoint& P : Points)
			{
				LastLayer.push_back(P.Loc);
			}
			Layers.push_back(LastLayer);
			Points.clear();
		}
		else
		{
			std::vector<Point> Layer = Scan(); // Zunanja lupina.
			Layers.push_back(Layer);
		}
	}
	return Layers;
}

bool Graham::CompareLoc(GPoint A, GPoint B)
{
	return (A.Loc.Y < B.Loc.Y) || ((std::abs(A.Loc.Y - B.Loc.Y) < std::numeric_limits<double>::epsilon()) && (A.Loc.X > B.Loc.X));
}
bool Graham::CompareAngle(GPoint A, GPoint B)
{
	return A.Angle < B.Angle;
}

std::vector<Point> Graham::Scan()
{
	// Poišèi P0 in uredi ostale toèke po kotu z P0.
	auto MinIt = std::min_element(Points.begin(), Points.end(), &Graham::CompareLoc);
	P0 = *MinIt;
	Points.erase(MinIt);
	for (auto &P : Points)
	{
		P.Angle = P.Loc.PseudoAngle(P0.Loc);
	}
	std::sort(Points.begin(), Points.end(), &Graham::CompareAngle);
	auto It = Points.begin() + 1;
	while (It != Points.end())
	{
		GPoint A = *It;
		GPoint B = *(It - 1);
		if ((std::abs(A.Angle - B.Angle) < std::numeric_limits<double>::epsilon()))
		{
			if (A.Loc.Y < B.Loc.Y)
			{
				Inner.push_back(*It);
				It = Points.erase(It);
			}
			else
			{
				It = Points.erase(It - 1);
			}
		}
		else
		{
			It++;
		}
	}

	// Dodaj zaèetni toèki.
	std::vector<GPoint> Stack;
	Stack.push_back(P0);
	Stack.push_back(Points[0]);

	// Glavna zanka.
	for (auto It = Points.begin() + 1; It != Points.end(); )
	{
		if (Stack.size() < 2)
		{
			Stack.push_back(*It);
			It++;
			continue;
		}
		GPoint P = *(Stack.end() - 2);
		GPoint Q = *(Stack.end() - 1);
		GPoint R = *It;
		double Det = (Q.Loc.X - P.Loc.X) * (R.Loc.Y - P.Loc.Y) - (Q.Loc.Y - P.Loc.Y) * (R.Loc.X - P.Loc.X);
		if (Det > 0)
		{
			Stack.push_back(R);
			++It;
		}
		else
		{
			Inner.push_back(Stack.back());
			Stack.pop_back();
		}
	}

	// Toèke znotraj konveksne lupine nastavi za naslendje iteracije.
	Points = std::move(Inner);
	Inner.clear();

	std::vector<Point> Ret;
	Ret.resize(Stack.size());
	std::transform(Stack.begin(), Stack.end(), Ret.begin(), [](GPoint P) { return P.Loc; });

	return Ret;
}
