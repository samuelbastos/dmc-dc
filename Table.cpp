#include "Table.h"
#include <iostream>
#include <fstream>
#include <algorithm>

Table::Table()
{
}

Table::~Table()
{
}

void Table::construct()
{
	if (!readTable())
		constructTable();

}

Table::Config Table::getConfigFromIndex(int index)
{
	return table[index];
}

void Table::constructTable()
{
	constructBaseConfigs();

	table = std::vector<Config>(256, Config());

	for (int i = 0; i < 256; i++)
	{
		if (i == 138)
			std::cout << "dsa";
        
		// 1 - Pegar quantidade de corners positivos Q;
		std::vector<int> corners;
		int qtd = 0;
		for (int k = 0; k < 8; k++)
		{
			int kvalue = nbit(i, k);
			if (kvalue)
				qtd++;
			corners.push_back(kvalue);
		}

		CornerConfig newCornerConfig;
		std::vector<int> rotated; // newCornerConfigINT
		PointEdge newPointEdges;
		int ccValue = -99;

		// 2 - Com Q, apenas acessar as configurações necessárias, o conjunto Z;
		for (unsigned int cc = 0; cc < base[qtd].size(); cc++)
		{
			std::vector<int> configurationCorners(8, 0);
			auto a = base[qtd][cc].config;
			for (unsigned int i = 0; i < a.size(); i++)
			{
				int value = a[i];
				configurationCorners[value] = 1;
			}

			// 3 - foreach Z : rotacionar e ver se encaixa com alguma configuração base (obrigatoriamente vai achar um);
			bool isSameConfig = false;
			rotated = configurationCorners;
			std::vector<Rotation> rotations;
			bool xEdgeRotation = false;
			bool yEdgeRotation = false;
			bool zEdgeRotation = false;

			// Y-EDGES ROTATION
			for (int ry = 0; ry < 4; ry++)
			{
				isSameConfig = sameConfig(rotated, corners);
				rotated = rotateY(rotated);
				if (isSameConfig)
				{
					for (int ryy = 0; ryy < ry; ryy++)
						rotations.push_back(Rotation::YR);
					break;
				}
			}

			if (!isSameConfig)
			{
				rotations.clear();
				rotated = configurationCorners;
				for (int rx = 0; rx < 2; rx++)
				{
					rotations.push_back(Rotation::XR);
					rotated = rotateX(rotated);
				}

				for (int ry = 0; ry < 4; ry++)
				{
					isSameConfig = sameConfig(rotated, corners);
					rotated = rotateY(rotated);
					if (isSameConfig)
					{
						for (int ryy = 0; ryy < ry; ryy++)
							rotations.push_back(Rotation::YR);
						break;
					}
				}
			}

			if (isSameConfig)
				yEdgeRotation = true;

			// X-EDGES ROTATION - Só executa se as primeiras configs
			//					  não deram certo.
			if (!yEdgeRotation)
			{
				rotations.clear();
				rotated = configurationCorners;

				rotated = rotateZ(rotated);
				rotations.push_back(Rotation::ZR);

				for (int rx = 0; rx < 4; rx++)
				{
					isSameConfig = sameConfig(rotated, corners);
					rotated = rotateX(rotated);
					if (isSameConfig)
					{
						for (int rxx = 0; rxx < rx; rxx++)
							rotations.push_back(Rotation::XR);
						break;
					}
				}

				if (!isSameConfig)
				{
					rotations.clear();
					rotated = configurationCorners;

					for (int rz = 0; rz < 3; rz++)
					{
						rotations.push_back(Rotation::ZR);
						rotated = rotateZ(rotated);
					}

					for (int rx = 0; rx < 4; rx++)
					{
						isSameConfig = sameConfig(rotated, corners);
						rotated = rotateX(rotated);
						if (isSameConfig)
						{
							for (int rxx = 0; rxx < rx; rxx++)
								rotations.push_back(Rotation::XR);
							break;
						}
					}
				}

				if (isSameConfig)
					xEdgeRotation = true;
			}


			// Z-EDGES ROTATION

			if (!xEdgeRotation && !yEdgeRotation)
			{
				rotations.clear();
				rotated = configurationCorners;

				rotated = rotateX(rotated);
				rotations.push_back(Rotation::XR);

				for (int rz = 0; rz < 4; rz++)
				{
					isSameConfig = sameConfig(rotated, corners);
					rotated = rotateZ(rotated);
					if (isSameConfig)
					{
						for (int rzz = 0; rzz < rz; rzz++)
							rotations.push_back(Rotation::ZR);
						break;
					}
				}

				if (!isSameConfig)
				{
					rotations.clear();
					rotated = configurationCorners;

					for (int rx = 0; rx < 3; rx++)
					{
						rotated = rotateX(rotated);
						rotations.push_back(Rotation::XR);
					}

					for (int rz = 0; rz < 4; rz++)
					{
						isSameConfig = sameConfig(rotated, corners);
						rotated = rotateZ(rotated);
						if (isSameConfig)
						{
							for (int rzz = 0; rzz < rz; rzz++)
								rotations.push_back(Rotation::ZR);
							break;
						}
					}
				}

				if (isSameConfig)
					zEdgeRotation = true;
			}

			if (!isSameConfig)
				continue;

			// 4 - Quando achar a configuração Zmatch, fazer traceback de rotações tendo a configuração Zmatch como base
			//     para achar as Edges de cada ponto de cube(i);

			auto points = base[qtd][cc].pes;
			auto pointsSize = points.size();
			
			points = getRotatedConfig(rotations, points);

			// Achou a configuração, sai do loop
			newPointEdges = points;
			ccValue = cc;

			break;
		}

		// 5 - Guardar essa configuração;
		newCornerConfig = convert(corners);
		Config newConfig = Config(newCornerConfig, newPointEdges);
		newConfig.configA = qtd;
		newConfig.configB = ccValue;
		table[i] = newConfig;
	}

	std::vector<int> problematicConfig;
	for (unsigned int i = 1; i < 255; i++)
	{
		if (table[i].pes.size() == 0)
			problematicConfig.push_back(i);
	}

	constructBadConfigs();

	problematicConfig.clear();
	for (unsigned int i = 1; i < 255; i++)
	{
		if (table[i].pes.size() == 0)
			problematicConfig.push_back(i);
	}

	writeTable();
}

void Table::constructBadConfigs()
{
	/* ENTRY 23 */
	std::vector<int> corners = { 0,0,0,1,0,1,1,1 };
	std::reverse(corners.begin(), corners.end());
	CornerConfig newCornerConfig = convert(corners);

	PointEdge newPointEdges = { {Edge::EDGE1, Edge::EDGE2, Edge::EDGE4, Edge::EDGE7,
								Edge::EDGE8, Edge::EDGE9} };
	Config newConfig = Config(newCornerConfig, newPointEdges);
	newConfig.configA = 4;
	newConfig.configB = 3;
	table[23] = newConfig;

	/* Entry 46 */
	corners = { 0,0,1,0,1,1,1,0 };
	std::reverse(corners.begin(), corners.end());
	newCornerConfig = convert(corners);
	newPointEdges = { {Edge::EDGE0, Edge::EDGE2, Edge::EDGE4, Edge::EDGE6,
								Edge::EDGE9, Edge::EDGE11} };
	newConfig = Config(newCornerConfig, newPointEdges);
	newConfig.configA = 4;
	newConfig.configB = 3;
	table[46] = newConfig;

	/* Entry 57 */
	corners = { 0,0,1,1,1,0,0,1};
	std::reverse(corners.begin(), corners.end());
	newCornerConfig = convert(corners);
	newPointEdges = { {Edge::EDGE0, Edge::EDGE1, Edge::EDGE6, Edge::EDGE7,
								Edge::EDGE8, Edge::EDGE11} };
	newConfig = Config(newCornerConfig, newPointEdges);
	newConfig.configA = 4;
	newConfig.configB = 3;
	table[57] = newConfig;

	/* Entry 77 */
	corners = { 0,1,0,0,1,1,0,1 };
	std::reverse(corners.begin(), corners.end());
	newCornerConfig = convert(corners);
	newPointEdges = { {Edge::EDGE0, Edge::EDGE3, Edge::EDGE5, Edge::EDGE6,
								Edge::EDGE10, Edge::EDGE11} };
	newConfig = Config(newCornerConfig, newPointEdges);
	newConfig.configA = 4;
	newConfig.configB = 3;
	table[77] = newConfig;

	/* Entry 99 */
	corners = { 0,1,1,0,0,0,1,1 };
	std::reverse(corners.begin(), corners.end());
	newCornerConfig = convert(corners);
	newPointEdges = { {Edge::EDGE2, Edge::EDGE3, Edge::EDGE4, Edge::EDGE5,
								Edge::EDGE9, Edge::EDGE10} };
	newConfig = Config(newCornerConfig, newPointEdges);
	newConfig.configA = 4;
	newConfig.configB = 3;
	table[99] = newConfig;

	/* Entry 116 */
	corners = { 0,1,1,1,0,1,0,0 };
	std::reverse(corners.begin(), corners.end());
	newCornerConfig = convert(corners);
	newPointEdges = { {Edge::EDGE1, Edge::EDGE3, Edge::EDGE5, Edge::EDGE7,
								Edge::EDGE8, Edge::EDGE10} };
	newConfig = Config(newCornerConfig, newPointEdges);
	newConfig.configA = 4;
	newConfig.configB = 3;
	table[116] = newConfig;

	/* Entry 139 */
	corners = { 1,0,0,0,1,0,1,1 };
	std::reverse(corners.begin(), corners.end());
	newCornerConfig = convert(corners);
	newPointEdges = { {Edge::EDGE1, Edge::EDGE3, Edge::EDGE5, Edge::EDGE7,
								Edge::EDGE8, Edge::EDGE10} };
	newConfig = Config(newCornerConfig, newPointEdges);
	newConfig.configA = 4;
	newConfig.configB = 3;
	table[139] = newConfig;

	/* Entry 156 */
	corners = { 1,0,0,1,1,1,0,0 };
	std::reverse(corners.begin(), corners.end());
	newCornerConfig = convert(corners);
	newPointEdges = { {Edge::EDGE2, Edge::EDGE3, Edge::EDGE4, Edge::EDGE5,
								Edge::EDGE9, Edge::EDGE10} };
	newConfig = Config(newCornerConfig, newPointEdges);
	newConfig.configA = 4;
	newConfig.configB = 3;
	table[156] = newConfig;

	/* Entry 178 */
	corners = { 1,0,1,1,0,0,1,0};
	std::reverse(corners.begin(), corners.end());
	newCornerConfig = convert(corners);
	newPointEdges = { {Edge::EDGE0, Edge::EDGE3, Edge::EDGE5, Edge::EDGE6,
								Edge::EDGE10, Edge::EDGE11} };
	newConfig = Config(newCornerConfig, newPointEdges);
	newConfig.configA = 4;
	newConfig.configB = 3;
	table[178] = newConfig;

	/* Entry 198 */
	corners = { 1,1,0,0,0,1,1,0};
	std::reverse(corners.begin(), corners.end());
	newCornerConfig = convert(corners);
	newPointEdges = { {Edge::EDGE0, Edge::EDGE1, Edge::EDGE6, Edge::EDGE7,
								Edge::EDGE8, Edge::EDGE11} };
	newConfig = Config(newCornerConfig, newPointEdges);
	newConfig.configA = 4;
	newConfig.configB = 3;
	table[198] = newConfig;

	/* Entry 209 */
	corners = { 1,1,0,1,0,0,0,1 };
	std::reverse(corners.begin(), corners.end());
	newCornerConfig = convert(corners);
	newPointEdges = { {Edge::EDGE0, Edge::EDGE2, Edge::EDGE4, Edge::EDGE6,
								Edge::EDGE9, Edge::EDGE11} };
	newConfig = Config(newCornerConfig, newPointEdges);
	newConfig.configA = 4;
	newConfig.configB = 3;
	table[209] = newConfig;

	/* Entry 232 */
	corners = { 1,1,1,0,1,0,0,0 };
	std::reverse(corners.begin(), corners.end());
	newCornerConfig = convert(corners);
	newPointEdges = { {Edge::EDGE1, Edge::EDGE2, Edge::EDGE4, Edge::EDGE7,
								Edge::EDGE8, Edge::EDGE9} };
	newConfig = Config(newCornerConfig, newPointEdges);
	newConfig.configA = 4;
	newConfig.configB = 3;
	table[232] = newConfig;
}

Table::PointEdge Table::getRotatedConfig(std::vector<Rotation> v, PointEdge points)
{
	PointEdge aux = points;

	for (unsigned int i = 0; i < v.size(); i++)
	{
		PointEdge auxIn = aux;
		PointEdge newPoints;
		for (unsigned int p = 0; p < points.size(); p++)
		{
			auto edges = auxIn[p];
			std::vector<Edge> newEdges;
			for (auto edge : edges)
			{
				Edge ne;
				if(v[i] == Rotation::XR)
					ne = rxEdge(edge);
				else if(v[i] == Rotation::YR)
					ne = ryEdge(edge);
				else
					ne = rzEdge(edge);
				newEdges.push_back(ne);
			}
			newPoints.push_back(newEdges);
		}
		aux = newPoints;
	}
	return aux;
}

std::string Table::configNToLetter(int n)
{
	switch (n)
	{
	case 0:
		return "A";
	case 1:
		return "B";
	case 2:
		return "C";
	case 3:
		return "D";
	case 4:
		return "E";
	case 5:
		return "F";
	}

	return "null";
}

// EXEMPLO DE ENTRADA
// Config: 0
// { 0, 7, 3 } { 2, 4, 6 }
// Template: 1-A

void Table::writeTable()
{
	std::ofstream tablefile;
	tablefile.open("table.txt");

	for (unsigned int i = 1; i < table.size() - 1; i++)
	{
		auto tableConfig = table[i].config;
		auto tablePoints = table[i].pes;

		tablefile << tableConfig.size() << " ";

		// Corners
		for (unsigned int j = 0; j < tableConfig.size(); j++)
			tablefile << tableConfig[j] << " ";

		tablefile << std::endl;

		// Points/Edges
		tablefile << tablePoints.size() << " ";

		for (unsigned int p = 0; p < tablePoints.size(); p++)
		{
			tablefile << tablePoints[p].size() << " ";

			for (unsigned int e = 0; e < tablePoints[p].size(); e++)
				tablefile << tablePoints[p][e] << " ";		
		}

		// Template
		tablefile << std::endl;
		tablefile << table[i].configA << " " << table[i].configB;
		tablefile << std::endl << std::endl;
	}

	tablefile.close();
}

bool Table::readTable()
{
	std::ifstream tablefile;
	tablefile.open("table.txt");

	if (tablefile.fail())
		return false;
	
	std::vector<Config> readTable;

	// First entry
	PointEdge pointedge = std::vector<std::vector<Edge>>();
	CornerConfig config = std::vector<Corner>();
	Config zero = Config(config, pointedge);
	zero.configA = 0;
	zero.configB = 0;
	readTable.push_back(zero);

	for (int i = 1; i < 255; i++)
	{
		int qtd;
		tablefile >> qtd;
		
		// Corner config
		std::vector<Corner> corners;
		for (int j = 0; j < qtd; j++)
		{
			int corner;
			tablefile >> corner;
			corners.push_back(static_cast<Corner>(corner));
		}

		//  Point-Edges
		int qtdPoint;
		tablefile >> qtdPoint;
		
		PointEdge pe;
		for (int j = 0; j < qtdPoint; j++)
		{
			int qtdEdge;
			tablefile >> qtdEdge;

			std::vector<Edge> edges;
			for (int k = 0; k < qtdEdge; k++)
			{
				int edge;
				tablefile >> edge;
				edges.push_back(static_cast<Edge>(edge));
			}

			pe.push_back(edges);
		}

		int ca, cb;
		tablefile >> ca;
		tablefile >> cb;

		Config ci = Config(corners, pe);
		ci.configA = ca;
		ci.configB = cb;
		readTable.push_back(ci);
	}

	// Last entry
	config = { Corner::C0, Corner::C1, Corner::C2, Corner::C3,
			   Corner::C4, Corner::C5, Corner::C6, Corner::C7 };

	Config eight = Config(config, pointedge);
	eight.configA = 8;
	eight.configB = 0;
	readTable.push_back(eight);

	table = readTable;

	return true;
}

int Table::nbit(int value, int k)
{
	return ((value & (1 << k)) >> k);
}

bool Table::sameConfig(std::vector<int> v1, std::vector<int> v2)
{
	auto size = v1.size();
	for (unsigned int i = 0; i < size; i++)
	{
		if (v1[i] != v2[i])
			return false;
	}
	return true;
}

std::vector<Table::Corner> Table::convert(std::vector<int> v)
{
	std::vector<Table::Corner> vn;
	for (unsigned int i = 0; i < v.size(); i++)
	{
		if (v[i] == 1)
			vn.push_back(static_cast<Corner>(i));
	}
	return vn;
}

std::vector<int> Table::rotateZ(std::vector<int> v)
{
	std::vector<int> nv(8, 0);
	for (unsigned int i = 0; i < v.size(); i++)
	{
		int value = v[i];
		int rotatedValue = rzCorner(i);
		nv[rotatedValue] = value;
	}
	return nv;
}

std::vector<int> Table::rotateY(std::vector<int> v)
{
	std::vector<int> nv(8, 0);
	for (unsigned int i = 0; i < v.size(); i++)
	{
		int value = v[i];
		int rotatedValue = ryCorner(i);
		nv[rotatedValue] = value;
	}
	return nv;
}

std::vector<int> Table::rotateX(std::vector<int> v)
{
	std::vector<int> nv(8, 0);
	for (unsigned int i = 0; i < v.size(); i++)
	{
		int value = v[i];
		int rotatedValue = rxCorner(i);
		nv[rotatedValue] = value;
	}
	return nv;
}

void Table::constructBaseConfigs()
{
	//std::vector<Config> base;

	// Cell Edges
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

	for (int i = 0; i < 9; i++)
		base.push_back(std::vector<Config>());

	/* Zero - 0 *//////////////////////////////////////////////////////////////
	Config c0 = Config(std::vector<Corner>(), std::vector<std::vector<Edge>>());
	base[0].push_back(c0);
	c0.config = {};

	/* One - 1 *///////////////////////////////////////////////////////////////
	Config c1 = Config(std::vector<Corner>(), std::vector<std::vector<Edge>>());
	std::vector<Edge> p0({ Edge::EDGE4, Edge::EDGE8, Edge::EDGE11 });
	c1.pes.push_back(p0);
	c1.config = { Corner::C3 };
	base[1].push_back(c1);


	/* Two - 2A *///////////////////////////////////////////////////////////////
	Config c2A = Config(std::vector<Corner>(), std::vector<std::vector<Edge>>());
	p0 = std::vector<Edge>({ Edge::EDGE5, Edge::EDGE6, Edge::EDGE8, Edge::EDGE10 });
	c2A.pes.push_back(p0);
	c2A.config = { Corner::C2, Corner::C6 };

	/* Two - 2B */
	Config c2B = Config(std::vector<Corner>(), std::vector<std::vector<Edge>>());
	p0 = std::vector<Edge>({ Edge::EDGE5, Edge::EDGE8, Edge::EDGE7,
						   Edge::EDGE10, Edge::EDGE9, Edge::EDGE11 });
	c2B.pes.push_back(p0);
	c2B.config = { Corner::C2, Corner::C7 };

	/* Two - 2C */
	Config c2C = Config(std::vector<Corner>(), std::vector<std::vector<Edge>>());
	p0 = std::vector<Edge>({ Edge::EDGE6, Edge::EDGE9, Edge::EDGE10 });
	std::vector<Edge> p1({ Edge::EDGE0, Edge::EDGE3, Edge::EDGE4 });
	c2C.pes.push_back(p0);
	c2C.pes.push_back(p1);

	c2C.config = { Corner::C0, Corner::C6 };

	base[2].push_back(c2A);
	base[2].push_back(c2B);
	base[2].push_back(c2C);



	/* Three - 3A */////////////////////////////////////////////////////////////
	Config c3A = Config(std::vector<Corner>(), std::vector<std::vector<Edge>>());
	p0 = std::vector<Edge>({ Edge::EDGE4, Edge::EDGE7, Edge::EDGE6, Edge::EDGE8, Edge::EDGE9 });
	c3A.pes.push_back(p0);
	c3A.config = { Corner::C3, Corner::C6, Corner::C7 };

	/* Three - 3B */
	Config c3B = Config(std::vector<Corner>(), std::vector<std::vector<Edge>>());
	p0 = std::vector<Edge>({ Edge::EDGE11, Edge::EDGE10, Edge::EDGE9,
						   Edge::EDGE8, Edge::EDGE0, Edge::EDGE1, Edge::EDGE7 });
	c3B.pes.push_back(p0);
	c3B.config = { Corner::C1, Corner::C2, Corner::C7 };

	/* Three - 3C */
	Config c3C = Config(std::vector<Corner>(), std::vector<std::vector<Edge>>());
	p0 = std::vector<Edge>({ Edge::EDGE0, Edge::EDGE1, Edge::EDGE4,
					   Edge::EDGE6, Edge::EDGE11, Edge::EDGE10 });
	p1 = std::vector<Edge>({ Edge::EDGE8, Edge::EDGE9, Edge::EDGE5 });
	c3C.pes.push_back(p0);
	c3C.pes.push_back(p1);
	c3C.config = { Corner::C1, Corner::C6, Corner::C3 };

	base[3].push_back(c3A);
	base[3].push_back(c3B);
	base[3].push_back(c3C);



	/* Four - 4A *////////////////////////////////////////////////////////////////
	Config c4A = Config(std::vector<Corner>(), std::vector<std::vector<Edge>>());
	p0 = std::vector<Edge>({ Edge::EDGE5, Edge::EDGE6, Edge::EDGE7,
					   Edge::EDGE4 });
	c4A.pes.push_back(p0);

	c4A.config = { Corner::C2, Corner::C3, Corner::C6, Corner::C7 };

	/* Four - 4B */
	Config c4B = Config(std::vector<Corner>(), std::vector<std::vector<Edge>>());
	p0 = std::vector<Edge>({ Edge::EDGE0, Edge::EDGE1, Edge::EDGE4,
					   Edge::EDGE6, Edge::EDGE11, Edge::EDGE10 });
	c4B.pes.push_back(p0);

	c4B.config = { Corner::C1, Corner::C2, Corner::C3, Corner::C6 };

	/* Four - 4C */
	Config c4C = Config(std::vector<Corner>(), std::vector<std::vector<Edge>>());
	p0 = std::vector<Edge>({ Edge::EDGE11, Edge::EDGE10, Edge::EDGE2,Edge::EDGE3 });
	p1 = std::vector<Edge>({ Edge::EDGE0, Edge::EDGE1, Edge::EDGE8,Edge::EDGE9 });
	c4C.pes.push_back(p0);
	c4C.pes.push_back(p1);

	c4C.config = { Corner::C0, Corner::C3, Corner::C5, Corner::C6 };

	/* Four - 4D */
	Config c4D = Config(std::vector<Corner>(), std::vector<std::vector<Edge>>());
	p0 = std::vector<Edge>({ Edge::EDGE1, Edge::EDGE3, Edge::EDGE6, Edge::EDGE4, Edge::EDGE8, Edge::EDGE10 });
	c4D.pes.push_back(p0);

	c4D.config = { Corner::C0, Corner::C1, Corner::C2, Corner::C6 };

	/* Four - 4E */
	Config c4E = Config(std::vector<Corner>(), std::vector<std::vector<Edge>>());
	p0 = std::vector<Edge>({ Edge::EDGE1, Edge::EDGE2, Edge::EDGE6 });
	p1 = std::vector<Edge>({ Edge::EDGE5, Edge::EDGE4, Edge::EDGE7, Edge::EDGE9, Edge::EDGE10 });
	c4E.pes.push_back(p0);
	c4E.pes.push_back(p1);

	c4E.config = { Corner::C0, Corner::C1, Corner::C4, Corner::C6 };

	/* Four - 4F */
	Config c4F = Config(std::vector<Corner>(), std::vector<std::vector<Edge>>());
	p0 = std::vector<Edge>({ Edge::EDGE6, Edge::EDGE9, Edge::EDGE10 });
	p1 = std::vector<Edge>({ Edge::EDGE0, Edge::EDGE1, Edge::EDGE5 });
	std::vector<Edge> p2({ Edge::EDGE4, Edge::EDGE8, Edge::EDGE11 });
	std::vector<Edge> p3({ Edge::EDGE3, Edge::EDGE2, Edge::EDGE7 });
	c4F.pes.push_back(p0);
	c4F.pes.push_back(p1);
	c4F.pes.push_back(p2);
	c4F.pes.push_back(p3);
	c4F.config = { Corner::C0, Corner::C5, Corner::C2, Corner::C7 };

	base[4].push_back(c4A);
	base[4].push_back(c4B);
	base[4].push_back(c4C);
	base[4].push_back(c4D);
	base[4].push_back(c4E);
	base[4].push_back(c4F);



	/* Five - 5A *////////////////////////////////////////////////////////////////
	Config c5A = Config(std::vector<Corner>(), std::vector<std::vector<Edge>>());
	p0 = std::vector<Edge>({ Edge::EDGE6, Edge::EDGE7, Edge::EDGE4, Edge::EDGE8, Edge::EDGE9 });
	c5A.pes.push_back(p0);

	c5A.config = { Corner::C0, Corner::C1, Corner::C4, Corner::C5, Corner::C2 };

	/* Five - 5B */
	Config c5B = Config(std::vector<Corner>(), std::vector<std::vector<Edge>>());
	p0 = std::vector<Edge>({ Edge::EDGE11, Edge::EDGE10, Edge::EDGE7 });
	p1 = std::vector<Edge>({ Edge::EDGE0, Edge::EDGE1, Edge::EDGE8, Edge::EDGE9 });
	c5B.pes.push_back(p0);
	c5B.pes.push_back(p1);

	c5B.config = { Corner::C0, Corner::C3, Corner::C5, Corner::C6, Corner::C4 };

	/* Five - 5C */
	Config c5C = Config(std::vector<Corner>(), std::vector<std::vector<Edge>>());
	p0 = std::vector<Edge>({ Edge::EDGE4, Edge::EDGE8, Edge::EDGE11 });
	p1 = std::vector<Edge>({ Edge::EDGE6, Edge::EDGE9, Edge::EDGE10 });
	p2 = std::vector<Edge>({ Edge::EDGE0, Edge::EDGE1, Edge::EDGE5 });
	c5C.pes.push_back(p0);
	c5C.pes.push_back(p1);
	c5C.pes.push_back(p2);
	c5C.config = { Corner::C0, Corner::C4, Corner::C5, Corner::C2, Corner::C7 };

	base[5].push_back(c5A);
	base[5].push_back(c5B);
	base[5].push_back(c5C);


	/* Six - 6A *///////////////////////////////////////////////////////////////////
	Config c6A = Config(std::vector<Corner>(), std::vector<std::vector<Edge>>());
	p0 = std::vector<Edge>({ Edge::EDGE5, Edge::EDGE8, Edge::EDGE6, Edge::EDGE10 });
	c6A.pes.push_back(p0);

	c6A.config = { Corner::C0, Corner::C1, Corner::C3, Corner::C4, Corner::C5, Corner::C7 };

	/* Six - 6B */
	Config c6B = Config(std::vector<Corner>(), std::vector<std::vector<Edge>>());
	p0 = std::vector<Edge>({ Edge::EDGE7, Edge::EDGE10, Edge::EDGE11 });
	p1 = std::vector<Edge>({ Edge::EDGE8, Edge::EDGE9, Edge::EDGE5 });
	c6B.pes.push_back(p0);
	c6B.pes.push_back(p1);

	c6B.config = { Corner::C0, Corner::C1, Corner::C3, Corner::C4, Corner::C5, Corner::C6 };

	/* Six - 6C */
	Config c6C = Config(std::vector<Corner>(), std::vector<std::vector<Edge>>());
	p0 = std::vector<Edge>({ Edge::EDGE0, Edge::EDGE3, Edge::EDGE4 });
	p1 = std::vector<Edge>({ Edge::EDGE6, Edge::EDGE9, Edge::EDGE10 });
	c6C.pes.push_back(p0);
	c6C.pes.push_back(p1);
	c6C.config = { Corner::C1, Corner::C2, Corner::C3, Corner::C4, Corner::C5, Corner::C7 };

	base[6].push_back(c6A);
	base[6].push_back(c6B);
	base[6].push_back(c6C);



	/* Seven - 7 */////////////////////////////////////////////////////////////////
	Config c7 = Config(std::vector<Corner>(), std::vector<std::vector<Edge>>());
	p0 = std::vector<Edge>({ Edge::EDGE4, Edge::EDGE8, Edge::EDGE11 });
	c7.pes.push_back(p0);
	c7.config = { Corner::C0, Corner::C1, Corner::C2, Corner::C4, Corner::C5, Corner::C6, Corner::C7 };
	base[7].push_back(c7);



	/* Eight - 8 *////////////////////////////////////////////////////////////////
	Config c8 = Config(std::vector<Corner>(), std::vector<std::vector<Edge>>());
	c8.config = { Corner::C0, Corner::C1, Corner::C2, Corner::C3, Corner::C4, Corner::C5, Corner::C6, Corner::C7 };
	base[8].push_back(c8);
}

Table::Corner Table::rxCorner(int c)
{
	Corner cn;
	switch (c)
	{
	case Corner::C0:
		cn = Corner::C3;
		break;
	case Corner::C1:
		cn = Corner::C2;
		break;
	case Corner::C2:
		cn = Corner::C6;
		break;
	case Corner::C3:
		cn = Corner::C7;
		break;
	case Corner::C4:
		cn = Corner::C0;
		break;
	case Corner::C5:
		cn = Corner::C1;
		break;
	case Corner::C6:
		cn = Corner::C5;
		break;
	case Corner::C7:
		cn = Corner::C4;
		break;
	}
	return cn;
}

Table::Corner Table::ryCorner(int c)
{
	Corner cn;
	switch (c)
	{
	case Corner::C0:
		cn = Corner::C1;
		break;
	case Corner::C1:
		cn = Corner::C5;
		break;
	case Corner::C2:
		cn = Corner::C6;
		break;
	case Corner::C3:
		cn = Corner::C2;
		break;
	case Corner::C4:
		cn = Corner::C0;
		break;
	case Corner::C5:
		cn = Corner::C4;
		break;
	case Corner::C6:
		cn = Corner::C7;
		break;
	case Corner::C7:
		cn = Corner::C3;
		break;
	}
	return cn;
}

Table::Corner Table::rzCorner(int c)
{
	Corner cn;
	switch (c)
	{
	case Corner::C0:
		cn = Corner::C1;
		break;
	case Corner::C1:
		cn = Corner::C2;
		break;
	case Corner::C2:
		cn = Corner::C3;
		break;
	case Corner::C3:
		cn = Corner::C0;
		break;
	case Corner::C4:
		cn = Corner::C5;
		break;
	case Corner::C5:
		cn = Corner::C6;
		break;
	case Corner::C6:
		cn = Corner::C7;
		break;
	case Corner::C7:
		cn = Corner::C4;
		break;
	}
	return cn;
}

Table::Edge Table::rxEdge(Edge e)
{
	Edge en;
	switch (e)
	{
	case Edge::EDGE0:
		en = Edge::EDGE8;
		break;
	case Edge::EDGE1:
		en = Edge::EDGE5;
		break;
	case Edge::EDGE2:
		en = Edge::EDGE0;
		break;
	case Edge::EDGE3:
		en = Edge::EDGE4;
		break;
	case Edge::EDGE4:
		en = Edge::EDGE11;
		break;
	case Edge::EDGE5:
		en = Edge::EDGE9;
		break;
	case Edge::EDGE6:
		en = Edge::EDGE1;
		break;
	case Edge::EDGE7:
		en = Edge::EDGE3;
		break;
	case Edge::EDGE8:
		en = Edge::EDGE10;
		break;
	case Edge::EDGE9:
		en = Edge::EDGE6;
		break;
	case Edge::EDGE10:
		en = Edge::EDGE2;
		break;
	case Edge::EDGE11:
		en = Edge::EDGE7;
		break;
	}
	return en;
}

Table::Edge Table::ryEdge(Edge e)
{
	Edge en;
	switch (e)
	{
	case Edge::EDGE0:
		en = Edge::EDGE1;
		break;
	case Edge::EDGE1:
		en = Edge::EDGE2;
		break;
	case Edge::EDGE2:
		en = Edge::EDGE3;
		break;
	case Edge::EDGE3:
		en = Edge::EDGE0;
		break;
	case Edge::EDGE4:
		en = Edge::EDGE5;
		break;
	case Edge::EDGE5:
		en = Edge::EDGE6;
		break;
	case Edge::EDGE6:
		en = Edge::EDGE7;
		break;
	case Edge::EDGE7:
		en = Edge::EDGE4;
		break;
	case Edge::EDGE8:
		en = Edge::EDGE9;
		break;
	case Edge::EDGE9:
		en = Edge::EDGE10;
		break;
	case Edge::EDGE10:
		en = Edge::EDGE11;
		break;
	case Edge::EDGE11:
		en = Edge::EDGE8;
		break;
	}
	return en;
}

Table::Edge Table::rzEdge(Edge e)
{
	Edge en;
	switch (e)
	{
	case Edge::EDGE0:
		en = Edge::EDGE5;
		break;
	case Edge::EDGE1:
		en = Edge::EDGE9;
		break;
	case Edge::EDGE2:
		en = Edge::EDGE6;
		break;
	case Edge::EDGE3:
		en = Edge::EDGE1;
		break;
	case Edge::EDGE4:
		en = Edge::EDGE0;
		break;
	case Edge::EDGE5:
		en = Edge::EDGE8;
		break;
	case Edge::EDGE6:
		en = Edge::EDGE10;
		break;
	case Edge::EDGE7:
		en = Edge::EDGE2;
		break;
	case Edge::EDGE8:
		en = Edge::EDGE4;
		break;
	case Edge::EDGE9:
		en = Edge::EDGE11;
		break;
	case Edge::EDGE10:
		en = Edge::EDGE7;
		break;
	case Edge::EDGE11:
		en = Edge::EDGE3;
		break;
	}
	return en;
}