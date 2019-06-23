#include "DataManager.h"
#include <iostream>
#include <fstream>

DataManager::DataManager()
{
}


DataManager::~DataManager()
{
}

/// Load volume from raw file.
bool DataManager::loadRawFile(Volume& volume, std::string const & fileName, int32_t dimX, int32_t dimY, int32_t dimZ)
{
	// check provided dimensions
	if (dimX < 1 || dimY < 1 || dimZ < 1) {
		std::cerr << "Invalid RAW file dimensions specified" << std::endl;
		return false;
	}

	// open raw file
	std::ifstream file(fileName, std::ifstream::binary);
	if (!file) {
		std::cerr << "Unable to open file '" << fileName << "'" << std::endl;
		return false;
	}

	// check consistency of file size and volume dimensions
	size_t const expectedFileSize = size_t(dimX) * size_t(dimY) * size_t(dimZ);
	file.seekg(0, file.end);
	size_t const fileSize = file.tellg();
	file.seekg(0, file.beg);

	if (expectedFileSize != fileSize) {
		if (expectedFileSize * 2 == fileSize) {
			std::cout << "Assuming 16-bit RAW file" << std::endl;
			volume.bitDepth = 16;
		}
		else {
			std::cerr << "File size inconsistent with specified dimensions" << std::endl;
			return false;
		}
	}
	else {
		volume.bitDepth = 8;
	}

	//
	if (expectedFileSize >= 0xffffffffu) {
		std::cerr << "Too many voxels. Please improve the dual mc implementation." << std::endl;
		return false;
	}

	// initialize volume dimensions and memory
	volume.dimX = dimX;
	volume.dimY = dimY;
	volume.dimZ = dimZ;
	volume.data.resize(fileSize);

	// read data
	file.read((char*)&volume.data[0], fileSize);

	if (!file) {
		std::cerr << "Error while reading file" << std::endl;
		return false;
	}

	return true;
}

/// Write a Wavefront OBJ model for the extracted ISO surface.
void DataManager::writeOBJ(std::string const & fileName, std::vector<Point> vertices, std::vector<Quad> quads)
{
	std::cout << "Writing OBJ file" << std::endl;
	// check if we actually have an ISO surface
	if (vertices.size() == 0 || quads.size() == 0) {
		std::cout << "No ISO surface generated. Skipping OBJ generation." << std::endl;
		return;
	}

	// open output file
	std::ofstream file(fileName);
	if (!file) {
		std::cout << "Error opening output file" << std::endl;
		return;
	}

	std::cout << "Generating OBJ mesh with " << vertices.size() << " vertices and "
		<< quads.size() << " quads" << std::endl;

	// write vertices
	for (auto const & v : vertices) {
		file << "v " << v.getX() << ' ' << v.getY() << ' ' << v.getZ() << '\n';
	}

	// write quad indices
	for (auto const & q : quads) {
		file << "f " << (q.i1 + 1) << ' ' << (q.i2 + 1) << ' ' << (q.i3 + 1) << ' ' << (q.i4 + 1) << '\n';
	}

	file.close();
}