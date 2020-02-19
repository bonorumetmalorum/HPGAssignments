#include "ObjLoader.h"



ObjLoader::ObjLoader()
{
}

bool ObjLoader::loadObj(std::string path)
{
	std::fstream inStream(path.data(), std::ios::in);
	if (!inStream.is_open) {
		throw std::runtime_error("unable to open file");
	}
	std::string currentLine = "";
	std::string mtlPath;
	while (inStream.peek() != -1) {
		std::getline(inStream, currentLine);
		std::istringstream stream(currentLine);
		std::string type;
		stream >> type;
		if (type == "#") continue; //check if comment, ignore
		else if (type == "mtllib") {		//get path filename of mtllib if it declared
			stream >> mtlPath;
		}
		else if (type == "v") {	//load in vertices
			glm::vec3 position;
			stream >> position.x;
			stream >> position.y;
			stream >> position.z;
			vertices.push_back(position);
		}
		else if (type == "vn") {//load in vn
			glm::vec3 normal;
			stream >> normal.x;
			stream >> normal.y;
			stream >> normal.z;
			normals.push_back(normal);
		}
		else if (type == "vt") {//load in vt
			glm::vec2 tex;
			stream >> tex.x;
			stream >> tex.y;
			uvcoords.push_back(tex);
		}
		else if (type == "g") {//load in group name
			continue;
		}
		else if (type == "usemtl") {//load in group mtl
			continue;
		}
		else if (type == "s") {	//load in smooth shading const
			continue;
		}
		else if (type == "f") {	//load in face indices
			std::string v1, v2, v3, v4;
			stream >> v1 >> v2 >> v3 >> v4;
			VertexIndicies vertIndices1, vertIndices2, vertIndices3, vertIndices4;
			sscanf(v1.data(), "%l/%l/%l", vertIndices1.v, vertIndices1.n, vertIndices1.t);
			sscanf(v1.data(), "%l/%l/%l", vertIndices2.v, vertIndices2.n, vertIndices2.t);
			sscanf(v1.data(), "%l/%l/%l", vertIndices3.v, vertIndices3.n, vertIndices3.t);
			sscanf(v1.data(), "%l/%l/%l", vertIndices4.v, vertIndices4.n, vertIndices4.t);
			faceVertices.push_back(vertIndices1);
			faceVertices.push_back(vertIndices2);
			faceVertices.push_back(vertIndices3);
			faceVertices.push_back(vertIndices4);
		}
	}
	return false;
}

bool ObjLoader::loadMtl(std::string path)
{
	return false;
}


bool ObjLoader::loadTexture(std::string path)
{
	return false;
}

ObjLoader::~ObjLoader()
{
}
