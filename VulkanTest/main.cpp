#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <iostream>

#include "Renderer.h"

#include "ObjLoader.h"

int main() {
	ObjLoader loader;
	try {
		bool result = loader.loadObj("../models/duck/12248_Bird_v1_L2.obj");
	}
	catch (std::exception e) {
		std::cout << "exception: " << e.what() << std::endl;
		return -1;
	}
	Renderer app;
	try {
		app.run();
	}
	catch (std::exception e) {
		std::cout << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}