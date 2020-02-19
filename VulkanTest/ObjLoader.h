#pragma once
#include <vector>
#include <string>
#include <fstream>
#include <stdexcept>
#include <sstream>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

class ObjLoader
{
public:
	ObjLoader();
	//load obj
	bool loadObj(std::string path);
	//load mtl
	bool loadMtl(std::string path);
	//load texture
	bool loadTexture(std::string path);
	~ObjLoader();
	struct VertexIndicies {
		long v, n, t;
	};

private:
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> uvcoords;
	std::vector<VertexIndicies> faceVertices;
};

