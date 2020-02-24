#pragma once
#include <vector>
#include <string>
#include <fstream>
#include <stdexcept>
#include <sstream>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <vulkan/vulkan.h>
#include <array>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <stb_image.h>


struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 uv;

	static VkVertexInputBindingDescription getBindingDescription() {
		VkVertexInputBindingDescription desc = {};
		desc.binding = 0; //which layout is this in reference to, layout location 0 is what we are binding to
		desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; //load after each vertex (can also be instance, but we wont use that)
		desc.stride = sizeof(Vertex);
		return desc;
	}

	//add attribute description for normal and uv and remove for color 
	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = {};
		attributeDescriptions[0].binding = 0;//which binding the vertex data comes from
		attributeDescriptions[0].location = 0;//the location directive input of the vertex shader
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT; //type of the data attribute
		attributeDescriptions[0].offset = offsetof(Vertex, position); //offset, where to start reading (offset of position within struct, this is important because of potential compiler mangle)

		attributeDescriptions[1].binding = 0; //which binding does the data come from
		attributeDescriptions[1].location = 1; //refrence to a layout location within shader
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT; //the format of the data
		attributeDescriptions[1].offset = offsetof(Vertex, normal); //where in struct to read data for color

		attributeDescriptions[2].binding = 0; //which binding does the data come from
		attributeDescriptions[2].location = 2; //refrence to a layout location within shader
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT; //the format of the data
		attributeDescriptions[2].offset = offsetof(Vertex, uv); //where in struct to read data for color
		return attributeDescriptions;
	}

	bool operator==(const Vertex& other) const {
		return position == other.position && normal == other.normal && uv == other.uv;
	}
};

namespace std {
	template<> struct hash<Vertex> {
		size_t operator()(Vertex const& vertex) const {
			return ((hash<glm::vec3>()(vertex.position) ^
				(hash<glm::vec3>()(vertex.normal) << 1)) >> 1) ^
				(hash<glm::vec2>()(vertex.uv) << 1);
		}
	};
};


struct OBJ {
	std::vector<Vertex> vertexList; //unique vertices
	std::vector<uint32_t> indices; //grouped in triplets
};

struct Texture {
	VkDeviceSize imageSize;
	stbi_uc* pixels;
	int width, height;
};

class ObjLoader
{
public:
	ObjLoader();
	//load obj
	bool loadObj(std::string path);
	//load mtl
	bool loadMtl(std::string path);
	//load texture
	Texture loadTexture(std::string path);
	~ObjLoader();
	
	struct VertexIndicies {
		uint32_t v, n, t = 0;
	};

	OBJ createObj();

	std::vector<glm::vec3> & getVertices();

private:
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> uvcoords;
	std::vector<VertexIndicies> faceVertices;
};

