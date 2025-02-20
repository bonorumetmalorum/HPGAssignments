#include "TriangleApp.h"



TriangleApp::TriangleApp()
{
}

/*
	main loop
*/
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

/*
	initialize vulkan
*/
void TriangleApp::initVulkan()
{
	createInstance(); //create an instance to store vulkan related state
	setupDebugMessenger();//setup the debug messenger to hold state for the debug extension layer
	createSurface(); //create a surface we can render images to
	pickPhysicalDevice(); //pick a physical device we will use for our graphics pipeline
	createLogicalDevice(); //create a logical device wrapper with the necessary resources around the physical device
	createSwapChain(); //create a swapchain that we can use to render images to the surface
	createImageViews(); //create the image views that will hold additional info about the images in the swapchain
	createRenderPass(); //create a render pass that specifies all the stages of the render
	createGraphicsPipeline(); //create a graphics pipeline to process drawing commands and render to the surface
	createFramebuffers(); //create a framebuffer to represent the set of images the graphics pipeline will render to
	createCommandPool(); //create a command pool to manage allocation of command buffers
	createCommandBuffers(); //create the command buffer from the pool with the appropriate commands
	createSyncObjects(); //create synchronization primitives to control rendering
}

/*
	create a window that we can use to present to
*/
void TriangleApp::initWindow()
{
	glfwInit(); //init glfw
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);//set glfw to no API
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);//we want the window to be resize-able
	window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);//create the window
	glfwSetWindowUserPointer(window, this); //set the user pointer (used to determine who is controlling the window)
	glfwSetFramebufferSizeCallback(window, framebufferResizeCallback); //setup the window resize call back function
}

/*
	the main loop that will draw the triangle to screen and handle window events
*/
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
	 Physical devices are normally parts of the system's graphics card, accelerator, DSP, or other component
	 once we have an instance, we can use this method to select an appropriate physical device
	 there are a fixed number of devices and each device has specific capabilities
*/
void TriangleApp::pickPhysicalDevice()
{
	uint32_t deviceCount = 0;//find the number of supported devices
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

/*
	after choosing a physical device(s) we need to create a logical device wrapper. A logical device represents a physical device in an initialized state
	When creating the logical device we have the choice to opt in to additional features and turn on any extensions we want to use...
*/
void TriangleApp::createLogicalDevice()
{
	//setup structs for describing the queues we want to create
	QueueFamilyIndices indices = findQueueFamilies(physicalDevice); //get the indices of the queues we want to use
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos; //store the createInfo structs for the queues we want to use on the device 
	std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };
	//command priority execution, determines how commands are scheduled and this is even needed in the case of 1 queue
	float queuePriority = 1.0f; //priority of the queues we wish to create
	for (uint32_t queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO; //createInfo struct type
		queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value(); //the index of the queue family
		queueCreateInfo.queueCount = 1; //the number of queues we are constructing (device must support at least this many)
		queueCreateInfo.pQueuePriorities = &queuePriority; //optional parameter of an array containing values from 0.0 - 1.0. The higher the number the more resources a queue gets allocated and more aggressively scheduled it is
		queueCreateInfos.push_back(queueCreateInfo); //push back the struct on to our array of queue create info structs
	}

	//now we setup the features we want to use with vulkan, but nothing much as of yet
	VkPhysicalDeviceFeatures deviceFeatures = {};
	vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures); //enable all the device features available by simply querying the device for what features it supports
	//this is like before but now we are setting the config for the device we chose
	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO; //type of createInfo struct
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()); //the number of queues we wish to use
	createInfo.pQueueCreateInfos = queueCreateInfos.data(); //the config data for the queues we wish to use
	createInfo.pEnabledFeatures = &deviceFeatures; //features we are opting in to use
	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());//the number of enabled extensions we have
	createInfo.ppEnabledExtensionNames = deviceExtensions.data(); //the array containing the names of all the extensions we wish to use
	if (enableValidationLayers) { //if we want to enable layers (validation in this case)
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size()); //set the number of enabled layers we have (1 in this case)
		createInfo.ppEnabledLayerNames = validationLayers.data(); // provide the names of the validation layers we want to enable
	}
	else {
		createInfo.enabledLayerCount = 0; //set it to 0 if we don't want any validation layers
	}
	//instantiate the logical device now
	if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) { //if we are not successful
		throw std::runtime_error("failed to create logical device!"); //stop and throw an error
	}

	vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue); //store a reference to the graphics queue that was created on the device
	vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentationQueue); //store a reference to the presentation queue that was created on the device
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

/*
	destroy the debug utils messenger object and its state information
*/
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}

/*
	destroy all resources on shutdown
*/
void TriangleApp::cleanup()
{
	
	cleanupSwapChain(); //first we clean up the swap chain and all related resources

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) { //destroy all synchronization objects
		vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
		vkDestroyFence(device, inFlightFences[i], nullptr);
	}

	vkDestroyCommandPool(device, commandPool, nullptr); //destroy the command pool

	vkDestroyDevice(device, nullptr); //destroy the logical device

	if (enableValidationLayers) {
		DestroyDebugUtilsMessengerEXT(vkInstance, debugMessenger, nullptr); //destroy the validation layer
	}

	vkDestroySurfaceKHR(vkInstance, surface, nullptr); //destroy the surface used for presentation
	vkDestroyInstance(vkInstance, nullptr); //destroy the vulkan instance

	glfwDestroyWindow(window); //destroy the window

	glfwTerminate(); //stop GLFW
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
	A physical device will have a certain number of queue families where each family has specific capabilities and have a certain number of queues available to process commands
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
	//the VK_QUEUE_GRAPHICS_BIT means that the queue family supports drawing operations such as drawing points, lines and triangles
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

/*
	create a vulkan instance to store state
*/
void TriangleApp::createInstance()
{
	if (enableValidationLayers && !checkValidationLayerSupport()) {
		throw std::runtime_error("validation layers requested, but not available");
	}
	//application info
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO; //type of the struct - all if not most structs in vulkan need this field to do type introspection at runtime, furthermore, this is a good paradigm to use because of the POD loophole
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
																				  //additional parameters for use to configure the validation layers
	}
	else {
		createInfo.enabledLayerCount = 0; //number of layers that are going to be enabled (none in this case because we don't want any layers

		createInfo.pNext = nullptr; //field to provide additional arguments in a linked list like fashion, useful for extending structs without having to rewrite them entirely (nothing here)
	}

	VkResult result = vkCreateInstance(&createInfo, nullptr, &vkInstance); 	//issue instance creation call with info, and no callbacks
	
	if (result != VK_SUCCESS) { //if the instance was not created successfully
		throw std::runtime_error("failed to create instance!"); //throw an error and stop proceeding with setup
	}	
}

/*
	create a surface that we can use to present rendered images to
*/
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

/*
	A swap chain is used to manage a set of images
	when you create a surface aka VkSurfaceKHR handle this refers to vulkans view of a window
	in order to render anything to the window it is necessary to create an image that can be used to store the data that will be rendered to the window
	because these images might be tied tightly to the windowing system on the platform, we create a swap chain which will setup the images by negotiating with the windowing system
	the swap chain will then present us with 1 or more images we can use to render to the window
	the swap chain will manage the images in a ring or circular buffer, where we can ask it to give us the next available image, while another is being rendered to the window
*/
void TriangleApp::createSwapChain()
{
	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice); //query what is supported by the swap chain on the physical device (we want surface capabilities, surface formats, and presentation modes) 
	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats); //setup the surface formats (buffer properties), presentation mode (buffers) and extent (resolution of rendering)
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes); //which kind of presentation mode do we want to use (MAIL_BOX etc...)
	VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities); //choose an appropriate swapchain extent

	//setting for the minimum number of images that must be in the swap chain. +1 because we don't want to wait and do nothing while the device does driver operations.
	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1; //if this is set to 1 it means that we want to render directly to the front buffer, which is bad and not supported by all devices, 3 is recommended
	//we also do not want to exceed the maximum number of images so we check what it is and set our image count to that.
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) { //in order to not exceed the maximum number of images available in the swap chain, if the max image count is less than the current image count
		imageCount = swapChainSupport.capabilities.maxImageCount; //set the image count to the max supported by the swap chain
	}
	//setup the config for the swap chain
	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR; //swap chain type
	createInfo.surface = surface; //surface we are tying the swap chain to
	createInfo.minImageCount = imageCount; // the minimum number of images we need
	createInfo.imageFormat = surfaceFormat.format; //the supported format (in memory representation of pixels) we wish to use to render
	createInfo.imageColorSpace = surfaceFormat.colorSpace; //the color space which will be RGB or sRGB, whichever is supported by the surface
	createInfo.imageExtent = extent; //dimensions of the image in the swap chain
	createInfo.imageArrayLayers = 1; //number of layers each image consists of, can be used to present a layered image to the user, or specific layers of an image
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; //how will we use the image (in addition to it being used as presentation)

	//we need to tell vulkan how the images will be used by the queue families
	//we have two queues, graphics and presentation.
	//we modify the images using the graphics queue and then submit them for rendering via the presentation queue
	//there are two ways to determine how images are used across different queues
	// CONCURRENT (can be used across multiple queue families without explicit ownership) or EXCLUSIVE (one family at a time ones an image)
	QueueFamilyIndices indices = findQueueFamilies(physicalDevice); //get the queue families we will use
	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() }; //the indices of the queue families we are using
	//if the presentation and graphics queues are separate then use concurrent otherwise use exclusive
	if (indices.graphicsFamily != indices.presentFamily) { //are the queues the same?
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT; //will the image be used by only one queue or will it be shared by multiple queues, currently set to be shared
		createInfo.queueFamilyIndexCount = 2; //we have 2 queue families being uses (graphics and presentation)
		createInfo.pQueueFamilyIndices = queueFamilyIndices; //the array of queue family indices
	}
	else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; //if the graphics and presentation queue families are the same then the images are used by only one queue family, no need for sharing
	}
	
	createInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR; //specify that no transforms should be applied to images in the swap chain (i.e. rotations and flipping, in this case no transformations are applied by specifying identity transform)
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; //controls how alpha composition is handled by windowing system (for example, transparent terminals etc), this is ignored by setting it to opaque (no transparency)
	createInfo.presentMode = presentMode; //presentation mode controls synchronization with the window system and rate at which images are presented to the surface - either immediate or mailbox 
	createInfo.clipped = VK_TRUE; // used to optimize cases where not all of the surface might be visible - we don't care about colour of pixels that are obscured by other windows
	createInfo.oldSwapchain = VK_NULL_HANDLE; //we need to keep a pointer to the old swap chain which might be invalidated when recreating the swap chain due to window resize events

	if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) { //if we did not make the swap chain successfully
		throw std::runtime_error("failed to create swap chain!"); //throw an error
	}

	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr); // get number of swap chain images in the swap chain object
	swapChainImages.resize(imageCount); //resize the array to hold all the images in the swap chain
	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data()); // load the swap chain images into memory
	swapChainImageFormat = surfaceFormat.format; //store a reference to the swap chain image format being used
	swapChainExtent = extent; //store a reference to the size of the swap chain images
}

/*
	helper method to setup the debug utils messenger create info
*/
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

/*
	setup the debug messenger instance
*/
void TriangleApp::setupDebugMessenger()
{
	if (!enableValidationLayers) return; //we don't want to debug / don't want the validation layers return and don't do any of the following setup
	VkDebugUtilsMessengerCreateInfoEXT createInfo; //information to create the debug messenger
	populateDebugMessengerInfo(createInfo); //populate the info with the correct setup information
	if (CreateDebugUtilsMessengerEXT(vkInstance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) { //create the instance by providing the vkInstance handle, create info struct containing setup parameters, null allocation call back and finally a variable to hold the instance handle
		throw std::runtime_error("failed to set up debug messenger!"); //throw an error if we are unsuccessful
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

	std::vector<VkExtensionProperties> availableExtensions(extensionCount); // resize the array such that we can allocate the information related to supported extensions
	//get the information by providing the physical device, null layer name, the number of extensions and an array to store the layer names in
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data()); 
	//convert the extensions we want to be available to strings
	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	//now iterate over them, crossing them off
	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName); //remove this extension from our list of wanted extensions if it is there
	}

	//return that we either have all the extensions we want (true) or that we are missing some (false)
	return requiredExtensions.empty(); 
}

/*
	helper function to check what the swap chain supports
*/
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

/*
	choose an appropriate format for the swap surface
*/
VkSurfaceFormatKHR TriangleApp::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	/*
		each available format is a struct that details the properties of each surface format
		notable properties are the colour channels and type where R8G8B8A8 is R, G, B and alpha represented as bytes
		and colour space which can be sRGB which is what is being used below
	*/
	for (const auto& availableFormat : availableFormats) { //for all available formats
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat; // return if the format satisfies RGBA and the colour space is sRGB non linear
		}
	}

	return availableFormats[0]; //default to the first format
}

/*
	helper function to choose the swap chain presentation mode
	The swapchain presentation mode determines how the windowing system is synchronized with images that are being presented to the surface
*/
VkPresentModeKHR TriangleApp::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	for (const auto& availablePresentMode : availablePresentModes) { //iterate through all presentation modes
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) { 
			//when an image is ready to be shown, it is marked as pending, and when the vertical blank is finished it is then show. However, if a new image is ready to be shown before this, the pending image is discarded and updated with this new one
			return availablePresentMode; //return this mode if it is available
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR; //otherwise use this one as the default (queue data structure where images are shown at regular intervals, after the vertical blank)
}

/*
	resolution of the swap chain images
	is almost always equal to the resolution of the window we are drawing to
	some window managers allow us to differ the resolution of what we are drawing
	by setting the currentExtent value to INT32_MAX
*/
VkExtent2D TriangleApp::chooseSwapExtent(const VkSurfaceCapabilitiesKHR & capabilities)
{
	if (capabilities.currentExtent.width != UINT32_MAX) { //if the current width of the window is not equal to the max value (the window manager is allowing us to define the extents)
		return capabilities.currentExtent; //return the surfaces current extent width and height
	}
	else { //we shall choose an appropriate value for the swapchain extent
		//get the width and height of the window
		int width, height; //width and height
		glfwGetFramebufferSize(window, &width, &height); //get the framebuffer width and height

		VkExtent2D actualExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) }; //create the extent struct to hold the width and height values

		//here we clamp the values of the width and height between the minimum and maximum extents allowed by the implementation
		actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent; //return the extents
	}
}

/*
	We cannot use the image resources in the swap chain directly
	We cannot, for example, directly use an image as an attachment to a framebuffer
	We create image views to hold additional information about the use of the image and use these as attachment to our Framebuffer
	An image view is a collection of properties and a reference to a parent image
*/
void TriangleApp::createImageViews()
{
	swapChainImageViews.resize(swapChainImages.size());	//set size of image views array to the size of images available in the swap chain
	//loop over all images in the swap chain and create an image view for each one
	for (size_t i = 0; i < swapChainImages.size(); i++) {
		VkImageViewCreateInfo createInfo = {}; //create info struct that will contain the information for setting up the image view
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO; //type of the struct - image view
		createInfo.image = swapChainImages[i]; //parent image of the view that will be created
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; //type of the view that will be created - can be 1d, 2d, and 3d images, or even cube maps and cube map array images, there are also 1d and 2d array images
		createInfo.format = swapChainImageFormat; //the format of the image, which will be the same as the swap chain format to ensure compatibility (this can be different however)

		//component ordering in the view may be different from that in the parent
		//each member of the components struct will refer to the child rgba components and how it should be interpreted from the parent image
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY; //read from the corresponding channel in the parent - because we are using the identity swizzle
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY; //read from the corresponding channel in the parent - because we are using the identity swizzle
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY; //read from the corresponding channel in the parent - because we are using the identity swizzle
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY; //read from the corresponding channel in the parent - because we are using the identity swizzle

		//describes what part of the image should be accessed - colour in this case, no mipmapping or multiple layers here
		//because this image view can be a subset of the parent image, the subset is specified in the following struct (subresourceRange)
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; //bitfield made up of the members of the VkImageAspectFlagBits specifying which aspects of the image are effected by the barrier (here we are only modifying the colour part, there are other parts, such as the stencil buffer)
		createInfo.subresourceRange.baseMipLevel = 0; //used to create an image view that corresponds to a certain part of the parents mip map chain
		createInfo.subresourceRange.levelCount = 1; //how many mip levels do we want to use
		createInfo.subresourceRange.baseArrayLayer = 0; //only used when the parent image is an array image, which in our case is not (how many layers do we want to use)
		createInfo.subresourceRange.layerCount = 1; //we only have one layer

		if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) { //create the image view by passing the logical device, create info setup struct, null allocation callbacks and the out parameter to hold the swapchain views, if not successful stop
			throw std::runtime_error("failed to create image views!"); //throw an error
		}
	}
}

/*
	create the graphics pipeline
	In Vulkan, a shader module is a collection of shader programs
	In order to make use of the shader programs they must be a part of a pipeline
	In Vulkan there are two kinds of pipelines, Compute and Graphics
	Here we are creating a graphics pipeline to make use of the shader programs
	The graphics pipeline can be viewed as an assembly line where commands to execute come in the front and colourful pixels are displayed at the end
	The graphics pipeline is highly customizable and so in this method we go about setting it up
*/
void TriangleApp::createGraphicsPipeline()
{
	//read in shader programs in binary format (pre compiled)
	auto vertShaderCode = readFile("../shaders/vert.spv");
	auto fragShaderCode = readFile("../shaders/frag.spv");

	//create shader modules using read in code
	VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
	VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

	//now that the shader modules have been created we need to assign them to specific stages in the pipeline
	VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO; //type of struct
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT; //pipeline stage we are assigning the module to
	vertShaderStageInfo.module = vertShaderModule; //shader module containing the code 
	vertShaderStageInfo.pName = "main"; //entry point to shader program

	//same thing but now for the fragment shader
	VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO; //type of the struct
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT; //stage we are assigning the shader program to
	fragShaderStageInfo.module = fragShaderModule; //the shader module containing the code
	fragShaderStageInfo.pName = "main"; //entry point to the program

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo }; // store the shader stages in an array

	//describing the configuration of the newly created pipeline vertex input state -  
	//what we are doing here is describing the layout of geometric data in memory and then having Vulkan fetch it and then feed it to the shader
	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO; //type of struct
	vertexInputInfo.vertexBindingDescriptionCount = 0; // number of vertex bindings used by the pipeline
	vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional - we are not binding any vertex information
	vertexInputInfo.vertexAttributeDescriptionCount = 0; //type of the attributes passed to the vertex shader, which binding to load them from and at which offset
	vertexInputInfo.pVertexAttributeDescriptions = nullptr; // also not binding any attributes for the vertices

	//this stage will take vertex input data and groups them into primitives ready for processing
	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO; //type of the struct
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; //type of the primitive that vertices will be grouped into, in this case a triangle
	inputAssembly.primitiveRestartEnable = VK_FALSE; //used to allow strips and fan primitives topologies to be cut and restarted (use for optimizing draw calls) - we don't need this

	//define the viewport (area to which we will render)
	VkViewport viewport = {};
	viewport.x = 0.0f; //origin
	viewport.y = 0.0f; //origin
	viewport.width = (float)swapChainExtent.width; //max width (here we are matching the swap chain width)
	viewport.height = (float)swapChainExtent.height; //max height (here we are matching the swap chain height)
	viewport.minDepth = 0.0f; //frame buffer depth values - we don't really use them at the moment
	viewport.maxDepth = 1.0f; //frame buffer depth values - we don't really use them at the moment

	//filter that discards pixels, we want to draw the entire image so we have a scissor angle to cover it entirely
	VkRect2D scissor = {}; //VkRect2D is a type that defines a rectangle in vulkan, it can be used for other things as well
	scissor.offset = { 0, 0 }; //screen offset (in our case it starts at the origin)
	scissor.extent = swapChainExtent; // the dimensions of the swap chain image (so here we are not discarding any pixels, it covers the full extent of the swap chain image)

	//combine the viewport and scissor configuration into viewport state
	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO; //type of the struct
	viewportState.viewportCount = 1; //number of view ports we want to use
	viewportState.pViewports = &viewport; //the viewport(s)
	viewportState.scissorCount = 1; //the number of scissors we want to use
	viewportState.pScissors = &scissor; //the scissor(s)
	
	//setup rasterization stage
	/*
		important stage that converts primitives into fragments that are going to be shaded by the fragment shader
	*/
	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO; 
	rasterizer.depthClampEnable = VK_FALSE; //fragments beyond the near and far plane are culled - (not needed here, this way we don't need to process these fragments)
	rasterizer.rasterizerDiscardEnable = VK_FALSE; //when this is enabled the rasterizer will not run
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL; //turns triangles into points or lines - in this case triangles are solid, filled in
	rasterizer.lineWidth = 1.0f; //thickness of lines
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT; //which faces should we cull (here we choose to cull back faces, we could also do both)
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE; //how to iterate over vertices (determines which faces are front and back) can be CC or C
	//the following parameters can be used to fix issues with z-fighting by allowing fragments to be offset in depth
	rasterizer.depthBiasEnable = VK_FALSE; // can be modified based on slope but we don't want that here so it is disabled
	rasterizer.depthBiasConstantFactor = 0.0f; // Optional - depth bias equation
	rasterizer.depthBiasClamp = 0.0f; // Optional - puts an upper bound on the depth bias equation output if positive and non zero and lower bound if non zero and negative
	rasterizer.depthBiasSlopeFactor = 0.0f; // Optional - param in depth bias equation
	//depth bias is calculated by finding m which is steepest descent in z direction and then multiplying it by depthBiasSlopeFactor and depthBiasConstantFactor

	//multisampling
	/*
		process of generating samples for each pixel in an image
		Anti-Aliasing is a for of multi-sampling, there are different kinds, MSAA...
	*/
	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO; //struct type
	multisampling.sampleShadingEnable = VK_FALSE; //disable MS on the shading
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT; //number of samples to use, use 1 sample in this case
	multisampling.minSampleShading = 1.0f; // Optional - minimum number of times the shader will be run per pixel (value is between 0 - 1, 1 means each pixel will receive its own data by another invocation of the frag shader)
	multisampling.pSampleMask = nullptr; // Optional - used to update only a subset of the samples produced (bitmaps)
	multisampling.alphaToCoverageEnable = VK_FALSE; // Optional - use the alpha channel to store coverage values which will be used for easy transparency
	multisampling.alphaToOneEnable = VK_FALSE; // Optional - what do we do with actual alpha values, set the alpha to one as if the fragment shader has not produced an alpha value

	//depth stencil testing not being used so its create info is omitted

	//colour-blending specification
	//configuration per colour attachment (we only have one colour attachment)
	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT; //the channels we are writing to
	if (BLEND) {
		colorBlendAttachment.blendEnable = VK_TRUE; //if we want to blend the colours
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA; //use the source image alpha channel to determine how much of src colours we use
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA; //use the destination image alpha channel to determine how much of dst colours we use
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; //add the two colours together
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; //use the source images alpha
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; //don't use the destination images alpha
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; //add the alphas together to determine final alpha of new image
	}else{
		colorBlendAttachment.blendEnable = VK_FALSE; //do we blend, not in this case,  we simply overwrite the contents of the buffer the parameters below are ignored
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional - we treat the src as opaque and replace everything in the dst
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional - we are not using any amount of the destination images colour
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional - we add the colours (essentially replacing them)
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional - use all of the source images alpha value
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional - don't use any of the destination images alpha
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional - add the alphas together to determine final alpha value
	}

	//references the array of structures for all of the framebuffers and allows you to set blend constants that you can use as blend factors in the blend calculations
	//final stage in the graphics pipeline is the colour blend stage
	//this is responsible for writing fragments into color attachments
	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO; //struct type
	colorBlending.logicOpEnable = VK_FALSE; //specifies if we should perform logical operation between the output of the frag shader and the colour attachment
	colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional - we have chosen not to do any operations so the data is written unmodified to the colour attachment from the frag shader
	colorBlending.attachmentCount = 1; //number of attachments, we only have 1 which the colour attachment
	colorBlending.pAttachments = &colorBlendAttachment; //the colour blend attachment
	//4 constants used for RGBA blending depending on blend factor
	colorBlending.blendConstants[0] = 0.0f; // Optional - R
	colorBlending.blendConstants[1] = 0.0f; // Optional - G
	colorBlending.blendConstants[2] = 0.0f; // Optional - B
	colorBlending.blendConstants[3] = 0.0f; // Optional - A

	//dynamic state - what parameters can we change at runtime (can be nullptr if we don't have any)
	VkDynamicState dynamicStates[] = {
		VK_DYNAMIC_STATE_VIEWPORT, //we would like to change the viewport dimensions
		VK_DYNAMIC_STATE_LINE_WIDTH //we would also like to change the line width on the fly
	};

	/*
		It is possible to make particular parts of the graphics pipeline dynamic
		this means that they can be updated using commands in side the command buffer rather than through an object
	*/
	VkPipelineDynamicStateCreateInfo dynamicState = {};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO; //type of struct
	dynamicState.dynamicStateCount = 2; //the number of states we wish to make dynamic
	dynamicState.pDynamicStates = dynamicStates; //the states

	//pipeline layout - specifies uniform layout information - which we are not using here so the struct is blank, but we still need to provide it
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO; //struct type
	pipelineLayoutInfo.setLayoutCount = 0; // Optional - number of different uniform layouts
	pipelineLayoutInfo.pSetLayouts = nullptr; // Optional - the uniform layouts
	pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional - a push constant is uniform variable in a shader and is used similarly, but it is vulkan owned and managed, it is set through the command buffer (number of push constant ranges)
	pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional - the push constant ranges that specify what stage of the pipeline will access the uniform and it also specifies the start offset and size of the uniform

	if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) { //create the pipeline layout by passing the logical device, the pipeline layout information, nullptr allocation callbacks and finally an out parameter to hold a handle to the pipeline layout, if not successful
		throw std::runtime_error("failed to create pipeline layout!"); //throw an error
	}

	//create graphics pipeline
	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO; //struct type
	pipelineInfo.stageCount = 2; //the number of stages we have (2, shader and frag)
	pipelineInfo.pStages = shaderStages; // array of shader stages

	pipelineInfo.pVertexInputState = &vertexInputInfo; //vertex stage
	pipelineInfo.pInputAssemblyState = &inputAssembly; //input assembly stage
	pipelineInfo.pViewportState = &viewportState; //view port state
	pipelineInfo.pRasterizationState = &rasterizer; //rasterizer stage
	pipelineInfo.pMultisampleState = &multisampling; //MS stage
	pipelineInfo.pDepthStencilState = nullptr; // Optional - depth stencil stage, we don't use this
	pipelineInfo.pColorBlendState = &colorBlending; //colour blending stage
	pipelineInfo.pDynamicState = nullptr; // Optional - the state which are treating as dynamic, we don't use this either
	pipelineInfo.layout = pipelineLayout; // pipeline layout, we are not using any uniforms and other constants in our pipeline
	pipelineInfo.renderPass = renderPass; // the render passes associating operations and images
	pipelineInfo.subpass = 0; // we are not using any subpasses
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional - vulkan allows you to derive from another pipeline
	pipelineInfo.basePipelineIndex = -1; // Optional - no base pipeline so init to -1 as per spec

	//more parameters are used here as multiple graphics pipelines can be created in one go by providing a list of create info structs
	//second param is a cache which can be used to reuse data relevant to pipeline creation across multiple class
	//the third param is the count of create info structs, in our case we only have one and only one pipeline is created
	if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) { //make the graphics pipeline
		throw std::runtime_error("failed to create graphics pipeline!"); //throw an error if it was unsuccessful
	}

	vkDestroyShaderModule(device, fragShaderModule, nullptr); //destroy the shader modules since they have been loaded in the pipeline
	vkDestroyShaderModule(device, vertShaderModule, nullptr); //destroy the shader modules since they have been loaded in the pipeline
}

/*
	helper function to create a shader module given compiled shader code
*/
VkShaderModule TriangleApp::createShaderModule(const std::vector<char>& code)
{
	VkShaderModuleCreateInfo createInfo = {}; //information needed to create a shader module
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO; //type of the create info is Shader module
	createInfo.codeSize = code.size(); //the size of the buffer that holds the byte code of our shader program
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data()); //reinterpret cast our char array to unit32_t (needs to be aligned for int32 but it is because we used a vec)
	VkShaderModule shaderModule; //out param
	//in order to make the shader module we need the logical device, the setup information (compiled code), (no allocator callback) and finally an out param to hold the created shader
	if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) { //make the shader module
		throw std::runtime_error("failed to create shader module!"); //if we are unsuccessful throw an error
	}
	return shaderModule; //return the shader module
}

//specify how many color and depth buffers there will be, how many samples to use for each of them and how their contents should be handled throughout the rendering operations.
/*
	in a graphics pipeline we render pixels into images that will either be further processed or presented to the user
	in very complex graphics pipelines these images will be generated only after many passes
	each pass can do something different, such as render UI, apply post-processing, ...
	these passes are encapsulated in a renderPass object, where each part of the render pass is called a subpass
	the render pass object will contain information about he final output image
*/
void TriangleApp::createRenderPass()
{
	/*
		VkAttachmentDescription structures that define the attachments associated with the renderpass.
		Each will structure define a single image (attachment) that will be used as an input, output, or both in one or more subpasses comprising this renderpass
		for graphics related application there will be at least 1 of these and in this case there will also only be one subpass
	*/
	VkAttachmentDescription colorAttachment = {}; //attachment information
	colorAttachment.format = swapChainImageFormat; //match the format of the swapchain
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT; //1 sample since we are not using any form of Anti Aliasing (AA)
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; //Clear the values to a constant at the start (what should we do when the render pass starts)
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; //Rendered contents will be stored in memory and can be read later (what to do when the render pass ends)
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; //don't do anything with stencil buffer (same as before but now for the depth part of the image, which we for now don't care about)
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; //don't do anything with stencil buffer (again, don't care about this part of the image)
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; //specifies which layout the image will have before the render pass begins
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // layout to automatically transition to when the render pass finishes. Images to be presented in the swap chain

	//Subpasses and attachment references
	/*
		single render pass can consist of multiple subpasses. 
		Subpasses are subsequent rendering operations that depend on the contents of framebuffers in previous passes, 
		for example a sequence of post-processing effects that are applied one after another
	*/
	/*
		An attachment reference is a structure containing an index into the array of attachments,
		and what kind of image layout that the attachment is expected to be in at this subpass
	*/
	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0; //which attachment are we referring to (here we only have 1 attachment at index 0)
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; //the output image the attachment is associated with
	//describe subpass - each subpass references a number of attachments, here we only want 1 subpass.
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; //bind point of a pipeline object to a command buffer (specifying as a graphics pipeline here, but it could be a compute pipeline)
	subpass.colorAttachmentCount = 1; //number of color attachments (this is where output is written)
	subpass.pColorAttachments = &colorAttachmentRef; //pointer into array containing the attachments (1 in this case so not an array, this is the output)
	//we don't really have input attachments at the moment, since the triangle we render is defined in the shader itself

	//create render pass
	VkRenderPassCreateInfo renderPassInfo = {}; //render pass information
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO; //type of the struct
	renderPassInfo.attachmentCount = 1; //number of attachments
	renderPassInfo.pAttachments = &colorAttachment; //an array containing the attachment information for the number of attachments specified (in this case 1 and its just the color attachment)
	renderPassInfo.subpassCount = 1; // the number of subpasses
	renderPassInfo.pSubpasses = &subpass; //the subpass creation info

	//define subpass
	//first two fields specify the indices of the dependency and the dependent subpass
	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL; //producer of data (in this case is external to this subpass)
	dependency.dstSubpass = 0; //this subpass is the destination of the data (that is how the dependency goes)
	// next two fields specify the operations to wait on and the stages in which these operations occur
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; //which pipeline stage of the source subpass produced the data
	dependency.srcAccessMask = 0; //how source subpass accesses the data
	//next two fields specify the operations to wait on and the stages in which these operations occur
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; //which stages of the destination subpass will consume the data. 
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; //how destination subpass accesses the data
	
	//setup the subpass in the renderpassinfo struct
	renderPassInfo.dependencyCount = 1; //we have 1 dependency
	renderPassInfo.pDependencies = &dependency; //the dependency info

	if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) { //make the render pass
		throw std::runtime_error("failed to create render pass!"); //if we were not successful throw an error
	}
}

/*
	create a framebuffer object to represent the set of images the graphics pipeline will render to.
	usually there are a minimum of two but in most cases there will be more, this is called a front buffer and
	a back buffer.
*/
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
		}; //getting the swap chain images

		VkFramebufferCreateInfo framebufferInfo = {}; //struct to hold framebuffer creation info
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO; //struct type
		framebufferInfo.renderPass = renderPass; //render pass we are binding framebuffer to
		framebufferInfo.attachmentCount = 1; //number of attachments
		framebufferInfo.pAttachments = attachments; //the images that we wish to manage in the frame buffer
		framebufferInfo.width = swapChainExtent.width; //the width of the image
		framebufferInfo.height = swapChainExtent.height; //the height of the image
		framebufferInfo.layers = 1; //number of layers in image array

		if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) { //create the frame buffer object
			throw std::runtime_error("failed to create framebuffer!"); //throw an error if we are unsuccessful in creating the buffer
		}
	}
}

/*
	in order to submit work to a queue, we need to create a command buffer, but before we can do that
	we need to create a command pool to manage the command buffers.
	command pools are used to manage the memory that is used to store buffers and command buffers
*/
void TriangleApp::createCommandPool()
{
	QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice); //get the queue families we wish to submit work to

	VkCommandPoolCreateInfo poolInfo = {}; //command pool creation info
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO; //struct type
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value(); //specifies the family of the queue to which command buffers allocated from this pool will be submitted to
	/*
	flags:
	VK_COMMAND_POOL_CREATE_TRANSIENT_BIT: Hint that command buffers are rerecorded with new commands very often (may change memory allocation behavior)
	VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT: Allow command buffers to be rerecorded individually, without this flag they all have to be reset together

	*/
	poolInfo.flags = 0; // Optional - we don't have any specific requirements at this point

	if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) { //create the pool
		throw std::runtime_error("failed to create command pool!"); //if we are unsuccessful throw an error
	}

}

/*
	a command buffer represents a sequence of commands that are recorded and stored in a buffer.
	this buffer after recording will then be submitted to a queue for execution (batch execution).
	the command buffer is allocated from a command pool. So we need to have setup a command pool before we can allocate command buffers
*/
void TriangleApp::createCommandBuffers()
{
	commandBuffers.resize(swapChainFramebuffers.size()); //create a buffer for each frame buffer
	VkCommandBufferAllocateInfo allocInfo = {}; //command buffer allocation info
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO; //struct type
	allocInfo.commandPool = commandPool; //the command pool from which we will allocate the buffer
	//a primary buffer can call a secondary buffer, allows 2-deep function calls for example
	//we don't need secondary command buffers
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; //specifies if the allocated command buffers are primary or secondary command buffers
	allocInfo.commandBufferCount = (uint32_t)commandBuffers.size(); //the number of command buffers (one for each frame buffer)

	if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) { //create the command buffers by providing the logical device, the allocation information and the out parameter to store the handles to the buffers
		throw std::runtime_error("failed to allocate command buffers!");//throw an error if we were unsuccessful
	}

	//begin recording commands for the command buffer ( we want to draw a triangle )
	for (size_t i = 0; i < commandBuffers.size(); i++) {//for all command buffers
		VkCommandBufferBeginInfo beginInfo = {}; //information needed to tell the command buffer to begin recording
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO; //struct type
		beginInfo.flags = 0; // Optional - specifies how we're going to use the command buffer
		beginInfo.pInheritanceInfo = nullptr; // Optional - relevant for secondary command buffers

		/*
			If the command buffer was already recorded once, then a call to vkBeginCommandBuffer will implicitly reset it.
			It's not possible to append commands to a buffer at a later time.
		*/
		if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS) {// begin recording
			throw std::runtime_error("failed to begin recording command buffer!"); //if we didn't successfully begin recording throw an error
		}

		VkRenderPassBeginInfo renderPassInfo = {}; //create info needed to begin a render a pass
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO; //struct type
		renderPassInfo.renderPass = renderPass; //render pass itself
		renderPassInfo.framebuffer = swapChainFramebuffers[i]; //the buffer we want to render to
		//size of the render area
		renderPassInfo.renderArea.offset = { 0, 0 }; //origin of the buffer
		renderPassInfo.renderArea.extent = swapChainExtent; //dimensions of the buffer - matches swap chain images
		//clear values to use for VK_ATTACHMENT_LOAD_OP_CLEAR, which we used as load operation for the color attachment
		VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
		renderPassInfo.clearValueCount = 1; //one clear value
		renderPassInfo.pClearValues = &clearColor; //clear value
		//begin render pass
		//command buffer to record the command to
		//specifies the details of the render pass we've just provided
		//controls how the drawing commands within the render pass will be provided (execute in primary or secondary command buffer)
		vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		//bind graphics pipeline - we supply the command buffer we wish to feed to the pipeline, where we want to bind, our pipeline is a graphics pipeline
		//so we bind it to the VK_PIPELINE_BIND_POINT_GRAPHICS and finally we provide the pipeline handle.
		vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
		/*		
			vkCmdDraw:
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
			throw std::runtime_error("failed to record command buffer!"); //throw an error if are unable to stop recording
		}
	}
}

/*
	create semaphores to sync program (host) with rendering (device)
	In vulkan there are three different kinds of synchronization primitives
	we have the Fence
		this is a medium-weight synchronization primitive that is used with commands that interact with the operating system
		this can be used to wait for the presentation full frame to the user, for example.
	we have Events
		these represent fine grained synchronization between operations occurring within the pipeline
	we have Semaphores
		these represent flags that can be atomically set and reset by the physical device, this means that the semaphore is viewed the same way
		across all queues
		they are used to control ownership of resources across different queues on a physical device, essentially synchronizing work happening on different queues that would be
		operating asynchronously
*/
void TriangleApp::createSyncObjects()
{
	//we want to make semaphores for signaling that an image has been acquired for rendering
	//and another to signal that rendering has finished and presentation can happen
		//we need to do this because presentation cannot occur if rendering has not completed
	//similar pattern to previous steps when creating semaphores
	VkSemaphoreCreateInfo semaphoreInfo = {}; //information needed to create a semaphore
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;// struct type
	imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT); //create an array to store a semaphore for each frame that is currently available for rendering
	renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT); //create an array to store a semaphore for each frame that is currently available for presenting

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		//semaphores are created by providing the logical device, the semaphore setup information, null allocation callback function, and the out parameter to store the handle
		if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS) {//create two semaphores one for each state for each frame buffer
			throw std::runtime_error("failed to create semaphores for a frame!"); //throw an error if either fails to be created
		}
	}

	//we now also have to make fence objects to sync CPU and GPU
	//we need to create fences so that we limit the number of frames that are being processes, so we do not over submit work to the queues
	//this solves a problem with rapidly growing memory usage due to the over-submitting of work
	inFlightFences.resize(MAX_FRAMES_IN_FLIGHT); //resize so there is a fence for each frame
	/*
		this variable below is used to keep track of which image is being used by an in-flight frame, 
		this is done so that we avoid rendering to an in-flight image when MAX_FRAMES_INFLIGHT is 
		higher than the number of available swap chain images or if vkAcquireNextImageKHR returns images out of order
	*/
	imagesInFlight.resize(swapChainImages.size(), VK_NULL_HANDLE); //resize so there is a fence for each image and init each element to NULL


	VkFenceCreateInfo fenceInfo = {}; //fence creation info
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO; //struct type
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; //we need this to be set to signaled so we can render on the very first pass (they are otherwise initialized to a not signaled state and we wait for ever)
	
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) { //for each frame
		if (vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) { //create a fence by providing the logical device, fence setup information, null allocation callback function and the out parameter to store the handle to the fence
			throw std::runtime_error("failed to create fence for a frame!"); //if it is unsuccessful throw an error
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

/*
	method used to recreate the swap chain
	this is used whenever the window is resized
*/
void TriangleApp::recreateSwapChain()
{
	//handle minimization events
	//we basically wait till the window is in the foreground again
	//this can cause an error where the width and height of the window is 0 which are invalid swap chain params
	int width = 0, height = 0;
	glfwGetFramebufferSize(window, &width, &height);
	while (width == 0 || height == 0) { //while the buffer size is 0
		glfwGetFramebufferSize(window, &width, &height); //get the buffer size
		glfwWaitEvents(); //wait for more events
	}

	//wait for device to finish what it is doing
	vkDeviceWaitIdle(device);

	cleanupSwapChain(); //destroy the old swap chain and related resources

	//start to recreate the swap chain with new parameters
	createSwapChain(); //create a new swap chain with the new window width and height
	createImageViews(); //create new image views
	createRenderPass(); //create a new render pass
	createGraphicsPipeline(); //create a new graphics pipeline
	createFramebuffers(); //create a new framebuffer
	createCommandBuffers(); //create new command buffers (we recycle the command pool)
}

/*
	destroy the swap chain and its related resources
*/
void TriangleApp::cleanupSwapChain()
{
	for (size_t i = 0; i < swapChainFramebuffers.size(); i++) { //for all the frame buffers created to manage the images in the swap chain
		vkDestroyFramebuffer(device, swapChainFramebuffers[i], nullptr); //destroy them
	}

	//do this so we don't need to allocate and new command pool, we can reuse the old one to issue new command buffers
	//we need to provide the logical device, the pool from which we allocated the buffers and the buffers themselves
	vkFreeCommandBuffers(device, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data()); //free the command buffers

	//destroy the pipeline by providing the logical device and the pipeline handle
	vkDestroyPipeline(device, graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(device, pipelineLayout, nullptr); //destroy any uniforms allocated by destroying the layout, provide the logical device and the pipeline layout handle
	vkDestroyRenderPass(device, renderPass, nullptr); //destroy the render pass by providing the logical device and the render pass handle

	for (size_t i = 0; i < swapChainImageViews.size(); i++) {
		vkDestroyImageView(device, swapChainImageViews[i], nullptr); //destroy all image views by providing the logical device and swap chain image views handle
	}

	vkDestroySwapchainKHR(device, swapChain, nullptr); //finally, destroy the swap chain by providing the logical device and the swap chain handle
}

/*
	draw triangles
	Acquire an image from the swap chain
	Execute the command buffer with that image as attachment in the framebuffer
	Return the image to the swap chain for presentation
*/
void TriangleApp::drawFrame()
{
	//before we start drawing again, we have to wait for the previous frame to finish
	//vkWaitForFences takes an array of fences and waits for either any, or all of them to be signaled before returning
	//the last parameter is a timeout which we have disabled (so we wait forever, if the frame is never finishing) by setting it to uint64 max value
	vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX); //provide a logical device, the number of frames to wait on and the array of frames, a boolean if we want to wait on all of the fences

	uint32_t imageIndex; //variable to hold image index we will use to render to

	//logical device, swap chain, timeout (disabled in this case) to wait for image, imageAvailable semaphore to signal that we can start drawing, 
	//finally variable to hold image (used to get right command buffer to submit)
	VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

	//if the swap chain turns out to be out of date then we have to recreate the swap chain and continue in the next call
	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		recreateSwapChain();
		return;
	}

	//if the result is either a success or suboptimal value we proceed, if it is anything else, something has gone wrong and we throw an error
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	// Check if a previous frame is using this image (i.e. there is its fence to wait on)
	if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
		vkWaitForFences(device, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
	}
	// Mark the image as now being in use by this frame
	imagesInFlight[imageIndex] = inFlightFences[currentFrame];

	VkSubmitInfo submitInfo = {}; //information needed to submit a queue for execution
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO; //struct type

	VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] }; //semaphore we have to wait on to commence execution
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT }; //the stage we are waiting on to be available
	submitInfo.waitSemaphoreCount = 1; //the number of semaphores we are waiting on
	submitInfo.pWaitSemaphores = waitSemaphores; //the semaphore(s) we are waiting on
	submitInfo.pWaitDstStageMask = waitStages; //which stage(s) are waiting

	//command buffer we are submitting
	submitInfo.commandBufferCount = 1; //number of command buffers we are submitting
	submitInfo.pCommandBuffers = &commandBuffers[imageIndex]; //the command buffer(s) to submit, use imageIndex from earlier to find the right one

	//which semaphore should we use to signal that rendering is complete
	VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
	submitInfo.signalSemaphoreCount = 1; //the number of semaphores we should signal when complete
	submitInfo.pSignalSemaphores = signalSemaphores; //the semaphore we will use to signal that rendering is complete

	vkResetFences(device, 1, &inFlightFences[currentFrame]); //unlike with semaphores, we need to manually restore the fence to the original state

	//the graphics queue that will receive the  work to execute, the submit info which describes the work, the number of submissions (we can make multiple submissions in one go).
	//The last parameter references an optional fence that will be signaled when the command buffers finish execution (CPU-GPU sync).
	if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) { //submit the queue
		throw std::runtime_error("failed to submit draw command buffer!"); //throw an error if could not submit it
	}


	VkPresentInfoKHR presentInfo = {}; //information needed to present the framebuffer
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR; //struct type
	// first two parameters specify which semaphores to wait on before presentation can happen,
	presentInfo.waitSemaphoreCount = 1; //the number of semaphores to wait on before presenting - this is so that we present a fully rendered image and not anything else
	presentInfo.pWaitSemaphores = signalSemaphores; //semaphore(s) - the semaphores we are going to wait on
	//next two parameters specify the swap chains to present images to and the index of the image for each swap chain
	VkSwapchainKHR swapChains[] = { swapChain }; //swap chain we are using to present the images
	presentInfo.swapchainCount = 1; //the number of swap chains - in this case we only have one swap chain which we are using to present
	presentInfo.pSwapchains = swapChains; //the swap chains that will do the presenting
	presentInfo.pImageIndices = &imageIndex; //the image index we are going to present 

	presentInfo.pResults = nullptr; // Optional allows you to specify an array of VkResult values to check for every individual swap chain if presentation was successful

	VkResult result1 = vkQueuePresentKHR(presentationQueue, &presentInfo); //submits the request to present an image to the swap chain
	
	//we have to check the same conditions here and recreate the swapchain if we need to (window management)
	if (result1 == VK_ERROR_OUT_OF_DATE_KHR || result1 == VK_SUBOPTIMAL_KHR || framebufferResized) {
		//if the result of presentation is either a out of date or suboptimal or we have a window resize event we need to recreate the swap chain
		framebufferResized = false; //reset the window resize flag
		recreateSwapChain(); //recreate the swap chain
	}
	else if (result1 != VK_SUCCESS) { //otherwise we have not successfully presented the image
		throw std::runtime_error("failed to present swap chain image!"); //throw an error
	}
	
	//increment the frame we're rendering
	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT; //increment to the next frame to render to (circular as we are using the modulo)
}

/*
	simple method to read in files
	used in our app to read in SPIR-V shader files
*/
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

/*
	helper method to check layer support
	In this method we check if the instance supports the validation layer
*/
bool TriangleApp::checkValidationLayerSupport()
{
	uint32_t layerCount = 0; //variable to store the number of supported layers by the instance
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr); //get the number of supported layers by passing nullptr as the out param

	std::vector<VkLayerProperties> availableLayers(layerCount); //an array to store all the available layers
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data()); // store the available layers by passing an out parameter

	for (const char * layerName : validationLayers) { //for each layer we want to use
		bool layerFound = false; //is the layer supported?
		for (const auto& layerProperties : availableLayers) { //for all available layers in the instance
			if (strcmp(layerName, layerProperties.layerName) == 0) { //are they the same?
				layerFound = true; //if so, we found the layer we want to use
				break; //stop searching
			}
		}
		if (!layerFound) {
			return false; //we did not find the layer
		}
	}
	return true; //we found the layer
}

/*
	static method to be used with GLFW to catch window resize events and execute code when it happens
*/
void TriangleApp::framebufferResizeCallback(GLFWwindow * window, int width, int height)
{
	auto app = reinterpret_cast<TriangleApp*>(glfwGetWindowUserPointer(window)); //get a pointer to the app instance
	app->framebufferResized = true; //we resized the window
}
