#include "ObjLoader.h"
#include <iostream>
#include <unordered_map>

ObjLoader::ObjLoader()
{
}

//load an obj into memory
void ObjLoader::loadObj(std::string path, ReadMode mode)
{
	this->mode = mode;
	std::fstream inStream(path.data(), std::ios::in);
	if (!inStream.is_open()) {
		throw std::runtime_error("unable to open file");
	}
	std::string currentLine = "";
	while (inStream.peek() != -1) {
		std::getline(inStream, currentLine);
		std::istringstream stream(currentLine);
		std::string type;
		stream >> type;
		if (type == "#") continue; //check if comment, ignore
		else if (type == "mtllib") {		//get path filename of mtllib if it declared
			continue;
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
			tex.y *= -1;
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
			VertexIndicies vertIndices1, vertIndices2, vertIndices3, vertIndices4;
			switch (mode) {
			case ReadMode::QUADS:
				stream >> v1 >> v2 >> v3 >> v4;
				sscanf_s(v1.c_str(), "%ld/%ld/%ld", &vertIndices1.v, &vertIndices1.t, &vertIndices1.n);
				sscanf_s(v2.c_str(), "%ld/%ld/%ld", &vertIndices2.v, &vertIndices2.t, &vertIndices2.n);
				sscanf_s(v3.c_str(), "%ld/%ld/%ld", &vertIndices3.v, &vertIndices3.t, &vertIndices3.n);
				sscanf_s(v4.c_str(), "%ld/%ld/%ld", &vertIndices4.v, &vertIndices4.t, &vertIndices4.n);
				faceVertices.push_back(vertIndices1);
				faceVertices.push_back(vertIndices2);
				faceVertices.push_back(vertIndices3);
				faceVertices.push_back(vertIndices4);
				break;
			case ReadMode::TRIANGLES:
				stream >> v1 >> v2 >> v3;
				sscanf_s(v1.c_str(), "%ld/%ld/%ld", &vertIndices1.v, &vertIndices1.t, &vertIndices1.n);
				sscanf_s(v2.c_str(), "%ld/%ld/%ld", &vertIndices2.v, &vertIndices2.t, &vertIndices2.n);
				sscanf_s(v3.c_str(), "%ld/%ld/%ld", &vertIndices3.v, &vertIndices3.t, &vertIndices3.n);
				faceVertices.push_back(vertIndices1);
				faceVertices.push_back(vertIndices2);
				faceVertices.push_back(vertIndices3);
				break;
			}
		}
	}
}

//load and create an mtl for use with vulkan
Mtl ObjLoader::loadMtl(std::string path)
{
	std::fstream inStream(path.data(), std::ios::in);
	if(!inStream.is_open()){
		throw std::runtime_error("unable to open mtl file");
	}
	Mtl mtl = {};
	std::string currentLine = "";
	while (inStream.peek() != -1) {
		std::getline(inStream, currentLine);
		std::istringstream stream(currentLine);
		std::string type;
		stream >> type;
		if (type == "#") {
			continue;
		}
		if (type == "Ns") {
			stream >> mtl.specularExponent;
		}
		if (type == "Ka") {
			stream >> mtl.ambient.r;
			stream >> mtl.ambient.g;
			stream >> mtl.ambient.b;
		}
		if (type == "Kd") {
			stream >> mtl.diffuse.r;
			stream >> mtl.diffuse.g;
			stream >> mtl.diffuse.b;
		}
		if (type == "Ks") {
			stream >> mtl.specular.r;
			stream >> mtl.specular.g;
			stream >> mtl.specular.b;
		}
	}
	return mtl;
}

//load and create a texture for use with vulkan
Texture ObjLoader::loadTextureJpg(std::string path)
{
	Texture tex = {};
	int texWidth, texHeight, texChannels;
	tex.pixels = stbi_load(path.data(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	tex.imageSize = texWidth * texHeight * 4;
	tex.width = texWidth;
	tex.height = texHeight;
	if (!tex.pixels) {
		throw std::runtime_error("failed to load texture image!");
	}
	return tex;
}

//load and create a texture for use with vulkan
Texture ObjLoader::loadTexturePpm(std::string path)
{
	Texture t = {};
	std::ifstream instream(path.c_str(), std::ios::binary|std::ios::in);
	if (!instream.is_open()) {
		throw std::runtime_error("unable to open ppm file");
	}
	char magicL, magicN;
	instream >> magicL;
	instream >> magicN;
	if (magicL == 'P' && magicN == '6') { //check if binary format, if not throw error
		char comment;
		instream >> comment;
		if (comment == '#') {
			char commentline[256];
			instream.getline(commentline, 256);
		}
		int height, width, maxVal = 0;
		instream >> height;
		instream >> width;
		instream >> maxVal;
		std::cout << maxVal << std::endl;
		t.height = height;
		t.width = width;
		t.imageSize = height * width * 4;
		t.pixels = new unsigned char[(size_t)t.imageSize];
		instream.get();
		for (size_t i = 0; i < t.imageSize / 4; i++) {
			unsigned char r, g, b, a = (unsigned char)255;
			instream.read((char*)&r, sizeof(char));
			instream.read((char*)&g, sizeof(char));
			instream.read((char*)&b, sizeof(char));
			t.pixels[(i * 4)] = r;
			t.pixels[(i * 4) + 1] = g; 
			t.pixels[(i * 4) + 2] = b;
			t.pixels[(i * 4) + 3] = b;
		}
		return t;
	}
	else if (magicL == 'P' && magicN == '3') {
		char comment;
		instream >> comment;
		if (comment = '#') {
			instream.ignore(100, '\n');
		}
		int height, width, maxVal;
		instream >> height;
		instream >> width;
		instream >> maxVal;
		//instream >> height >> width >> maxVal;
		t.height = height;
		t.width = width;
		t.imageSize = height * width * 4;
		t.pixels = new unsigned char[(size_t)t.imageSize];
		instream.get();
		for (size_t i = 0; i < t.imageSize / 4; i++) {
			unsigned int r, g, b, a = 255;
			instream.read((char*)&r, sizeof(char));
			instream.read((char*)&g, sizeof(char));
			instream.read((char*)&b, sizeof(char));
			//instream.read(&a, sizeof(char));
			if (r == (char)0 || g == (char)0 || b == (char)0)
				a = 0;
			t.pixels[(i * 4)] = r;
			t.pixels[(i * 4) + 1] = g;
			t.pixels[(i * 4) + 2] = b;
			t.pixels[(i * 4) + 3] = a;
		}
		return t;
	}
	else {
		throw std::runtime_error("unsuported ppm format");
	}

}

ObjLoader::~ObjLoader()
{
}

//create an obj with data ready to be loaded into vulkan (triangulated)
OBJ ObjLoader::createObj()
{
	OBJ object;
	int numPerF;
	switch (this->mode) {
	case ReadMode::QUADS:
		triangulate(object);
		break;
	case ReadMode::TRIANGLES:
		package(object);
		break;
	}
	return object;
}

std::vector<glm::vec3>& ObjLoader::getVertices()
{
	return this->vertices;
}

void ObjLoader::triangulate(OBJ & object)
{
	std::unordered_map<Vertex, uint32_t> uniqueVertices; //keep track of unique face vertices
	for (long i = 0; i < faceVertices.size() / 4; i++) {
		//data for 0 vertex
		glm::vec3 position = vertices[faceVertices[i * 4].v - 1];
		glm::vec3 normal = normals[faceVertices[i * 4].n - 1];
		glm::vec2 uv = uvcoords[faceVertices[i * 4].t - 1];
		Vertex v0 = { position, normal, uv };
		//data for 1 vertex
		glm::vec3 position1 = vertices[faceVertices[i * 4 + 1].v - 1];
		glm::vec3 normal1 = normals[faceVertices[i * 4 + 1].n - 1];
		glm::vec2 uv1 = uvcoords[faceVertices[i * 4 + 1].t - 1];
		Vertex v1 = { position1, normal1, uv1 };
		//data for 2 vertex
		glm::vec3 position2 = vertices[faceVertices[i * 4 + 2].v - 1];
		glm::vec3 normal2 = normals[faceVertices[i * 4 + 2].n - 1];
		glm::vec2 uv2 = uvcoords[faceVertices[i * 4 + 2].t - 1];
		Vertex v2 = { position2, normal2, uv2 };
		//data for 3 vertex
		glm::vec3 position3 = vertices[faceVertices[i * 4 + 3].v - 1];
		glm::vec3 normal3 = normals[faceVertices[i * 4 + 3].n - 1];
		glm::vec2 uv3 = uvcoords[faceVertices[i * 4 + 3].t - 1];
		Vertex v3 = { position3, normal3, uv3 };

		//routine to add vertices in right order
		uint32_t iv0, iv1, iv2, iv3 = 0;
		if (uniqueVertices.count(v0) == 0) {
			uniqueVertices[v0] = object.vertexList.size();
			iv0 = object.vertexList.size();
			object.vertexList.push_back(v0);
		}
		else {
			iv0 = uniqueVertices[v0];
		}
		if (uniqueVertices.count(v1) == 0) {
			uniqueVertices[v1] = object.vertexList.size();
			iv1 = object.vertexList.size();
			object.vertexList.push_back(v1);
		}
		else {
			iv1 = uniqueVertices[v1];
		}
		if (uniqueVertices.count(v2) == 0) {
			uniqueVertices[v2] = object.vertexList.size();
			iv2 = object.vertexList.size();
			object.vertexList.push_back(v2);
		}
		else {
			iv2 = uniqueVertices[v2];
		}
		if (uniqueVertices.count(v3) == 0) {
			uniqueVertices[v3] = object.vertexList.size();
			iv3 = object.vertexList.size();
			object.vertexList.push_back(v3);
		}
		else {
			iv3 = uniqueVertices[v3];
		}
		//add 0, 1, 2, 2, 3, 0 to triangulate the quad
		object.indices.push_back(iv0);
		object.indices.push_back(iv1);
		object.indices.push_back(iv2);
		object.indices.push_back(iv2);
		object.indices.push_back(iv3);
		object.indices.push_back(iv0);
	}
}

void ObjLoader::package(OBJ& object)
{
	std::unordered_map<Vertex, uint32_t> uniqueVertices; //keep track of unique face vertices
	std::vector<uint32_t>noNormals;
	for (long i = 0; i < faceVertices.size() / 3; i++) {
		//data for 0 vertex
		glm::vec3 position = vertices[faceVertices[i * 3].v - 1];
		glm::vec3 normal = normals[faceVertices[i * 3].n - 1];
		glm::vec2 uv = uvcoords[faceVertices[i * 3].t - 1];
		Vertex v0 = { position, normal, uv };
		//data for 1 vertex
		glm::vec3 position1 = vertices[faceVertices[i * 3 + 1].v - 1];
		glm::vec3 normal1 = normals[faceVertices[i * 3 + 1].n - 1];
		glm::vec2 uv1 = uvcoords[faceVertices[i * 3 + 1].t - 1];
		Vertex v1 = { position1, normal1, uv1 };
		//data for 2 vertex
		glm::vec3 position2 = vertices[faceVertices[i * 3 + 2].v - 1];
		glm::vec3 normal2 = normals[faceVertices[i * 3 + 2].n - 1];
		glm::vec2 uv2 = uvcoords[faceVertices[i * 3 + 2].t - 1];
		Vertex v2 = { position2, normal2, uv2 };
		
		uint32_t iv0, iv1, iv2, iv3 = 0;
		if (uniqueVertices.count(v0) == 0) {
			uniqueVertices[v0] = object.vertexList.size();
			iv0 = object.vertexList.size();
			object.vertexList.push_back(v0);
			noNormals.push_back(1);

		}
		else {
			iv0 = uniqueVertices[v0];
			object.vertexList[iv0].normal += v0.normal;
			noNormals[iv0]++;
		}
		if (uniqueVertices.count(v1) == 0) {
			uniqueVertices[v1] = object.vertexList.size();
			iv1 = object.vertexList.size();
			object.vertexList.push_back(v1);
			noNormals.push_back(1);
		}
		else {
			iv1 = uniqueVertices[v1];
			object.vertexList[iv1].normal += v0.normal;
			noNormals[iv1]++;
		}
		if (uniqueVertices.count(v2) == 0) {
			uniqueVertices[v2] = object.vertexList.size();
			iv2 = object.vertexList.size();
			object.vertexList.push_back(v2);
			noNormals.push_back(1);
		}
		else {
			iv2 = uniqueVertices[v2];
			object.vertexList[iv2].normal += v0.normal;
			noNormals[iv2]++;
		}

		object.indices.push_back(iv0);
		object.indices.push_back(iv1);
		object.indices.push_back(iv2);
	}
	for (long j = 0; j < object.vertexList.size(); j++)
	{
		//object.vertexList[j].normal /= noNormals[j];
		std::vector<Vertex*> veticesToCorrect;
		glm::vec3 tempNormal = object.vertexList[j].normal;
		veticesToCorrect.push_back(&object.vertexList[j]);
		uint32_t count = 0;
		for (long y = j; y < object.vertexList.size(); y++)
		{
			if (object.vertexList[j].position == object.vertexList[y].position)
			{
				tempNormal += object.vertexList[y].normal;
				veticesToCorrect.push_back(&object.vertexList[y]);
				count++;
			}
		}
		tempNormal /= count;
		for (auto vert : veticesToCorrect)
		{
			vert->normal = tempNormal;
		}
	}
}

void ObjLoader::packageAdjacency(OBJ& obj)
{
}
