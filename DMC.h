#pragma once
#include <vector>
#include "Quad.h"
#include "Table.h"
#include "Point.h"

class DMC
{

public:

	DMC();
	~DMC();
	void run();

	struct Voxel
	{
		Voxel() {}
		Voxel(int ii, int jj, int kk)
		{
			i = ii;
			j = jj;
			k = kk;
		}
		int i;
		int j;
		int k;
	};

private:

	enum Axis
	{
		X = 0,
		Y,
		Z,
	};

	enum Cube
	{
		One = 0,
		Two,
		Three,
		Four,
	};


	inline
		unsigned int lin(unsigned int const x,
								   unsigned int const y,
								   unsigned int const z) 
								   const {
		return x + xLimit * (y + yLimit * z);
	};

	void constructQuads();

	std::vector<std::vector<unsigned int>> getCubeConfiguration(unsigned int x,
							  unsigned int y, unsigned int z, Axis a);

	int getTableIndex(std::vector<unsigned int> cubeConfig);

	Point wEdge(Voxel p, Voxel q);

	std::pair<Voxel, Voxel> getCornerFromEdge(int i, int j, int k, Table::Edge edge);

	void saveQuads(Axis a, std::vector<Point> points, bool direction);

	bool getCubeOrientation(Axis a, int i, int j, int k);

	Table::Edge getCubeEdge(Axis a, int cubeIndex);

	Voxel getCornerZeroParam(Axis a, int cubeIndex, int i, int j, int k);

	int getCubePointIndex(Table::PointEdge pes, Table::Edge cubeEdge);

	/** voxel data */
	std::vector<uint8_t> data;
	std::vector<int> dataP;

	/** gama value */
	uint8_t gama;

	/* Grid limits */
	unsigned int xLimit;
	unsigned int yLimit;
	unsigned int zLimit;

	/** Vector of mesh faces */
	std::vector<Quad> quads;
	std::vector<Point> vertexes;

	/** Lookup table */
	Table table;
};