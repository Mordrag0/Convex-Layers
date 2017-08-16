#pragma once
#include <limits>
/** Razred za 2D toèke. */
struct Point
{
public:
	double X;
	double Y;
	Point() : X(std::numeric_limits<double>::lowest()), Y(std::numeric_limits<double>::lowest())
	{
	}
	Point(double X, double Y) : X(X), Y(Y)
	{
	}
	~Point()
	{
	}

	/** Overload operatorjev za sortiranje z std::sort*/
	inline bool operator<(const Point& Other) const
	{
		if (std::abs(X - Other.X) < std::numeric_limits<double>::epsilon() * 10000)
		{
			return Y < Other.Y;
		}
		return X < Other.X;
	}
	inline bool operator>(const Point& Other) const
	{
		if (std::abs(X - Other.X) < std::numeric_limits<double>::epsilon() * 10000)
		{
			return std::abs(Y) > std::abs(Other.Y);
		}
		return X > Other.X;
	}

	inline bool operator==(const Point& Other) const
	{
		return (std::abs(X - Other.X) < std::numeric_limits<double>::epsilon()) && (std::abs(Y - Other.Y) < std::numeric_limits<double>::epsilon());
	}
	inline bool operator!=(const Point& Other) const
	{
		return !(*this == Other);
	}

	inline Point operator+(const Point& Other) const
	{
		return Point(X + Other.X, Y + Other.Y);
	}
	inline Point operator-(const Point& Other) const
	{
		return Point(X - Other.X, Y - Other.Y);
	}

	double PseudoAngle(const Point& Other) const
	{
		Point Delta = Other - *this;
		double angle = std::copysign(1. - Delta.X / (abs(Delta.X) + abs(Delta.Y)), Delta.Y);
		return angle + 2.0f;
	}

	double Slope(const Point& Other) const
	{
		return (Other.Y - Y) / (Other.X - X);
	}
};
