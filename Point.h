#pragma once
#include <math.h>

class Point
{
public:
	Point();
	Point(int iindex, float xx, float yy, float zz);
	~Point();

	inline void set(float xx, float yy, float zz) { x = xx; y = yy;  z = zz; }
	inline float getX() const { return x; }
	inline float getY() const { return y; }
	inline float getZ() const { return z; }
	inline int getIndex() const { return index; }
	inline void  setFixed(int f) { fixed = f; }
	inline int getFixed() { return  fixed; }
	inline static float distance(Point p1, Point p2)
	{
		float xx = (p1.x - p2.x) * (p1.x - p2.x);
		float yy = (p1.y - p2.y) * (p1.y - p2.y);
		float zz = (p1.z - p2.z) * (p1.z - p2.z);
		return sqrt(xx + yy + zz);
	}

private:

	int index;
	float x;
	float y;
	float z;
	int fixed;
};

inline Point operator-(const Point& p1, const Point& p2)
{
	float x = p1.getX() - p2.getX();
	float y = p1.getY() - p2.getY();
	float z = p1.getZ() - p2.getZ();

	return Point(-1, x, y, z);
}

inline Point operator+(const Point& p1, const Point& p2)
{
	float x = p2.getX() + p1.getX();
	float y = p2.getY() + p1.getY();
	float z = p2.getZ() + p1.getZ();

	if (isnan(x) || isnan(y) || isnan(z))
		return Point(-1, x, y, z);

	return Point(-1, x, y, z);
}

inline Point operator*(const Point& p1, const Point& p2)
{
	float x = p2.getX() * p1.getX();
	float y = p2.getY() * p1.getY();
	float z = p2.getZ() * p1.getZ();

	return Point(-1, x, y, z);
}

inline Point operator*(const Point& p1, float n)
{
	float x = p1.getX() * n;
	float y = p1.getY() * n;
	float z = p1.getZ() * n;

	return Point(-1, x, y, z);
}

inline Point operator*(const Point& p1, int n)
{
	float x = p1.getX() * n;
	float y = p1.getY() * n;
	float z = p1.getZ() * n;

	return Point(-1, x, y, z);
}

inline Point operator/(const Point& p1, const Point& p2)
{
	float x = p1.getX() / p2.getX();
	float y = p1.getY() / p2.getY();
	float z = p1.getZ() / p2.getZ();

	return Point(-1, x, y, z);
}

inline Point operator/(const Point& p1, int n)
{
	float x = p1.getX() / n;
	float y = p1.getY() / n;
	float z = p1.getZ() / n;

	if (isnan(x) || isnan(y) || isnan(z))
		return Point(-1, x, y, z);

	return Point(-1, x, y, z);
}

inline Point operator/(const Point& p1, float n)
{
	float x = p1.getX() / n;
	float y = p1.getY() / n;
	float z = p1.getZ() / n;

	return Point(-1, x, y, z);
}