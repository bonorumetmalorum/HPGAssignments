#pragma once

#include <string>
#include <vector>
#include <glm/vec3.hpp>
#include <glm/geometric.hpp>
#include "ObjLoader.h"

// define a macro for "not used" flag
#define NO_SUCH_ELEMENT -1

// use macros for the "previous" and "next" IDs
#define PREVIOUS_EDGE(x) ((x) % 3) ? ((x) - 1) : ((x) + 2)
#define NEXT_EDGE(x) (((x) % 3) == 2) ? ((x) - 2) : ((x) + 1)
struct OBJ;

namespace diredge
{
	struct diredgeMesh
	{
		std::vector<glm::vec3> normal;
		std::vector<uint32_t> position;

		std::vector<long> faceVertices;
		std::vector<long> otherHalf;
		std::vector<long> firstDirectedEdge;

	};

	// Makes a half edge mesh data structure from a triangle soup.
	diredgeMesh createMesh(OBJ& obj);

	// Computes mesh.position and mesh.normal and mesh.faceVertices
	void makeFaceIndices(OBJ & raw, diredgeMesh&);

	// Computes mesh.firstDirectedEdge and mesh.firstDirectedEdge, given mesh.position and mesh.normal and mesh.faceVertices
	void makeDirectedEdges(diredgeMesh& mesh);

	void createLineAdjacency(OBJ& model, diredgeMesh & mesh);
}
