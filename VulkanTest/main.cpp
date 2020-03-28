#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define STB_IMAGE_IMPLEMENTATION

#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <iostream>

#include "Renderer.h"

#include "ObjLoader.h"

int main() {
	ObjLoader loader;
	try {
		loader.loadObj("../models/bunny/sphere.obj", ReadMode::TRIANGLES);
	}
	catch (std::exception e) {
		std::cout << "exception reading obj: " << e.what() << std::endl;
		return -1;
	}
	OBJ model = loader.createObj();
	Texture texture;
	Mtl mtl;
	try {
		texture = loader.loadTexturePpm("../models/bunny/fur-bump.ppm");
		mtl = loader.loadMtl("../models/duck/12248_Bird_v1_L2.mtl");
	}
	catch (std::exception e) {
		std::cout << "exception: " << e.what() << std::endl;
		return -1;
	}
 	Renderer app(model, texture, mtl);
	try {
		app.run();
	}
	catch (std::exception e) {
		std::cout << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}