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
	void createLogicalDevice();
	bool isDeviceSuitable(VkPhysicalDevice device);
	void cleanup();
	void createInstance();
	void createSurface();
	void createSwapChain();
	void populateDebugMessengerInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	void setupDebugMessenger();
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	void createImageViews();
	void createGraphicsPipeline();
	VkShaderModule createShaderModule(const std::vector<char>& code);
	void createRenderPass();
	void createFramebuffers();
	void createCommandPool();
	void createCommandBuffers();

	static std::vector<char> readFile(const std::string& filename);
	
	void  mainLoop();
	
	std::vector<const char*> getRequiredExtensions();
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
	
	GLFWwindow * window;
	const int WIDTH = 800;
	const int HEIGHT = 600;

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
	VkInstance vkInstance;
	//physical device handle
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	//logical device handle
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
};

