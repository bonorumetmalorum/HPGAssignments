#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define STB_IMAGE_IMPLEMENTATION

#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <iostream>

#include "Renderer.h"

#include "ObjLoader.h"

int main() {
	std::cout << "bool size "<< sizeof(bool) << std::endl;
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
	Mtl mtl;
	//load the texture here
	try {
		//texture = loader.loadTexture("../models/duck/12248_Bird_v1_diff.jpg");
		std::cout << "loading ppm now" << std::endl;
		texture = loader.loadPpm("../models/duck/ducktex.ppm");
		mtl = loader.loadMtl("../models/duck/12248_Bird_v1_L2.mtl");
	}
	catch (std::exception e) {
		std::cout << "exception: " << e.what() << std::endl;
		return -1;
	}
	/*for (long i = 0; i < texture.imageSize/4; i+=4) {
		std::cout << (unsigned int)texture.pixels[i] << " " << (unsigned int)texture.pixels[i+1] << " " << (unsigned int)texture.pixels[i+2] << " " << (unsigned int)texture.pixels[i+3] << std::endl;
	}*/
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