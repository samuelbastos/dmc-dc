#include "DMC.h"
#include "DataManager.h"
#include <iostream>
#include <omp.h>

DMC::DMC()
{
}

DMC::~DMC()
{
}


void DMC::run()
{
	/* Read or construct lookup table for DMC */
	table = Table();
	table.construct();

	DataManager::Volume volumeData;
	DataManager::loadRawFile(volumeData, "Bonsai1-HI.raw", 512, 512, 182);
	data = volumeData.data;
	float iso = 0.3;
	gama = iso * std::numeric_limits<uint8_t>::max();

	xLimit = volumeData.dimX;
	yLimit = volumeData.dimY;
	zLimit = volumeData.dimZ;

	dataP = std::vector<int>(data.size(), 0);
	minus = 0;
	plus = 1;

	/* Goes until grid limit - 2 because only
	   wants internal edges.
	*/
	#pragma omp parallel for num_threads(8)
	for (int i = 0; i < xLimit; i++)
	{
		for (unsigned int j = 0; j < yLimit; j++)
		{
			for (unsigned int k = 0; k < zLimit; k++)
			{
				int index = lin(i, j, k);
				int value = minus;
				if (data[index] < gama)
					value = minus;
				else
					value = plus;
				#pragma omp critical
				dataP[index] = value;
			}
		}
	}

	constructQuads();

	DataManager::writeOBJ("mesh.obj", vertexes, quads);
}


void DMC::constructQuads()
{
	int numXProcessed = 0;
	#pragma omp parallel for num_threads(8)
	for (int i = 0; i < xLimit - 1; i++)
	{
		std::cout << "Processado: " << numXProcessed << " " << 
			"Thread: " << omp_get_thread_num() << std::endl;
		for (unsigned int j = 0; j < yLimit - 1; j++)
			for (unsigned int k = 0; k < zLimit - 1; k++)
			{
				/* X = 0, Y = 1, Z = 2*/
				std::vector<bool> bipolar;
				bool bipolarX = j > 0 && k > 0 && dataP[lin(i, j, k)] != dataP[lin(i + 1, j, k)];
				bool bipolarY = i > 0 && k > 0 && dataP[lin(i, j, k)] != dataP[lin(i, j + 1, k)];
				bool bipolarZ = i > 0 && j > 0 && dataP[lin(i, j, k)] != dataP[lin(i, j, k + 1)];
				bipolar = { bipolarX, bipolarY, bipolarZ };

				for (int a = 0; a < 3; a++)
				{
					if (!bipolar[a])
						continue;

					Axis axis = static_cast<Axis>(a);
					bool orientation = getCubeOrientation(axis, i, j, k);

					std::vector<Point> wcms;
					// Pega a configuração dos 4 cubos que possuem a aresta
					auto config = getCubeConfiguration(i, j, k, axis);

					for (unsigned int l = 0; l < 4; l++)
					{
						auto cube = config[l];
						int tableIndex = getTableIndex(cube);

						// checa tabela com a configuração do cubo e a Edge'
						// e adiciona quad

						Table::Config ic = table.getConfigFromIndex(tableIndex);
						auto pointEdges = ic.pes;

						Table::Edge cubeEdge = getCubeEdge(axis, l);
						int pointIndex = getCubePointIndex(ic.pes, cubeEdge);

						// Arestas associadas ao ponto do cubo
						auto edges = pointEdges[pointIndex];
						Voxel param = getCornerZeroParam(axis, l, i, j, k);

						// Constrói os quads
						std::vector<Point> wei;
						for (auto edge : edges)
						{
							auto pq = getCornerFromEdge(param.i, param.j, param.k, edge);
							auto we = wEdge(pq.first, pq.second);
							wei.push_back(we);
						}

						// Calcula vertex
						Point wcm = Point(-1, 0.0, 0.0, 0.0);
						for (unsigned int w = 0; w < wei.size(); w++)
						{
							wcm = wcm + wei[w];
						}

						wcm = wcm / static_cast<int>(wei.size());
						wcms.push_back(wcm);
					}

					// Salva os pontos e os Quads
					#pragma omp critical
					saveQuads(axis, wcms, orientation);
				}
			}

		numXProcessed++;
	}
}

int DMC::getCubePointIndex(Table::PointEdge pes, Table::Edge cubeEdge)
{
	int pointIndex = -1;
	bool found = false;

	for (unsigned int p = 0; p < pes.size(); p++)
	{
		if (!found)
		{
			for (auto edge : pes[p])
				if (edge == cubeEdge)
				{
					pointIndex = p;
					found = true;
					break;
				}
		}
		else
			break;
	}

	return pointIndex;
}

Table::Edge DMC::getCubeEdge(Axis a, int cubeIndex)
{
	Table::Edge cubeEdge;
	switch (a)
	{
	case Axis::X:
		switch (cubeIndex)
		{
		case Cube::One:
			cubeEdge = Table::Edge::EDGE2;
			break;
		case Cube::Two:
			cubeEdge = Table::Edge::EDGE0;
			break;
		case Cube::Three:
			cubeEdge = Table::Edge::EDGE10;
			break;
		case Cube::Four:
			cubeEdge = Table::Edge::EDGE8;
			break;
		}
		break;
	case Axis::Y:
		switch (cubeIndex)
		{
		case Cube::One:
			cubeEdge = Table::Edge::EDGE7;
			break;
		case Cube::Two:
			cubeEdge = Table::Edge::EDGE4;
			break;
		case Cube::Three:
			cubeEdge = Table::Edge::EDGE5;
			break;
		case Cube::Four:
			cubeEdge = Table::Edge::EDGE6;
			break;
		}
		break;
	case Axis::Z:
		switch (cubeIndex)
		{
		case Cube::One:
			cubeEdge = Table::Edge::EDGE9;
			break;
		case Cube::Two:
			cubeEdge = Table::Edge::EDGE11;
			break;
		case Cube::Three:
			cubeEdge = Table::Edge::EDGE1;
			break;
		case Cube::Four:
			cubeEdge = Table::Edge::EDGE3;
			break;
		}
		break;
	}

	return cubeEdge;
}


DMC::Voxel DMC::getCornerZeroParam(Axis a, int cubeIndex, int i, int j, int k)
{
	Voxel param;

	switch (a)
	{
	case Axis::X:
		switch (cubeIndex)
		{
		case Cube::One:
			param = Voxel(i, j, k + 1);
			break;
		case Cube::Two:
			param = Voxel(i, j, k);
			break;
		case Cube::Three:
			param = Voxel(i, j - 1, k + 1);
			break;
		case Cube::Four:
			param = Voxel(i, j - 1, k);
			break;
		}
		break;
	case Axis::Y:
		switch (cubeIndex)
		{
		case Cube::One:
			param = Voxel(i, j, k + 1);
			break;
		case Cube::Two:
			param = Voxel(i, j, k);
			break;
		case Cube::Three:
			param = Voxel(i - 1, j, k);
			break;
		case Cube::Four:
			param = Voxel(i - 1, j, k + 1);
			break;
		}
		break;
	case Axis::Z:
		switch (cubeIndex)
		{
		case Cube::One:
			param = Voxel(i - 1, j - 1, k + 1);
			break;
		case Cube::Two:
			param = Voxel(i, j - 1, k + 1);
			break;
		case Cube::Three:
			param = Voxel(i - 1, j, k + 1);
			break;
		case Cube::Four:
			param = Voxel(i, j, k + 1);
			break;
		}
		break;
	}

	return param;
}

bool DMC::getCubeOrientation(Axis a, int i, int j, int k)
{
	bool ccw = false;

	switch (a)
	{
	case Axis::X:
		if (dataP[lin(i, j, k)] == 0 && dataP[lin(i + 1, j, k)] == 1)
			ccw = true;
		break;
	case Axis::Y:
		if (dataP[lin(i, j, k)] == 0 && dataP[lin(i, j + 1, k)] == 1)
			ccw = true;
		break;
	case Axis::Z:
		if (dataP[lin(i, j, k)] == 0 && dataP[lin(i, j, k + 1)] == 1)
			ccw = true;
		break;
	}

	return ccw;
}

void DMC::saveQuads(Axis a, std::vector<Point> points, bool direction)
{
	// Salva os pontos e os Quads
	int numVertex = vertexes.size();
	for (unsigned int t = 0; t < points.size(); t++)
		vertexes.push_back(points[t]);
	Quad q;
	if (a == Axis::X)
	{
		if (direction)
			q = Quad(numVertex, numVertex + 1,
				numVertex + 3, numVertex + 2);
		else
			q = Quad(numVertex, numVertex + 2,
				numVertex + 3, numVertex + 1);
	}
	else if (a == Axis::Y)
	{

		if (!direction)
			q = Quad(numVertex, numVertex + 1,
				numVertex + 2, numVertex + 3);
		else
			q = Quad(numVertex, numVertex + 3,
				numVertex + 2, numVertex + 1);
	}
	else
	{
		if (!direction)
			q = Quad(numVertex, numVertex + 1,
				numVertex + 3, numVertex + 2);
		else
			q = Quad(numVertex, numVertex + 2,
				numVertex + 3, numVertex + 1);
	}

	quads.push_back(q);
}

//         Cell Edges
//  
//       o--------10---------o
//      /|                  /|
//    11 |                 9 |
//    /  |                /  |
//   o--------8----------o   |
//   |   7               |   6
//   |   |               |   |
//   |   |               |   |
//   4   |               5   |
//   |   o--------2------|---o
//   |  /                |  /
//   | 3                 | 1
//   |/                  |/
//   o--------0----------o
//

// Cell Corners
//
//       7-------------------6
//      /|                  /|
//     / |                 / |
//    /  |                /  |
//   3-------------------2   |
//   |   |               |   |
//   |   |               |   |
//   |   |               |   |
//   |   |               |   |
//   |   4---------------|---5
//   |  /                |  /
//   | /                 | /
//   |/                  |/
//   0-------------------1
//


std::vector<std::vector<unsigned int>> DMC::getCubeConfiguration(unsigned int x,
	unsigned int y, unsigned int z, Axis a)
{
	std::vector<unsigned int> c1;
	std::vector<unsigned int> c2;
	std::vector<unsigned int> c3;
	std::vector<unsigned int> c4;

	switch (a)
	{
	case Axis::X:

		/** CUBE 1 - EDGE 2 - xyz: CORNER 4 */
		c1.push_back(dataP[lin(x, y, z + 1)]);		    // Corner 0
		c1.push_back(dataP[lin(x + 1, y, z + 1)]);      // Corner 1
		c1.push_back(dataP[lin(x + 1, y + 1, z + 1)]);  // Corner 2
		c1.push_back(dataP[lin(x, y + 1, z + 1)]);      // Corner 3
		c1.push_back(dataP[lin(x, y, z)]);              // Corner 4
		c1.push_back(dataP[lin(x + 1, y, z)]);          // Corner 5
		c1.push_back(dataP[lin(x + 1, y + 1, z)]);      // Corner 6
		c1.push_back(dataP[lin(x, y + 1, z)]);          // Corner 7

		/** CUBE 2 - EDGE 0 - xyz: CORNER 0 */
		c2.push_back(dataP[lin(x, y, z)]);		        // Corner 0
		c2.push_back(dataP[lin(x + 1, y, z)]);          // Corner 1
		c2.push_back(dataP[lin(x + 1, y + 1, z)]);      // Corner 2
		c2.push_back(dataP[lin(x, y + 1, z)]);          // Corner 3
		c2.push_back(dataP[lin(x, y, z - 1)]);          // Corner 4
		c2.push_back(dataP[lin(x + 1, y, z - 1)]);      // Corner 5
		c2.push_back(dataP[lin(x + 1, y + 1, z - 1)]);  // Corner 6
		c2.push_back(dataP[lin(x, y + 1, z - 1)]);      // Corner 7

		/** CUBE 3 - EDGE 10 - xyz: CORNER 7 */
		c3.push_back(dataP[lin(x, y - 1, z + 1)]);		// Corner 0
		c3.push_back(dataP[lin(x + 1, y - 1, z + 1)]);	// Corner 1
		c3.push_back(dataP[lin(x + 1, y, z + 1)]);		// Corner 2
		c3.push_back(dataP[lin(x, y, z + 1)]);			// Corner 3
		c3.push_back(dataP[lin(x, y - 1, z)]);			// Corner 4
		c3.push_back(dataP[lin(x + 1, y - 1, z)]);		// Corner 5
		c3.push_back(dataP[lin(x + 1, y, z)]);			// Corner 6
		c3.push_back(dataP[lin(x, y, z)]);				// Corner 7

		/* CUBE 4 - EDGE 8 - Corner 3 */
		c4.push_back(dataP[lin(x, y - 1, z)]);			// Corner 0
		c4.push_back(dataP[lin(x + 1, y - 1, z)]);		// Corner 1
		c4.push_back(dataP[lin(x + 1, y, z)]);			// Corner 2
		c4.push_back(dataP[lin(x, y, z)]);				// Corner 3
		c4.push_back(dataP[lin(x, y - 1, z - 1)]);		// Corner 4
		c4.push_back(dataP[lin(x + 1, y - 1, z - 1)]);	// Corner 5
		c4.push_back(dataP[lin(x + 1, y, z - 1)]);		// Corner 6
		c4.push_back(dataP[lin(x, y, z - 1)]);			// Corner 7

		break;

	case Axis::Y:

		/** CUBE 1 - EDGE 7 - Corner 4 */
		c1.push_back(dataP[lin(x, y, z + 1)]);		    // Corner 0
		c1.push_back(dataP[lin(x + 1, y, z + 1)]);      // Corner 1
		c1.push_back(dataP[lin(x + 1, y + 1, z + 1)]);  // Corner 2
		c1.push_back(dataP[lin(x, y + 1, z + 1)]);      // Corner 3
		c1.push_back(dataP[lin(x, y, z)]);              // Corner 4
		c1.push_back(dataP[lin(x + 1, y, z)]);          // Corner 5
		c1.push_back(dataP[lin(x + 1, y + 1, z)]);      // Corner 6
		c1.push_back(dataP[lin(x, y + 1, z)]);          // Corner 7

		/** CUBE 2 - EDGE 4 - Corner 0 */
		c2.push_back(dataP[lin(x, y, z)]);		        // Corner 0
		c2.push_back(dataP[lin(x + 1, y, z)]);          // Corner 1
		c2.push_back(dataP[lin(x + 1, y + 1, z)]);      // Corner 2
		c2.push_back(dataP[lin(x, y + 1, z)]);          // Corner 3
		c2.push_back(dataP[lin(x, y, z - 1)]);          // Corner 4
		c2.push_back(dataP[lin(x + 1, y, z - 1)]);      // Corner 5
		c2.push_back(dataP[lin(x + 1, y + 1, z - 1)]);  // Corner 6
		c2.push_back(dataP[lin(x, y + 1, z - 1)]);      // Corner 7

		/** CUBE 3 - EDGE 5 - Corner 1 */
		c3.push_back(dataP[lin(x - 1, y, z)]);			// Corner 0
		c3.push_back(dataP[lin(x, y, z)]);				// Corner 1
		c3.push_back(dataP[lin(x, y + 1, z)]);			// Corner 2
		c3.push_back(dataP[lin(x - 1, y + 1, z)]);		// Corner 3
		c3.push_back(dataP[lin(x - 1, y, z - 1)]);		// Corner 4
		c3.push_back(dataP[lin(x, y, z - 1)]);			// Corner 5
		c3.push_back(dataP[lin(x, y + 1, z - 1)]);		// Corner 6
		c3.push_back(dataP[lin(x - 1, y + 1, z - 1)]);	// Corner 7

		/* CUBE 4 - EDGE 6 - Corner 5 */
		c4.push_back(dataP[lin(x - 1, y, z + 1)]);		// Corner 0
		c4.push_back(dataP[lin(x, y, z + 1)]);			// Corner 1
		c4.push_back(dataP[lin(x, y + 1, z + 1)]);		// Corner 2
		c4.push_back(dataP[lin(x - 1, y + 1, z + 1)]);	// Corner 3
		c4.push_back(dataP[lin(x - 1, y, z)]);			// Corner 4
		c4.push_back(dataP[lin(x, y, z)]);				// Corner 5
		c4.push_back(dataP[lin(x, y + 1, z)]);			// Corner 6
		c4.push_back(dataP[lin(x - 1, y + 1, z)]);		// Corner 7

		break;

	case Axis::Z:

		/** CUBE 1 - EDGE 9 - Corner 6 */
		c1.push_back(dataP[lin(x - 1, y - 1, z + 1)]);	// Corner 0
		c1.push_back(dataP[lin(x, y - 1, z + 1)]);      // Corner 1
		c1.push_back(dataP[lin(x, y, z + 1)]);			// Corner 2
		c1.push_back(dataP[lin(x - 1, y, z + 1)]);      // Corner 3
		c1.push_back(dataP[lin(x - 1, y - 1, z)]);		// Corner 4
		c1.push_back(dataP[lin(x, y - 1, z)]);			// Corner 5
		c1.push_back(dataP[lin(x, y, z)]);				// Corner 6
		c1.push_back(dataP[lin(x - 1, y, z)]);			// Corner 7

		/** CUBE 2 - EDGE 11 - Corner 7 */
		c2.push_back(dataP[lin(x, y - 1, z + 1)]);		// Corner 0
		c2.push_back(dataP[lin(x + 1, y - 1, z + 1)]);  // Corner 1
		c2.push_back(dataP[lin(x + 1, y, z + 1)]);		// Corner 2
		c2.push_back(dataP[lin(x, y, z + 1)]);			// Corner 3
		c2.push_back(dataP[lin(x, y - 1, z)]);		    // Corner 4
		c2.push_back(dataP[lin(x + 1, y - 1, z)]);      // Corner 5
		c2.push_back(dataP[lin(x + 1, y, z)]);			// Corner 6
		c2.push_back(dataP[lin(x, y, z)]);				// Corner 7

		/** CUBE 3 - EDGE 1 - Corner 5 */
		c3.push_back(dataP[lin(x - 1, y, z + 1)]);		// Corner 0
		c3.push_back(dataP[lin(x, y, z + 1)]);			// Corner 1
		c3.push_back(dataP[lin(x, y + 1, z + 1)]);		// Corner 2
		c3.push_back(dataP[lin(x - 1, y + 1, z + 1)]);  // Corner 3
		c3.push_back(dataP[lin(x - 1, y, z)]);		    // Corner 4
		c3.push_back(dataP[lin(x, y, z)]);				// Corner 5
		c3.push_back(dataP[lin(x, y + 1, z)]);			// Corner 6
		c3.push_back(dataP[lin(x - 1, y + 1, z)]);      // Corner 7

		/* CUBE 4 - EDGE 3 - Corner 4 */
		c4.push_back(dataP[lin(x, y, z + 1)]);		    // Corner 0
		c4.push_back(dataP[lin(x + 1, y, z + 1)]);      // Corner 1
		c4.push_back(dataP[lin(x + 1, y + 1, z + 1)]);  // Corner 2
		c4.push_back(dataP[lin(x, y + 1, z + 1)]);      // Corner 3
		c4.push_back(dataP[lin(x, y, z)]);				// Corner 4
		c4.push_back(dataP[lin(x + 1, y, z)]);			// Corner 5
		c4.push_back(dataP[lin(x + 1, y + 1, z)]);		// Corner 6
		c4.push_back(dataP[lin(x, y + 1, z)]);			// Corner 7

		break;
	}

	std::vector<std::vector<unsigned int>> c;
	c.push_back(c1);
	c.push_back(c2);
	c.push_back(c3);
	c.push_back(c4);

	return c;
}


int DMC::getTableIndex(std::vector<unsigned int> cubeConfig)
{
	int index = 0;
	for (unsigned int i = 0; i < 8; i++)
		if (cubeConfig[i] == 1)
			index |= static_cast<int>(powf(2, i));

	return index;
}

Point DMC::wEdge(Voxel p, Voxel q)
{
	uint8_t sp = data[lin(p.i, p.j, p.k)];
	uint8_t sq = data[lin(q.i, q.j, q.k)];

	Point pp = Point(-1, p.i, p.j, p.k);
	Point qq = Point(-1, q.i, q.j, q.k);

	float alpha = ((float)gama - (float)sp) / ((float)sq - (float)sp);
	Point n = pp * (1 - alpha) + qq * alpha;

	return n;
}

// Tem sempre como parametro o Corner0 = ijk
std::pair<DMC::Voxel, DMC::Voxel> DMC::getCornerFromEdge(int i, int j, int k, Table::Edge edge)
{
	std::pair<Voxel, Voxel> edgea;
	Voxel p, q;
	switch (edge)
	{
	case Table::Edge::EDGE0:
		p = Voxel(i, j, k);
		q = Voxel(i + 1, j, k);
		break;
	case Table::Edge::EDGE1:
		p = Voxel(i + 1, j, k - 1);
		q = Voxel(i + 1, j, k);
		break;
	case Table::Edge::EDGE2:
		p = Voxel(i, j, k - 1);
		q = Voxel(i + 1, j, k - 1);
		break;
	case Table::Edge::EDGE3:
		p = Voxel(i, j, k - 1);
		q = Voxel(i, j, k);
		break;


	case Table::Edge::EDGE5:
		p = Voxel(i + 1, j, k);
		q = Voxel(i + 1, j + 1, k);
		break;
	case Table::Edge::EDGE6:
		p = Voxel(i + 1, j, k - 1);
		q = Voxel(i + 1, j + 1, k - 1);
		break;
	case Table::Edge::EDGE7:
		p = Voxel(i, j, k - 1);
		q = Voxel(i, j + 1, k - 1);
		break;
	case Table::Edge::EDGE4:
		p = Voxel(i, j, k);
		q = Voxel(i, j + 1, k);
		break;


	case Table::Edge::EDGE8:
		p = Voxel(i, j + 1, k);
		q = Voxel(i + 1, j + 1, k);
		break;
	case Table::Edge::EDGE9:
		p = Voxel(i + 1, j + 1, k - 1);
		q = Voxel(i + 1, j + 1, k);
		break;
	case Table::Edge::EDGE10:
		p = Voxel(i, j + 1, k - 1);
		q = Voxel(i + 1, j + 1, k - 1);
		break;
	case Table::Edge::EDGE11:
		p = Voxel(i, j + 1, k - 1);
		q = Voxel(i, j + 1, k);
		break;
	}
	edgea.first = p;
	edgea.second = q;
	return edgea;
}