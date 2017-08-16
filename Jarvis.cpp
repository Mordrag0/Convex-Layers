#include "Jarvis.h"


Jarvis::Jarvis()
{
}


Jarvis::~Jarvis()
{
}

std::vector<Point> Jarvis::March()
{
	std::vector<Point> Layer;
	START = GetNextPointCCW(P0, false);
	Layer.push_back(START);

	Point Next = GetNextPointCCW(START, true);
	while (Next != START)
	{
		Layer.push_back(Next);
		P0 = Next;
		Next = GetNextPointCCW(Next, true);
	}
	return Layer;
}

std::vector<std::vector<Point>> Jarvis::GetConvexLayers(std::vector<Point> CPoints)
{
	Points = CPoints;
	Init();
	std::vector<std::vector<Point>> Layers;
	while (Points.size() > 0)
	{
		if (Points.size() <= 3)
		{
			Layers.push_back(Points);
			Points.clear();
		}
		else
		{
			std::vector<Point> Layer = March();
			Layers.push_back(Layer);
		}
	}
	return Layers;
}

void Jarvis::Init()
{
	float MinX = std::numeric_limits<float>::max();
	float MinY = std::numeric_limits<float>::max();

	for (const Point P : Points)
	{
		if (P.X < MinX)
		{
			MinX = P.X;
		}
		if (P.Y < MinY)
		{
			MinY = P.Y;
		}
	}
	P0 = Point(MinX, MinY);
	LastAngle = 0;
}

Point Jarvis::GetNextPointCCW(Point Current, bool bErase)
{
	float MinAngle = std::numeric_limits<float>::max();
	int Idx = 0;
	int NextIdx;

	for (auto It = Points.begin(); It != Points.end(); It++)
	{
		float Angle = Current.PseudoAngle(*It);
		Angle = Angle - LastAngle;
		if (Angle < 0)
		{
			Angle += 4;
		}
		if (Angle < MinAngle)
		{
			MinAngle = Angle;
			NextIdx = Idx;
		}
		Idx++;
	}
	Point Next = Points[NextIdx];
	LastAngle = Current.PseudoAngle(Next);
	if (bErase)
	{
		Points.erase(Points.begin() + NextIdx);
	}
	return Next;
}
