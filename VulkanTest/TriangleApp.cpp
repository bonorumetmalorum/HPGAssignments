#include "TriangleApp.h"



TriangleApp::TriangleApp()
{
}

void TriangleApp::run()
{
	initWindow();
	initVulkan();
	mainLoop();
	cleanup();
}


TriangleApp::~TriangleApp()
{
}

void TriangleApp::initVulkan()
{
	createInstance();
	setupDebugMessenger();
	createSurface();
	pickPhysicalDevice();
	createLogicalDevice();
	createSwapChain();
	createImageViews();
	createRenderPass();
	createGraphicsPipeline();
	createFramebuffers();
	createCommandPool();
	createCommandBuffers();
	createSyncObjects();
}

void TriangleApp::initWindow()
{
	//init glfw
	glfwInit();
	//set glfw to no API and turn off window resizing events
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	//create the window
	window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
	glfwSetWindowUserPointer(window, this);
	glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

void TriangleApp::mainLoop()
{
	//event loop
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		drawFrame();
	}

	//cleaning up resources that are in use are bad (async code in use). we wait for the device to finish rendering before cleaning up
	vkDeviceWaitIdle(device);
}

/*
 Physical devices are normally parts of the system—a graphics card, accelerator, DSP, or other component
 once we have an instance, we can use this method to select an appropriate physcial device
 there are a fixed number of devices and each device has specific capabilities
*/
void TriangleApp::pickPhysicalDevice()
{
	//find the number of supported devices
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(vkInstance, &deviceCount, nullptr);	//call this function to find all vulkan enabled devices in the system by providing the instance, 
																	//an in/out parameter for the number of max devices supported by app / number of available supported devices
																	//and finally an array which can hold this number of devices
	if (deviceCount == 0) {	//if there are no supported devices do not continue
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}
	//store all the physical devices available
	std::vector<VkPhysicalDevice> devices(deviceCount); //resize array to support the number of physical devices available
	vkEnumeratePhysicalDevices(vkInstance, &deviceCount, devices.data()); //call again to populate the array
	for (const auto& device : devices) { //iterate through all available vulkan enabled devices
		if (isDeviceSuitable(device)) {  //check if the devices satisfies our requirements
			physicalDevice = device; //we found a device that matches our needs
			break; //if so break since we found a device
		}
	}
	//if we did not find a suitable device do not proceed
	if (physicalDevice == VK_NULL_HANDLE) {
		throw std::runtime_error("failed to find a suitable GPU!");
	}

}

void TriangleApp::createLogicalDevice()
{
	//setup structs for describing the queues we want to create
	QueueFamilyIndices indices = findQueueFamilies(physicalDevice); //get the indices of the queues we want to use
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos; //store the createInfo structs for the queues we want to use on the device 
	std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };
	//command priority execution, determines how commands are scheduled and this is even needed in the case of 1 queue
	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	//now we setup the features we want to use with vulkan, but nothing much as of yet
	VkPhysicalDeviceFeatures deviceFeatures = {};
	//this is like before but now we are setting config up for the device we chose
	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();
	if (enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else {
		createInfo.enabledLayerCount = 0;
	}
	//instantiate the device now
	if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
		throw std::runtime_error("failed to create logical device!");
	}
	vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
	vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentationQueue);
}

/*
	check if the device is suitable for rendering images
*/
bool TriangleApp::isDeviceSuitable(VkPhysicalDevice device) {
	QueueFamilyIndices indices = findQueueFamilies(device); //get the queue families we want to use
	bool deviceHasExtensions = checkDeviceExtensionSupport(device); //check if the device also supports the extensions we want to use
	bool swapChainAdequate = false; //boolean flag to check if the swapchain is good
	//proceed only if the device has extensions
	if (deviceHasExtensions) {
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device); // get the details of the swap chain supported by the device is good for our purposes
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty(); //check if there are formats and presentation modes for the swap chain
	}
	return indices.isComplete() && deviceHasExtensions && swapChainAdequate; //if the device has queues, extensions and a swap chain we can use we have found a suitable device
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}

void TriangleApp::cleanup()
{
	
	cleanupSwapChain();

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
		vkDestroyFence(device, inFlightFences[i], nullptr);
	}

	vkDestroyCommandPool(device, commandPool, nullptr);

	vkDestroyDevice(device, nullptr);

	if (enableValidationLayers) {
		DestroyDebugUtilsMessengerEXT(vkInstance, debugMessenger, nullptr);
	}

	vkDestroySurfaceKHR(vkInstance, surface, nullptr);
	vkDestroyInstance(vkInstance, nullptr);

	glfwDestroyWindow(window);

	glfwTerminate();
}

//get info to setup extensions
std::vector<const char*> TriangleApp::getRequiredExtensions() {
	uint32_t glfwExtensionCount = 0; //used as out parameter to know how many extensions are needed by GLFW
	const char** glfwExtensions; //array of extension names needed by GLFW
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount); //method to get all extensions needed by GLFW

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount); //create an array to hold all of the required extension names

	if (enableValidationLayers) {
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME); //if we want debugging using the validation layer add this extension to the list
	}

	return extensions; //return the extensions
}

/*
	Vulkan devices execute work that is submitted to queues
	A device will have one or more queues, and each of those queue will belong one of the devices queue families
	A queue family is a group of queues that have identical capabilities allowing them to run in parallel
	The number of queue families, the capabilities of each family, and the number of queues belonging to each family are all properties of the physical device
	Here we try to identify if the device has the desired queue families
*/
QueueFamilyIndices TriangleApp::findQueueFamilies(VkPhysicalDevice device)
{
	QueueFamilyIndices indices; //custom struct to hold the queue families we wish to use

	//get queue info from physical device
	uint32_t queueFamilyCount = 0; //out parameter to get the number of queue families
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr); // works somewhat like vkEnumeratePhysicalDevices(), returns structs describing the properties of each queue family supported by the device
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount); //resize the array such that it can hold all the queue families on the device
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data()); //get the queue families and allocate them in the array
	//find the index of the queue that has the VK_QUEUE_GRAPHICS_BIT set
	//the VK_QUEUE_GRAPHICS_BIT means that the queue familty supports drawing operations such as drawing points, lines and triangles
	int i = 0; //queue family index (the current one we are on)
	for (const auto& queueFamily : queueFamilies) {
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) { //if this queue has the graphics bit set
			indices.graphicsFamily = i; //we have found the queue we will use to submit render jobs
		}
		VkBool32 presentSupport = false; //boolean flag to indicate queues support of presentation operations
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport); //query if the queue has presentation operations supported
		if (presentSupport) { //if it does
			indices.presentFamily = i; //we found the queue we will use to render frames (usually the same as the graphics queue)
		}
		if (indices.isComplete()) { //if we have found all the queues we want
			break; //break early
		}
		i++; //increment to the next queue
	}
	return indices;
}

void TriangleApp::createInstance()
{
	if (enableValidationLayers && !checkValidationLayerSupport()) {
		throw std::runtime_error("validation layers requested, but not available");
	}
	//application info
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO; //type of the struct
	appInfo.pApplicationName = "Hello Triangle"; //nul-terminated string of the applications name
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0); //version of the application (useful for tools and drivers to act accordingly should there be a need)
	appInfo.pEngineName = "No Engine"; //name (string nul-terminated) of the engine middleware the application is based on
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0); //version of the middleware the application is based on
	appInfo.apiVersion = VK_API_VERSION_1_0; //version of the Vulkan API that your application is expecting to run on
	appInfo.pNext = nullptr; //field to provide additional arguments in a linked list like fashion, useful for extending structs without having to rewrite them entirely

	//creation info
	VkInstanceCreateInfo createInfo = {}; //contains parameters describing the new vk instance
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO; //type used to validate the structure (POD rule, lose typing info, this can be used to introspect and determine type)
	createInfo.pApplicationInfo = &appInfo; //optional structure that can be used to describe the application

	//setup extensions
	auto extensions = getRequiredExtensions();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size()); //number of extensions that will be enabled (extensions are extra functionality)
	createInfo.ppEnabledExtensionNames = extensions.data(); //the names of all the extensions that will be enabled

	//setup the validation layers if we have enabled it and setup the debugger
	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
	if (enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size()); //number of layers that are going to be enabled
		createInfo.ppEnabledLayerNames = validationLayers.data(); //the names of the layers that will be enabled

		populateDebugMessengerInfo(debugCreateInfo); //populate the fields of the debugCreateInfo struct
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo; //set the pNext pointer to the debugCreateInfo struct to indicate that there are 
																				  //aditional parameters for use to configure the validation layers
	}
	else {
		createInfo.enabledLayerCount = 0; //number of layers that are going to be enabled (none in this case because we dont want any layers

		createInfo.pNext = nullptr; //field to provide additional arguments in a linked list like fashion, useful for extending structs without having to rewrite them entirely (nothing here)
	}

	VkResult result = vkCreateInstance(&createInfo, nullptr, &vkInstance); 	//issue instance creation call with info, and no callbacks
	
	if (result != VK_SUCCESS) { //if the instance was not created successfully
		throw std::runtime_error("failed to create instance!"); //throw an error and stop proceeding with setup
	}

	//need to check support with GLFW TODO
	
}

void TriangleApp::createSurface()
{
	//very simple glfw method to make a surface to render to.
	//this is used because the vk method requires us to fill in a struct with config data
	//then send it over to setup the surface
	//glfwGetRequiredExtensions was used earlier to setup the required platform specific extensions that need to be used to create the surface
	//this method lets us continue writing platform independent code rather than having to specify for each platform the extensions and the appropriate
	//calls to create the surface
	VkResult result = glfwCreateWindowSurface(vkInstance, window, nullptr, &surface);
	if (result!= VK_SUCCESS) {
		std::cout << result << std::endl;
		throw std::runtime_error("failed to create window surface");
	}
}

void TriangleApp::createSwapChain()
{
	//query what is supported by the current physical device
	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);
	//setup the surface formats (buffer properties), presentation mode (buffers) and extent (resolution of rendering)
	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

	//setting for the minimum number of images that must be in the swap chain. +1 because we dont wait to wait and do nothing while the device does
	//driver operations.
	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	//we also do not want to exceed the maximum number of images so we check what it is and set our image count to that.
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	//BIG OL STRUCT TIME
	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR; //swap chain type :)
	createInfo.surface = surface; //surface we are tying the swap chain to
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1; //number of layers each image consists of
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; //what kind of operations we will use the images for
	//here we are rendering directly to them so we use attachment. There are other options...

	//we need to tell vulkan how the images will be used by the queue families
	//we have to queue, graphics and presentation.
	//we modify the images using the graphics queue and then submit them for rendering via the presentation queue
	//there are two ways to determine how images are used across different queues
	// CONCURRENT (can be used across multiple queue families without explicit ownership) or EXCLUSIVE (one family at a time ones an image)
	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };
	//if the presentation and graphics queues are seperate then use concurrent otherwise use exclusive
	if (indices.graphicsFamily != indices.presentFamily) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}
	
	//specify that no transforms should be applied to images in the swap chain
	createInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	//alpha channel that is used to blend this window with other windows (ignored)
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode; //presentation mode
	createInfo.clipped = VK_TRUE; //we dont care about colour of pixels that are obscured
	createInfo.oldSwapchain = VK_NULL_HANDLE; //we need to keep a pointer to the old swap chain which might be invalidated when recreate the swap chain

	if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) { //access violation
		throw std::runtime_error("failed to create swap chain!");
	}

	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
	swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());
	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent = extent;

}

void TriangleApp::populateDebugMessengerInfo(VkDebugUtilsMessengerCreateInfoEXT & createInfo)
{
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT; //type of the struct
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT; //the severity of messages we wish to catch
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT; //the types of messages we wish to intercept
	createInfo.pfnUserCallback = debugCallback; //static function to print validation errors to cout
}

//this is needed to find the function vkCreateDebugMessageUtilsEXT for use. It is not automatically loaded since it is an extension function.
//assuming that this is contained within the vulkan-1.dll file and we must first load the dll into memory and then find the address of the function in memory
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void TriangleApp::setupDebugMessenger()
{
	if (!enableValidationLayers) return;
	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	populateDebugMessengerInfo(createInfo);
	if (CreateDebugUtilsMessengerEXT(vkInstance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
		throw std::runtime_error("failed to set up debug messenger!");
	}
}

/*
	we check if the device has the capability to support the usage of the extensions we have specified (debugger and GLFW related extensions)
*/
bool TriangleApp::checkDeviceExtensionSupport(VkPhysicalDevice device)
{
	//enumerate all the extensions available
	uint32_t extensionCount; //same pattern as before, setup a variable to hold the number of supported extensions
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr); //get the number of supported extensions

	std::vector<VkExtensionProperties> availableExtensions(extensionCount); // resize the array such that we can allocate the informatiom related to supported extensions
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data()); //get the information
	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end()); //convert the extensions we want to be available to strings

	//now iterate over them, crossing them off
	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName); //remove this extension from our list of wanted extensions if it is there
	}

	//return that we either have all the extensions we want (true) or that we are missing some (false)
	return requiredExtensions.empty(); 
}

SwapChainSupportDetails TriangleApp::querySwapChainSupport(VkPhysicalDevice device)
{
	//we use the physical device and the window surface previously created to get information
	//on the supported swapchain features. We have to use these two because they are core parts of the swapchain
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities); 	//get the window surface capabilities, we provide the device that we wish to check, 
																						//the surface and then an out parameter to hold the various capabilities
	uint32_t formatCount; // get the number of formats available
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr); // query the physical devices supported surface formats , we do this by supplying a physical device, a surface and an out parameter to store the number of supported formats

	if (formatCount != 0) { //if there are surface formats supported by the physical device
		details.formats.resize(formatCount); //resize the array to hold all of them
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data()); // get the format names and store them in the array
	}

	uint32_t presentModeCount; // the number of presentation modes supported by the device and surface
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr); //get the number of presentation modes supported

	if (presentModeCount != 0) { //if we have presentation modes to use
		details.presentModes.resize(presentModeCount); // resize the array to hold the names of all the presentation modes supported 
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data()); //get the presentation modes
	}

	return details; //return the swap chain details
}

VkSurfaceFormatKHR TriangleApp::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	/*
		each available format is a struct that details the properties of each surface format
		notable properties are the colour channels and type where R8G8B8A8 is R, G, B and alpha represented as bytes
		and colour space which can be sRGB which is what is being used below
	*/
	for (const auto& availableFormat : availableFormats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat;
		}
	}

	return availableFormats[0];
}

VkPresentModeKHR TriangleApp::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	for (const auto& availablePresentMode : availablePresentModes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return availablePresentMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

/*
	resolution of the swap chain images
	is almost always equal to the resolution of the window we are drawing to
	some window managers allow us to differ the resolution of what we are drawing
	by setting the currentExtent value to INT32_MAX
*/
VkExtent2D TriangleApp::chooseSwapExtent(const VkSurfaceCapabilitiesKHR & capabilities)
{
	if (capabilities.currentExtent.width != UINT32_MAX) {
		return capabilities.currentExtent;
	}
	else {
		//get the width and height of the window
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		VkExtent2D actualExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

		actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}

void TriangleApp::createImageViews()
{
	//set size of image views array
	swapChainImageViews.resize(swapChainImages.size());
	//loop over all image views
	for (size_t i = 0; i < swapChainImages.size(); i++) {
		VkImageViewCreateInfo createInfo = {};
		//type of the struct - image view
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		//the iamge
		createInfo.image = swapChainImages[i];
		//how the image should be interpreted
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = swapChainImageFormat;

		//swizzle colour channels around, can be used to achieve a monochrom effect by mapping all colours to a single channel
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		//describes what part of the image should be accessed - colour in this case, no mipmapping or multiple layers here
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image views!");
		}
	}
}

void TriangleApp::createGraphicsPipeline()
{
	//read in shader programs in binary format
	auto vertShaderCode = readFile("../shaders/vert.spv");
	auto fragShaderCode = readFile("../shaders/frag.spv");

	//create shader modules using read in code
	VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
	VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

	//now that the shader modules have been created we need to assign them to specific stages in the pipeline
	VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO; //type of struct
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT; //pipeline stage we are assigning the module to
	vertShaderStageInfo.module = vertShaderModule; //shader module contaning the code 
	vertShaderStageInfo.pName = "main"; //entry point to shader program

	//same thing but now for the fragment shader
	VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	//describing the configuration of the newly created pipeline vertex input state
	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 0; //spacing between data and whether the data is per-vertex or per-instance
	vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
	vertexInputInfo.vertexAttributeDescriptionCount = 0; //type of the attributes passed to the vertex shader, which binding to load them from and at which offset
	vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional

	//describes two things: what kind of geometry will be drawn from the vertices and if primitive restart should be enabled
	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	//define the viewport (area to which we will render)
	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)swapChainExtent.width;
	viewport.height = (float)swapChainExtent.height;
	viewport.minDepth = 0.0f; //frame buffer depth values
	viewport.maxDepth = 1.0f; //frame buffer depth values

	//filter that discards pixels, we want to draw the entire image so we have a scissor angle to cover it entirely
	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = swapChainExtent;

	//combine the viewport and scissor configuration into viewport state
	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	//setup rasterization stage
	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO; 
	rasterizer.depthClampEnable = VK_FALSE; //fragments beyond the near and far plane are culled
	rasterizer.rasterizerDiscardEnable = VK_FALSE; //geometry does not pass through the rasterization stage
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL; //how fragments are generated from geometry
	rasterizer.lineWidth = 1.0f; //thickness of lines
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT; //which faces should we cull (here we choose to cull back faces)
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE; //how to iterate over vertices (determines which faces are front and back) can be CC or C
	rasterizer.depthBiasEnable = VK_FALSE; // can be modified based on slope but we dont want that here so it is disabled
	rasterizer.depthBiasConstantFactor = 0.0f; // Optional
	rasterizer.depthBiasClamp = 0.0f; // Optional
	rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

	//multisampling
	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f; // Optional
	multisampling.pSampleMask = nullptr; // Optional
	multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
	multisampling.alphaToOneEnable = VK_FALSE; // Optional

	//depth stencil testing not being used

	//colour-blending
	//configuration per framebuffer (we only have one framebuffer)
	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	if (BLEND) {
		colorBlendAttachment.blendEnable = VK_TRUE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
	}else{
		colorBlendAttachment.blendEnable = VK_FALSE; //do we blend?
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional
	}

	//references the array of structures for all of the framebuffers and allows you to set blend constants that you can use as blend factors in the blend calculations
	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE; //we dont want to use thse blend options
	colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f; // Optional
	colorBlending.blendConstants[1] = 0.0f; // Optional
	colorBlending.blendConstants[2] = 0.0f; // Optional
	colorBlending.blendConstants[3] = 0.0f; // Optional

	//dynamic state - what parameters can we change at runtime (can be nullptr if we dont have any)
	VkDynamicState dynamicStates[] = {
	VK_DYNAMIC_STATE_VIEWPORT,
	VK_DYNAMIC_STATE_LINE_WIDTH
	};

	VkPipelineDynamicStateCreateInfo dynamicState = {};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = 2;
	dynamicState.pDynamicStates = dynamicStates;

	//pipeline layout
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0; // Optional
	pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
	pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
	pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

	if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}

	//create graphics pipeline
	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;

	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = nullptr; // Optional
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = nullptr; // Optional
	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
	pipelineInfo.basePipelineIndex = -1; // Optional

	//more parameters are used here as multiple gps can be created in one go
	//second param is a cache which can be used to reuse data relevant to pipeline creation across multiple class
	if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	//destroy the shader programs because they have been setup in the pipeline
	vkDestroyShaderModule(device, fragShaderModule, nullptr);
	vkDestroyShaderModule(device, vertShaderModule, nullptr);
}

VkShaderModule TriangleApp::createShaderModule(const std::vector<char>& code)
{
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO; //type of the create info is Shader module
	createInfo.codeSize = code.size(); //the size of the buffer that holds the byte code of our shader program
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data()); //reinterpret cast our char array to unit32_t (needs to be alligned for int32 but it is because we used a vec
	VkShaderModule shaderModule;
	if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("failed to create shader module!");
	}
	return shaderModule;
}

//specify how many color and depth buffers there will be, how many samples to use for each of them and how their contents should be handled throughout the rendering operations.
void TriangleApp::createRenderPass()
{
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = swapChainImageFormat; //match format of swapchain
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT; //1 sample since we are not using any form of AA
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; //Clear the values to a constant at the start
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; //Rendered contents will be stored in memory and can be read later
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; //dont do anything with stencil buffer
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; //dont do anything with stencil buffer
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; //specifies which layout the image will have before the render pass begins
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // layout to automatically transition to when the render pass finishes. Images to be presented in the swap chain

	//Subpasses and attachment references
	/*
	single render pass can consist of multiple subpasses. 
	Subpasses are subsequent rendering operations that depend on the contents of framebuffers in previous passes, 
	for example a sequence of post-processing effects that are applied one after another
	*/
	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0; //which attachment are we referring to
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	//describe subpass
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1; //number of color attachments
	subpass.pColorAttachments = &colorAttachmentRef; //pointer into array containing the attachments (1 in this case so not an array)

	//render pass
	//create render pass
	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;

	//define subpass
	//first two fields specify the indices of the dependency and the dependent subpass
	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL; //implicit subpass before or after the render pass 
	dependency.dstSubpass = 0; //our subpass
	// next two fields specify the operations to wait on and the stages in which these operations occur
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	//next two fields specify the operations to wait on and the stages in which these operations occur
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	
	//setup the subpass in the renderpassinfo struct
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
		throw std::runtime_error("failed to create render pass!");
	}
}

void TriangleApp::createFramebuffers()
{
	//make space for the framebuffers
	swapChainFramebuffers.resize(swapChainImageViews.size());
	//iterate through all image views
	for (size_t i = 0; i < swapChainImageViews.size(); i++) {
		/*
			you can only use a framebuffer with a render pass that has the
			same number and type of attachments
		*/
		VkImageView attachments[] = {
			swapChainImageViews[i]
		};

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass; //render pass we are binding framebuffer to
		framebufferInfo.attachmentCount = 1; //number of attachments
		framebufferInfo.pAttachments = attachments; //attachments
		framebufferInfo.width = swapChainExtent.width;
		framebufferInfo.height = swapChainExtent.height;
		framebufferInfo.layers = 1; //number of layers in image array

		if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
}

void TriangleApp::createCommandPool()
{
	QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
	/*

	VK_COMMAND_POOL_CREATE_TRANSIENT_BIT: Hint that command buffers are rerecorded with new commands very often (may change memory allocation behavior)
	VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT: Allow command buffers to be rerecorded individually, without this flag they all have to be reset together

	*/
	poolInfo.flags = 0; // Optional

	if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create command pool!");
	}

}

void TriangleApp::createCommandBuffers()
{
	commandBuffers.resize(swapChainFramebuffers.size());
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; //specifies if the allocated command buffers are primary or secondary command buffers
	allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

	if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate command buffers!");
	}

	//begin recording commands for the command buffer
	for (size_t i = 0; i < commandBuffers.size(); i++) {
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0; // Optional specifies how we're going to use the command buffer
		beginInfo.pInheritanceInfo = nullptr; // Optional relevant for secondary command buffers

		/*If the command buffer was already recorded once, then a call to 
		vkBeginCommandBuffer will implicitly reset it.It's not possible to append commands to a buffer at a later time.
		*/
		if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass; //render pass itself
		renderPassInfo.framebuffer = swapChainFramebuffers[i]; //attachments (colour)
		//size of the render area
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = swapChainExtent;
		//clear values to use for VK_ATTACHMENT_LOAD_OP_CLEAR, which we used as load operation for the color attachment
		VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;
		//begin render pass
		//command buffer to record the command to
		//specifies the details of the render pass we've just provided
		//controls how the drawing commands within the render pass will be provided (execute in primary or secondary cmd buffer)
		vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		//bind graphics pipeline
		vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
		/*		
		vertexCount: Even though we don't have a vertex buffer, we technically still have 3 vertices to draw.
		instanceCount: Used for instanced rendering, use 1 if you're not doing that.
		firstVertex: Used as an offset into the vertex buffer, defines the lowest value of gl_VertexIndex.
		firstInstance: Used as an offset for instanced rendering, defines the lowest value of gl_InstanceIndex.
		*/
		vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);
		//end render pass
		vkCmdEndRenderPass(commandBuffers[i]);
		//end recording commands
		if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}
	}


}

/*
	create semaphores to sync program with rendering
*/
void TriangleApp::createSyncObjects()
{
	//similar pattern to previous steps when creating semaphores
	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS) {

			throw std::runtime_error("failed to create semaphores for a frame!");
		}
	}

	//we now also have to make fence objects to sync CPU and GPU
	inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
	imagesInFlight.resize(swapChainImages.size(), VK_NULL_HANDLE);

	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; //we need this so we can render on the very first pass (they are otherwise init to unsignaled and we wait for ever)
	
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		if (vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create fence for a frame!");
		}
	}


	/*
	we now have an array of semaphores, two for each image in flight, this way we do not submit more work than that which can be processed by the GPU
	this code is now deprecated
		if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS ||
			vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS) {

			throw std::runtime_error("failed to create semaphores!");
		}
	*/

}

void TriangleApp::recreateSwapChain()
{
	//handle minimisation events
	//we basically wait till the window is in the foreground again
	//this can cause an error where the width and height of the window is 0 which is invalid swap chain params
	int width = 0, height = 0;
	glfwGetFramebufferSize(window, &width, &height);
	while (width == 0 || height == 0) {
		glfwGetFramebufferSize(window, &width, &height);
		glfwWaitEvents();
	}

	//wait for device to finish what it is doing
	vkDeviceWaitIdle(device);

	cleanupSwapChain();

	//start to recreate the swap chain with new parameters
	createSwapChain();
	createImageViews();
	createRenderPass();
	createGraphicsPipeline();
	createFramebuffers();
	createCommandBuffers();
}

void TriangleApp::cleanupSwapChain()
{
	for (size_t i = 0; i < swapChainFramebuffers.size(); i++) {
		vkDestroyFramebuffer(device, swapChainFramebuffers[i], nullptr);
	}

	//do this so we dont need to allocate and new command pool, we can reuse the old one to issue new command buffers
	vkFreeCommandBuffers(device, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());

	vkDestroyPipeline(device, graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
	vkDestroyRenderPass(device, renderPass, nullptr);

	for (size_t i = 0; i < swapChainImageViews.size(); i++) {
		vkDestroyImageView(device, swapChainImageViews[i], nullptr);
	}

	vkDestroySwapchainKHR(device, swapChain, nullptr);
}

/*

	Acquire an image from the swap chain
	Execute the command buffer with that image as attachment in the framebuffer
	Return the image to the swap chain for presentation

*/
void TriangleApp::drawFrame()
{
	//before we start drawing again, we have to wait for the previous frame to finish
	//array of fences and waits for either any or all of them to be signaled before returning
	vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

	uint32_t imageIndex;
	//logical device, swap chain, timeout (disabled in this case) to wait for image, imageAvailable semaphore to signal that we can start drawing, 
	//finally variable to hold image (used to get right command buffer)
	VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

	//if the swap chain turns out to be out of date then we have to recreate the swap chain and continue in the next call
	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		recreateSwapChain();
		return;
	}
	//if the result is either a success or suboptimal we proceed, if it is anything else something has gone wrong and we throw an error
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	// Check if a previous frame is using this image (i.e. there is its fence to wait on)
	if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
		vkWaitForFences(device, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
	}
	// Mark the image as now being in use by this frame
	imagesInFlight[imageIndex] = inFlightFences[currentFrame];

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores; //which semaphore to wait on before submitting commands
	submitInfo.pWaitDstStageMask = waitStages; //which stages are waiting

	//command buffer we are submitting
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

	//which semaphore should we use to signal that rendering is complete
	VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	vkResetFences(device, 1, &inFlightFences[currentFrame]); //unlike with semaphores, we need to manually resotre the fence to the original state

	//The last parameter references an optional fence that will be signaled when the command buffers finish execution (CPU-GPU sync).
	if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
		throw std::runtime_error("failed to submit draw command buffer!");
	}


	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	// first two parameters specify which semaphores to wait on before presentation can happen,
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;
	//next two parameters specify the swap chains to present images to and the index of the image for each swap chain
	VkSwapchainKHR swapChains[] = { swapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;

	presentInfo.pResults = nullptr; // Optional allows you to specify an array of VkResult values to check for every individual swap chain if presentation was successful

	VkResult result1 = vkQueuePresentKHR(presentationQueue, &presentInfo); //submits the request to present an image to the swap chain
	
	//we have to check the same conditions here and recreate the swapchain if we need to
	if (result1 == VK_ERROR_OUT_OF_DATE_KHR || result1 == VK_SUBOPTIMAL_KHR || framebufferResized) {
		framebufferResized = false;
		recreateSwapChain();
	}
	else if (result1 != VK_SUCCESS) {
		throw std::runtime_error("failed to present swap chain image!");
	}
	
	//increment the frame we're rendering
	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

std::vector<char> TriangleApp::readFile(const std::string & filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("unable to open file");
	}
	
	size_t filesize = file.tellg();
	std::vector<char> filebuffer(filesize);

	file.seekg(0);
	file.read(filebuffer.data(), filesize);

	file.close();

	return filebuffer;

}

bool TriangleApp::checkValidationLayerSupport()
{
	uint32_t layerCount = 0;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char * layerName : validationLayers) {
		bool layerFound = false;
		for (const auto& layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}
		if (!layerFound) {
			return false;
		}
	}
	return true;
}

void TriangleApp::framebufferResizeCallback(GLFWwindow * window, int width, int height)
{
	auto app = reinterpret_cast<TriangleApp*>(glfwGetWindowUserPointer(window));
	app->framebufferResized = true;
}
