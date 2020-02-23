#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <vector>
#include <iostream>
#include <optional>
#include <set>
#include <algorithm>
#include <fstream>
#include "Mesh.h"
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <array>
#include <glm/mat4x4.hpp>
#include "ObjLoader.h"

#define DEBUG
#define BLEND true

/*
	helper struct to hold the indices for the queues that support the graphics family and present family
*/
struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily; //queue with graphics family capabilities
	std::optional<uint32_t> presentFamily; //queue with present family capabilities

	/*
		helper method to see if we have found a queue for both of these capabilities
	*/
	bool isComplete() {
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

//struct used querying swap chain support
struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities; //limits of the swapchain
	std::vector<VkSurfaceFormatKHR> formats; //supported surface formats (i.e. color and color space)
	std::vector<VkPresentModeKHR> presentModes; //how frames are presented to the surface
};

class Renderer
{

public:

	Renderer();
	Renderer(OBJ &model);// - store the reference to the OBJ, so you can set up the buffers
	~Renderer();
	
	void run();
	void renderMesh(Mesh mesh);

private:

	void initVulkan();
	void initWindow();
	void pickPhysicalDevice();
	
	/*
		Create the Vulkan Instance to maintain state variables of the pipeline
	*/
	void createInstance();
	void createLogicalDevice();
	void createSurface();
	void createSwapChain();
	void createRenderPass();
	void createImageViews();
	void createGraphicsPipeline();
	void createFramebuffers();
	void createCommandPool();
	void createVertexBuffer();
	void createIndexBuffer();
	void createDescriptorSetLayout();
	void createDescriptorPool();
	void createDescriptorSets();
	void createCommandBuffers();
	void createDepthResources();
	void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage & image, VkDeviceMemory & imageMemory);
	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
	VkCommandBuffer beginSingleTimeCommands();
	void endSingleTimeCommands(VkCommandBuffer commandBuffer);
	void createSyncObjects();

	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	VkShaderModule createShaderModule(const std::vector<char>& code);
	bool isDeviceSuitable(VkPhysicalDevice device);
	void populateDebugMessengerInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	void setupDebugMessenger();
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	void recreateSwapChain();
	void cleanupSwapChain();
	void cleanup();

	void drawFrame();

	static std::vector<char> readFile(const std::string& filename);
	
	void  mainLoop();
	
	std::vector<const char*> getRequiredExtensions();
	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
	VkFormat findDepthFormat();
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
	
	GLFWwindow * window;
	const int WIDTH = 800;
	const int HEIGHT = 600;

	/*
		 Layers are used to intercept the Vulkan API and provide logging, profiling, debugging, or other additional features.
		 Here we are using the validation layer to check and make sure that our setup is correct.
	*/
	const std::vector<const char*> validationLayers = {
		"VK_LAYER_KHRONOS_validation"
	};

	const std::vector<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

#ifdef DEBUG
	const bool enableValidationLayers = true;
#else
	const bool enableValidationLayers = false;
#endif // DEBUG
	bool checkValidationLayerSupport();

	//vulkan API handle
	/*
		The Vulkan instance is a software construct that logically separates the state of your application,
		from other applications or libraries running within the same context of your application.
		Other libraries in use here are GLFW for windowing and various std libs for data structures
		The vulkan instance keeps track of the state of the GPU pipeline and any resources allocated on GPU
		whereas GLFW keeps track of windows and event management, the two are seperate but interact with each other through the use
		of the APIs.
	*/
	VkInstance vkInstance;
	/*
		PHYSICAL DEVICE HANDLE

		A physical device usually represents a single piece of hardware or a collection of hardware that is interconnected. 
		There is a fixed, finite number of physical devices in any system unless that system supports reconfiguration such as hot-plug.
		A physical device has a selection of queues available for use by the software application through a logical device.
	*/
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

	/*
		LOGICAL DEVICE HANDLE

		This is the software construct around a physical device. It represents resources associated with a particular physical device. 
		This includes a possible subset of the available queues on the physical device. 
		It is possible to create multiple logical devices representing a single physical device.
		Application will spend most of its time interacting with logical device
	*/
	VkDevice device;

	//queue handle
	VkQueue graphicsQueue;

	//debug messenger handle
	VkDebugUtilsMessengerEXT debugMessenger;

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) {

		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

		return VK_FALSE;
	}

	VkSurfaceKHR surface;

	VkQueue presentationQueue;

	VkSwapchainKHR swapChain; //handle to the swapchain
	std::vector<VkImage> swapChainImages; //images (buffers) to use
	VkFormat swapChainImageFormat;//format we have decided to use
	VkExtent2D swapChainExtent;//resolution
	//image views: how to access and how to and which part to access
	std::vector<VkImageView> swapChainImageViews;

	//images to do with depth calculations
	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;

	VkRenderPass renderPass;
	VkDescriptorSetLayout descriptorSetLayout;
	VkPipelineLayout pipelineLayout;
	VkPipeline graphicsPipeline;
	/*
		a frame buffer wraps an attachment
		an attachment is represented in the pipeline by an image returned by the swap chain
		so we need to make an array of framebuffers as this is dependent on which image is returned
		by the swap chain (double buffered or more so there might be more than one framebuffer to draw to depending on this setup)
	*/
	std::vector<VkFramebuffer> swapChainFramebuffers;
	/*
		pool of commands to execute
		batch
	*/
	VkCommandPool commandPool;

	/*
		pool used to bind uniforms to buffers
	*/
	VkDescriptorPool descriptorPool;
	std::vector<VkDescriptorSet> descriptorSets; //the descriptor sets, 1 for each uniform, so 1 for each image in the swap chain

	/*
		per framebuffer recording of commands
	*/
	std::vector<VkCommandBuffer> commandBuffers;

	///*
	//	vertex buffer stuff
	//*/
	////struct to hold a vertex
	//struct Vertex {
	//	glm::vec2 position;
	//	glm::vec3 color;

	//	static VkVertexInputBindingDescription getBindingDescription() {
	//		VkVertexInputBindingDescription desc = {};
	//		desc.binding = 0; //which layout is this in reference to, layout location 0 is what we are binding to
	//		desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; //load after each vertex (can also be instance, but we wont use that)
	//		desc.stride = sizeof(Vertex);
	//		return desc;
	//	}

	//	static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
	//		std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions = {};
	//		attributeDescriptions[0].binding = 0;//which binding the vertex data comes from
	//		attributeDescriptions[0].location = 0;//the location directive input of the vertex shader
	//		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT; //type of the data attribute
	//		attributeDescriptions[0].offset = offsetof(Vertex, position); //offset, where to start reading (offset of position within struct, this is important because of potential compiler mangle)

	//		attributeDescriptions[1].binding = 0; //which binding does the data come from
	//		attributeDescriptions[1].location = 1; //refrence to a layout location within shader
	//		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT; //the format of the data
	//		attributeDescriptions[1].offset = offsetof(Vertex, color); //where in struct to read data for color
	//		return attributeDescriptions;
	//	}
	//};

	//buffer handle
	VkBuffer vertexBuffer;
	//memory handle
	VkDeviceMemory vertexBufferMemory;
	//index buffer handle
	VkBuffer indexBuffer;
	//index buffer memory
	VkDeviceMemory indexBufferMemory;

	//uniform buffer
	std::vector<VkBuffer> uniformBuffers;
	std::vector<VkDeviceMemory> uniformBuffersMemory;

	void createUniformBuffers();

	void updateUniformBuffer(uint32_t index);

	//vertices - update it to hold a 3d pos, 3d normal and 2d tex coord
	std::vector<Vertex> vertices;

	//uniforms
	struct UniformBufferObject {
		alignas(16) glm::mat4 model;
		alignas(16) glm::mat4 view;
		alignas(16) glm::mat4 proj;
	};


	//index buffer data
	std::vector<uint32_t> indices;

	//synchronization with render operations
	std::vector<VkSemaphore> imageAvailableSemaphores; //is the image available to render to? we use this to make sure we do not render to a frame that is being presented
	std::vector<VkSemaphore> renderFinishedSemaphores; //is the rendering finished? we use this to make sure that we do not render to an image that is already being rendered to
	std::vector<VkFence> inFlightFences; //used to sync CPU-GPU so we don't use in-flight frames
	std::vector<VkFence> imagesInFlight; //used to track which images are in flight
	const int MAX_FRAMES_IN_FLIGHT = 2; //number of frames that can be processed concurrently
	size_t currentFrame = 0; //variable to hold which frame we are currently rendering, it is circular so ranges between 0 - 1 (since we only have 2 frames to switch between)
	//we use this to handle resize events explicitly - whenever the window is resized this flag is set and then reset when the event is handled
	bool framebufferResized = false;
	static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
};

