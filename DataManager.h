#pragma once
#include <string>
#include <vector>

#include "Point.h"
#include "Quad.h"

class DataManager
{
public:
	DataManager();
	~DataManager();

	/// struct for volume data information
	struct Volume {
		// volume grid extents
		int32_t dimX;
		int32_t dimY;
		int32_t dimZ;
		// bit depth, should be 8 or 16
		int32_t bitDepth;
		/// volume data
		//Integer type with a width of exactly 8, 16, 32, or 64 bits.
		std::vector<uint8_t> data;
	};

	/// Load volume from raw file.
	static bool loadRawFile(Volume& volume, std::string const & fileName, int32_t dimX, int32_t dimY, int32_t dimZ);

	/// Write a Wavefront OBJ model for the extracted ISO surface.
	static void writeOBJ(std::string const & fileName, std::vector<Point> vertices, std::vector<Quad> quads);
};