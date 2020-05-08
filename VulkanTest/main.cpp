#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define STB_IMAGE_IMPLEMENTATION

#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <iostream>

#include "Renderer.h"

#include "ObjLoader.h"

#include "Diredge.h"

int main() {
	ObjLoader loader;
	try {
		loader.loadObj("../models/bunny/bunny.obj", ReadMode::TRIANGLES);
	}
	catch (std::exception e) {
		std::cout << "exception reading obj: " << e.what() << std::endl;
		return -1;
	}
	OBJ model = loader.createObj();
	Texture texture;
	Texture fin;
	Mtl mtl;
	
	try {
		//texture = loader.loadTexturePpm("../models/bunny/shell_noise.ppm");
		texture = {};
		texture.height = 256;
		texture.width = 256;
		texture.depth = SHELLS;
		texture.pixels = nullptr;
		mtl = loader.loadMtl("../models/duck/12248_Bird_v1_L2.mtl");
		fin = loader.loadTexturePpm("../models/bunny/Fin.ppm");
	}
	catch (std::exception e) {
		std::cout << "exception: " << e.what() << std::endl;
		return -1;
	}

	auto halfedges = diredge::createMesh(model);

	diredge::createLineAdjacency(model, halfedges);
 	
	Renderer app(model, texture, fin, mtl, false);
	
	try {
		app.run();
	}
	catch (std::exception e) {
		std::cout << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}