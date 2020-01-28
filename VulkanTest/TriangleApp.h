#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <vector>
#include <iostream>
#include <optional>

//#define DEBUG

struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;

	bool isComplete() {
		return graphicsFamily.has_value();
	}
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
	void  mainLoop();
	void pickPhysicalDevice();
	void createLogicalDevice();
	bool isDeviceSuitable(VkPhysicalDevice device);
	void cleanup();
	void createInstance();
	void populateDebugMessengerInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	void setupDebugMessenger();
	std::vector<const char*> getRequiredExtensions();
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

	GLFWwindow * window;
	const int WIDTH = 800;
	const int HEIGHT = 600;

	const std::vector<const char*> validationLayers = {
		"VK_LAYER_KHRONOS_validation"
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
};

