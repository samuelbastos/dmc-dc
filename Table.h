#pragma once
#include <vector>
#include <string>

class Table
{

public:

	enum Edge
	{
		EDGE0,
		EDGE1,
		EDGE2,
		EDGE3,
		EDGE4,
		EDGE5,
		EDGE6,
		EDGE7,
		EDGE8,
		EDGE9,
		EDGE10,
		EDGE11,
	};

	enum Corner
	{
		C0 = 0,
		C1,
		C2,
		C3,
		C4,
		C5,
		C6,
		C7,
	};

	enum Rotation
	{
		XR = 0,
		YR,
		ZR,
	};

	using PointEdge = std::vector<std::vector<Edge>>;
	using CornerConfig = std::vector<Corner>;
	struct Config
	{
		Config() {}
		Config(CornerConfig vconfig, PointEdge vpes)
		{
			config = vconfig;
			pes = vpes;
		}
		CornerConfig config;
		PointEdge pes;
		int configA;
		int configB;
	};

	Table();
	~Table();
	void construct();
	Config getConfigFromIndex(int index);
private:

	void constructTable();
	void constructBaseConfigs();
	int nbit(int value, int k);
	bool sameConfig(std::vector<int> v1, std::vector<int> v2);
	std::vector<Corner> convert(std::vector<int> v);

	std::vector<int> rotateZ(std::vector<int> v);
	std::vector<int> rotateY(std::vector<int> v);
	std::vector<int> rotateX(std::vector<int> v);

	Corner rxCorner(int c);
	Corner ryCorner(int c);
	Corner rzCorner(int c);

	Edge   rxEdge(Edge e);
	Edge   ryEdge(Edge e);
	Edge   rzEdge(Edge e);

	PointEdge getRotatedConfig(std::vector<Rotation> v, PointEdge points);
	void constructBadConfigs();

	std::string configNToLetter(int n);
	void writeTable();
	bool readTable();

	std::vector<std::vector<Config>> base;
	std::vector<Config> table;
};