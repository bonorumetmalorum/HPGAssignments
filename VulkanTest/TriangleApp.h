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

#define DEBUG
#define BLEND true

struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete() {
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

//struct used querying swap chain support
struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

class TriangleApp
{

public:

	TriangleApp();
	void run();
	~TriangleApp();

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
	void createCommandBuffers();
	void createSyncObjects();

	VkShaderModule createShaderModule(const std::vector<char>& code);
	bool isDeviceSuitable(VkPhysicalDevice device);
	void populateDebugMessengerInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	void setupDebugMessenger();
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	void recreateSwapChain();
	void cleanupSwapChain();
	void cleanup();

	void drawFrame();

	static std::vector<char> readFile(const std::string& filename);
	
	void  mainLoop();
	
	std::vector<const char*> getRequiredExtensions();
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

	VkSwapchainKHR swapChain;
	std::vector<VkImage> swapChainImages; //images (buffers) to use
	VkFormat swapChainImageFormat;//format we have decided to use
	VkExtent2D swapChainExtent;//resolution
	//image views: how to access and how to and which part to access
	std::vector<VkImageView> swapChainImageViews;

	VkRenderPass renderPass;
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
		per framebuffer recording of commands
	*/
	std::vector<VkCommandBuffer> commandBuffers;

	//synchronisation with render operations
	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences; //used to sync CPU-GPU so we dont use inflight frames
	std::vector<VkFence> imagesInFlight; //used to track which images are in flight
	const int MAX_FRAMES_IN_FLIGHT = 2;
	size_t currentFrame = 0;
	//we use this to handle resize events explicitly
	bool framebufferResized = false;
	static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
};

