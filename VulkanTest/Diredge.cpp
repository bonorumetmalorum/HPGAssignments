#include <iostream>
#include <fstream>
#include <vector>
#include <math.h>

#include "Diredge.h"

using namespace std;
using namespace diredge;

diredgeMesh diredge::createMesh(OBJ& raw_vertices)
{
	diredgeMesh mesh;
	//std::vector<Vertex*> rawPositions;
	//std::vector<glm::vec3*> normals;

	//for (auto i = raw_vertices.indices.begin(); i != raw_vertices.indices.end(); ++i)
	//{
	//	rawPositions.push_back(&raw_vertices.vertexList[*i]);
	//	normals.push_back(&raw_vertices.vertexList[*i].normal);
	//}

	mesh.faceVertices.resize(raw_vertices.indices.size(), -1);
	//mesh.normal.resize(rawPositions.size() / 3, glm::vec3(0.0, 0.0, 0.0));
	makeFaceIndices(raw_vertices, mesh);

	mesh.otherHalf.resize(mesh.faceVertices.size(), NO_SUCH_ELEMENT);
	mesh.firstDirectedEdge.resize(mesh.position.size(), NO_SUCH_ELEMENT);
	makeDirectedEdges(mesh);

	return mesh;
}

void diredge::makeFaceIndices(OBJ & raw, diredgeMesh& mesh)
{
	// set the initial vertex ID
	long nextVertexID = 0;

	// loop through the vertices
	for (unsigned long vertex = 0; vertex < raw.indices.size(); vertex++)
	{ // vertex loop
		// first see if the vertex already exists
		for (unsigned long other = 0; other < vertex; other++)
		{ // per other
			if (raw.vertexList[raw.indices[vertex]].position == raw.vertexList[raw.indices[other]].position)
				mesh.faceVertices[vertex] = mesh.faceVertices[other];
		} // per other
		// if not set, set to next available
		if (mesh.faceVertices[vertex] == -1)
			mesh.faceVertices[vertex] = nextVertexID++;
	} // vertex loop

	// id of next vertex to write
	long writeID = 0;

	for (long vertex = 0; vertex < raw.indices.size(); vertex++)
	{
		// if it's the first time found
		if (writeID == mesh.faceVertices[vertex])
		{
			mesh.position.push_back(raw.indices[vertex]);
			writeID++;
		}
	}
}

void diredge::makeDirectedEdges(diredgeMesh& mesh)
{
	// we will also want a temporary variable for the degree of each vertex
	std::vector<long> vertexDegree(mesh.position.size(), 0);

	// 2.	now loop through the directed edges
	for (long dirEdge = 0; dirEdge < (long)mesh.faceVertices.size(); dirEdge++)
	{ // for each directed edge
		// a. retrieve to and from vertices
		long from = mesh.faceVertices[dirEdge];
		long to = mesh.faceVertices[NEXT_EDGE(dirEdge)];

		// aa. Error check for duplicated vertices on faces
		if (from == to)
		{ // error: duplicate vertex on face
			printf("Error: Directed Edge %ld has matching ends %ld %ld\n", dirEdge, from, to);
			exit(0);
		} // error: duplicate vertex on face

		// b. if from vertex has no "first", set it
		if (mesh.firstDirectedEdge[from] == NO_SUCH_ELEMENT)
			mesh.firstDirectedEdge[from] = dirEdge;

		// increment the vertex degree
		vertexDegree[from]++;

		// c. if the other half is already set, we can skip this edge
		if (mesh.otherHalf[dirEdge] != NO_SUCH_ELEMENT)
			continue;

		// d. set a counter for how many matching edges
		long nMatches = 0;

		// e. now search all directed edges on higher index faces
		long face = dirEdge / 3;
		for (long otherEdge = 3 * (face + 1); otherEdge < (long)mesh.faceVertices.size(); otherEdge++)
		{ // for each higher face edge
			// i. retrieve other's to and from
			long otherFrom = mesh.faceVertices[otherEdge];
			long otherTo = mesh.faceVertices[NEXT_EDGE(otherEdge)];

			// ii. test for match
			if ((from == otherTo) && (to == otherFrom))
			{ // match
				// if it's not the first match, we have a non-manifold edge
				if (nMatches > 0)
				{ // non-manifold edge
					printf("Error: Directed Edge %ld matched more than one other edge (%ld, %ld)\n", dirEdge, mesh.otherHalf[dirEdge], otherEdge);
					exit(0);
				} // non-manifold edge

				// otherwise we set the two edges to point to each other
				mesh.otherHalf[dirEdge] = otherEdge;
				mesh.otherHalf[otherEdge] = dirEdge;

				// increment the counter
				nMatches++;
			} // match

		} // for each higher face edge

		// f. if it falls through here with no matches, we know it is non-manifold
		if (nMatches == 0)
		{ // non-manifold edge
			printf("Error: Directed Edge %ld (%ld, %ld) matched no other edge\n", dirEdge, from, to);
			exit(0);
		} // non-manifold edge

	} // for each other directed edge

	// 3.	now we assume that the data structure is correctly set, and test whether all neighbours are on a single cycle
	for (long vertex = 0; vertex < (long)mesh.position.size(); vertex++)
	{ // for each vertex
		// start a counter for cycle length
		long cycleLength = 0;

		// loop control is the neighbouring edge
		long outEdge = mesh.firstDirectedEdge[vertex];

		// could happen in a malformed input
		if (outEdge == NO_SUCH_ELEMENT)
		{ // no first edge
			printf("Error: Vertex %ld had not incident edges\n", vertex);
		} // no first edge

		// do loop to iterate correctly
		do
		{ // do loop
			// while we are at it, we can set the normal
			long face = outEdge / 3;
			// flip to the other half
			long edgeFlip = mesh.otherHalf[outEdge];
			// take the next edge on its face
			outEdge = NEXT_EDGE(edgeFlip);
			// increment the cycle length
			cycleLength++;

		} // do loop
		while (outEdge != mesh.firstDirectedEdge[vertex]);

		// now check the length against the vertex degree
		if (cycleLength != vertexDegree[vertex])
		{ // wrong cycle length
			printf("Error: vertex %ld has edge cycle of length %ld but degree of %ld\n", vertex, cycleLength, vertexDegree[vertex]);
			exit(0);
		} // wrong cycle length
	} // for each vertex
}

void diredge::createLineAdjacency(OBJ& model, diredgeMesh & mesh)
{
	for (long edge = 0; edge < mesh.faceVertices.size(); edge++)
	{
		//do first the previous
		long previous;
		uint32_t previousVertID;
		//get the other half
		long otherhalf = mesh.otherHalf[edge];
		//get the previous of the other half
		previous = PREVIOUS_EDGE(otherhalf);
		previousVertID = mesh.position[mesh.faceVertices[previous]];
		model.adjacencyIndices.push_back(previousVertID);
		//then do the current two
		uint32_t currentID, nextID;
		//vertex
		currentID = mesh.position[mesh.faceVertices[edge]];
		//then do the next
		nextID = mesh.position[mesh.faceVertices[NEXT_EDGE(edge)]];
		model.adjacencyIndices.push_back(currentID);
		model.adjacencyIndices.push_back(nextID);
		//next of next vertex
		uint32_t nextNextID = mesh.position[mesh.faceVertices[NEXT_EDGE(NEXT_EDGE(edge))]];
		model.adjacencyIndices.push_back(nextNextID);
	}
}
