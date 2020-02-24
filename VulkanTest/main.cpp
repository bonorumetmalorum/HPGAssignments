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
		if (!loader.loadObj("../models/duck/12248_Bird_v1_L2.obj")) {
			throw std::runtime_error("unable to load obj");
		}
	}
	catch (std::exception e) {
		std::cout << "exception: " << e.what() << std::endl;
		return -1;
	}
	OBJ model = loader.createObj();
	Texture texture;
	
	//load the texture here
	try {
		texture = loader.loadTexture("../models/duck/12248_Bird_v1_diff.jpg");
	}
	catch (std::exception e) {
		std::cout << "exception: " << e.what() << std::endl;
		return -1;
	}
 	Renderer app(model, texture);
	try {
		app.run();
	}
	catch (std::exception e) {
		std::cout << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}