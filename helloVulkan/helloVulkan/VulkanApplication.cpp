#include "VulkanApplication.h"
#include "CpuTimer.h"

#include <stdexcept>
#include <iostream>
#include <map>
#include <set>
#include <algorithm>

const int WIDTH = 800;
const int HEIGHT = 600;

const std::vector<Vertex> g_vertices = {
	{ { 0.0f, -0.5f, 0.f }, { 1.0f, 0.0f, 0.0f } },
	{ { 0.5f, 0.5f, 0.f }, { 0.0f, 1.0f, 0.0f } },
	{ { -0.5f, 0.5f, 0.f }, { 0.0f, 0.0f, 1.0f } }
};

VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const	VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback)
{
	auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
	if (func != nullptr)
	{
		return func(instance, pCreateInfo, pAllocator, pCallback);
	}
	else
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
	if (func != nullptr)
	{
		func(instance, callback, pAllocator);
	}
}

VulkanApplication::VulkanApplication()
	: window_(0)
	, instance_(0)
{
}


VulkanApplication::~VulkanApplication()
{
}

/*
*1.	初始化windowx相关（与vulkan无关部分）
*2.	vulkan初始化
*3.	frame主循环
*4.	资源回收与释放
*/
void VulkanApplication::run()
{
	initWindow(800,600);
	initVulkan();
	frame();
	uninit();
}

/*
*1.	showVkExtensions
*2.	vkCreateInstance
*3.	createSurface
*4.	选择一个物理GPU设备
*5.	创建一个逻辑设备与物理设备交互
*6.	创建交换链
*7.	为每个Image创建ImageView
*8.	创建RenderPass
*9.	创建图形管线
*10.创建Framebuffer,为每个VkImage创建对应的VkFragment对象，其attachments指向VkImageView
*11.创建commandpool来缓存command
*12.创建vectexBuffer
*13.创建commandbuffer
*14.创建信号量Semaphore, 由于绘图和交换的操作的都是异步的，这里需要创建信号量来指示可用的image和渲染完成的状态
*/
void VulkanApplication::initVulkan()
{
	showVkExtensions();
	createInstance();

	//setupDebugCallback();

	createSurface();

	pickPhtsicalDevice();
	//在选择要使用的物理设备之后，需要设置一个逻辑设备用于交互
	createLogicalDevice();

	createSwapChain();

	createImagesViews();

	createRenderPass();

	createGraphPipeline();

	createFramebuffer();

	createCommandPool();

	createVertexBuffer();

	createCommandBuffers();

	createSemaphores();
}

void VulkanApplication::initWindow(int width,int height)
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	window_ = glfwCreateWindow(width, height, "Vulkan", nullptr, nullptr);

	glfwSetWindowUserPointer(window_, this);
	glfwSetWindowSizeCallback(window_, VulkanApplication::onWindowResized);
}

void VulkanApplication::frame()
{
	while (!glfwWindowShouldClose(window_))
	{
		//CpuTimer ct;
		glfwPollEvents();
		drawFrame();
		//ct.Print();
	}

	vkDeviceWaitIdle(device_);
}

void VulkanApplication::uninit()
{
	cleanupSwapChain();

	vkDestroyBuffer(device_, vertexBuffer_, nullptr);
	vkFreeMemory(device_, vertexBufferMemory_, nullptr);

	vkDestroySemaphore(device_, renderFinishedSemaphore_, nullptr);
	vkDestroySemaphore(device_, imageAvailableSemaphore_, nullptr);

	vkDestroyCommandPool(device_, commandPool_, nullptr);


	vkDestroyDevice(device_, nullptr);
	//DestroyDebugReportCallbackEXT(instance_, callback_, nullptr);
	vkDestroySurfaceKHR(instance_, surface_, nullptr);

	vkDestroyInstance(instance_, nullptr);

	glfwDestroyWindow(window_);

	glfwTerminate();
}

void VulkanApplication::createInstance()
{
	//if (enableValidationLayers && !checkValidationLayerSupport())
	//{
	//	throw std::runtime_error("validation layers requested, but not available!");
	//}

	//vulkanApp的信息
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = nullptr;
	appInfo.pApplicationName = "Hello Vulkan";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	//VkInstanceCreateInfo
	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	unsigned int glfwExtensionCount = 0;
	const char** glfwExtensions;

	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	createInfo.enabledExtensionCount = glfwExtensionCount;
	createInfo.ppEnabledExtensionNames = glfwExtensions;

	createInfo.enabledLayerCount = 0;

	//uint32_t extensionCount = 0;
	//vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	//std::vector<VkExtensionProperties> extensions(extensionCount);
	//vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

	//std::cout << "available extensions:" << std::endl;
	//for (const auto& extension : extensions) {
	//	std::cout << "\t" << extension.extensionName << std::endl;
	//}

	//auto extensions = getRequiredExtensions();
	//createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	//createInfo.ppEnabledExtensionNames = extensions.data();
	//if (enableValidationLayers)
	//{
	//	createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
	//	createInfo.ppEnabledLayerNames = validationLayers.data();
	//}
	//else
	//{
	//	createInfo.enabledLayerCount = 0;
	//}
	
	VkResult res = vkCreateInstance(&createInfo, nullptr, &instance_);
	if (res != VK_SUCCESS)
		throw std::runtime_error("failed to create instance!");
}

void VulkanApplication::showVkExtensions()
{
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

	std::cout << "available extensions:" << std::endl;
	for (const auto& extension : extensions) {
		std::cout << "\t" << extension.extensionName << std::endl;
	}
}

std::vector<const char*> VulkanApplication::getRequiredExtensions()
{
	std::vector<const char*> extensions;

	unsigned int glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	for (unsigned int i = 0; i < glfwExtensionCount; i++) {
		extensions.push_back(glfwExtensions[i]);
	}

	if (enableValidationLayers) {
		extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	}

	return extensions;
}

bool VulkanApplication::checkValidationLayerSupport() 
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : c_validationLayers) {
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

/*
*1.	枚举出所有的物理GPU device
*2.	遍历list,找出一个程序最合适的device
*/
void VulkanApplication::pickPhtsicalDevice()
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance_, &deviceCount, nullptr);
	if (deviceCount == 0)
	{
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance_, &deviceCount, devices.data());

	// Use an ordered map to automatically sort candidates by increasing score
	std::multimap<int, VkPhysicalDevice> candidates;
	for (const auto& device : devices)
	{
		int score = rateDeviceSuitability(device);
		candidates.insert(std::make_pair(score, device));
	}
	// Check if the best candidate is suitable at all
	if (candidates.rbegin()->first > 0) {
		physicalDevice_ = candidates.rbegin()->second;
	}
	else {
		throw std::runtime_error("failed to find a suitable GPU!");
	}

	//for (const auto& device : devices)
	//{
	//	if (isDeviceSuitable(device))
	//	{
	//		physicalDevice_ = device;
	//		break;
	//	}
	//}

	if (physicalDevice_ == VK_NULL_HANDLE)
		throw std::runtime_error("failed to find a suitable GPU!");
}

bool VulkanApplication::isDeviceSuitable(VkPhysicalDevice device)
{
	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	QueueFamilyIndices indices = findQueueFamilies(device);

	bool result = true;

	result &= indices.isComplete();
	result &= deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && deviceFeatures.geometryShader;

	bool extensionsSupported = checkDeviceExtensionSupport(device);

	result &= extensionsSupported;

	bool swapChainAdequate = false;
	if (extensionsSupported) {
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();

		result &= swapChainAdequate;
	}
	
	return result;
}

int VulkanApplication::rateDeviceSuitability(VkPhysicalDevice device)
{
	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	int score = 0;

	// Discrete GPUs have a significant performance advantage
	if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) 
	{
		score += 1000;
	}

	// Maximum possible size of textures affects graphics quality
	score += deviceProperties.limits.maxImageDimension2D;

	// Application can't function without geometry shaders
	if (!deviceFeatures.geometryShader) 
	{
		return 0;
	}

	return score;
}

QueueFamilyIndices VulkanApplication::findQueueFamilies(VkPhysicalDevice device)
{
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies)
	{
		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.graphicsFamily = i;
		}

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface_, &presentSupport);

		if (queueFamily.queueCount > 0 && presentSupport) {
			indices.presentFamily = i;
		}

		if (indices.isComplete())
		{
			break;
		}

		i++;
	}

	return indices;
}

/*
*1.	获取物理设备上的可用的队列信息,findQueueFamilies
*2.	vkCreateDevice
*3.	获取图形队列句柄、呈现队列句柄
*/
void VulkanApplication::createLogicalDevice()
{
	QueueFamilyIndices indices = findQueueFamilies(physicalDevice_);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<int> uniqueQueueFamilies = { indices.graphicsFamily, indices.presentFamily };

	float queuePriority = 1.0f;
	for (int queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}
	
	VkPhysicalDeviceFeatures deviceFeatures = {};

	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();

	createInfo.pEnabledFeatures = &deviceFeatures;

	createInfo.enabledExtensionCount = static_cast<uint32_t>(c_deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = c_deviceExtensions.data();

	if (enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(c_validationLayers.size());
		createInfo.ppEnabledLayerNames = c_validationLayers.data();
	}
	else {
		createInfo.enabledLayerCount = 0;
	}

	if (vkCreateDevice(physicalDevice_, &createInfo, nullptr, &device_) != VK_SUCCESS) {
		throw std::runtime_error("failed to create logical device!");
	}

	vkGetDeviceQueue(device_, indices.graphicsFamily, 0, &graphicsQueue_);
	vkGetDeviceQueue(device_, indices.presentFamily, 0, &presentQueue_);
}

void VulkanApplication::createSurface()
{
#ifdef VK_USE_PLATFORM_WIN32_KHR
	VkWin32SurfaceCreateInfoKHR createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	createInfo.hwnd = glfwGetWin32Window(window_);
	createInfo.hinstance = GetModuleHandle(nullptr);

	auto CreateWin32SurfaceKHR = (PFN_vkCreateWin32SurfaceKHR)vkGetInstanceProcAddr(instance, "vkCreateWin32SurfaceKHR");

	if (!CreateWin32SurfaceKHR || CreateWin32SurfaceKHR(instance, &createInfo, nullptr, &surface) != VK_SUCCESS) {
		throw std::runtime_error("failed to create window surface!");
	}
#else
	if (glfwCreateWindowSurface(instance_, window_, nullptr, &surface_) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create window surface!");
	}
#endif
}

/*
*1.	querySwapChainSupport,查询物理设备对交换链的支持详情
*2.	vkCreateSwapchainKHR，创建交换链
*3.	vkGetSwapchainImagesKHR，获取交换链所包含的image引用
*/
void VulkanApplication::createSwapChain()
{
	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice_);

	//选择（设置）surface的像素格式
	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	//选择（设置）surface呈现模式
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	//选择（设置）swapsurface的size
	VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
	{
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface_;

	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = findQueueFamilies(physicalDevice_);
	uint32_t queueFamilyIndices[] = { (uint32_t)indices.graphicsFamily, (uint32_t)indices.presentFamily };

	if (indices.graphicsFamily != indices.presentFamily) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;	//并发模式 //图像可以被多个队列簇访问，不需要明确所有权从属关系。
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;	//独占模式 //同一时间图像只能被一个队列簇占用，如果其他队列簇需要其所有权需要明确指定。这种方式提供了最好的性能。
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;

	createInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(device_, &createInfo, nullptr, &swapChain_) != VK_SUCCESS) {
		throw std::runtime_error("failed to create swap chain!");
	}

	vkGetSwapchainImagesKHR(device_, swapChain_, &imageCount, nullptr);
	swapChainImages_.resize(imageCount);
	vkGetSwapchainImagesKHR(device_, swapChain_, &imageCount, swapChainImages_.data());

	swapChainImageFormat_ = surfaceFormat.format;
	swapChainExtent_ = extent;
}

bool VulkanApplication::checkDeviceExtensionSupport(VkPhysicalDevice device)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(c_deviceExtensions.begin(), c_deviceExtensions.end());

	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

SwapChainSupportDetails VulkanApplication::querySwapChainSupport(VkPhysicalDevice device)
{
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface_, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &formatCount, nullptr);

	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &formatCount, details.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &presentModeCount, nullptr);

	if (presentModeCount != 0) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &presentModeCount, details.presentModes.data());
	}

	return details;
}

VkSurfaceFormatKHR VulkanApplication::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED) {
		return{ VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	}

	for (const auto& availableFormat : availableFormats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat;
		}
	}

	return availableFormats[0];
}

VkPresentModeKHR VulkanApplication::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes)
{
	VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

	for (const auto& availablePresentMode : availablePresentModes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return availablePresentMode;
		}
		else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
			bestMode = availablePresentMode;
		}
	}

	return bestMode;
}

VkExtent2D VulkanApplication::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return capabilities.currentExtent;
	}
	else {
		int width, height;
		glfwGetWindowSize(window_, &width, &height);

		VkExtent2D actualExtent = { width, height };

		actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}

void VulkanApplication::createImagesViews()
{
	swapChainImageViews_.resize(swapChainImages_.size());

	for (size_t i = 0; i < swapChainImages_.size(); i++)
	{
		VkImageViewCreateInfo createInfo;
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = swapChainImages_[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = swapChainImageFormat_;
		//components字段允许调整颜色通道的最终的映射逻辑
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		//subresourceRangle 字段用于描述图像的使用目标是什么，以及可以被访问的有效区域;图像将会作为color targets，没有任何mipmapping levels 或是多层 multiple layers
		//如果在编写沉浸式的3D应用程序，比如VR，就需要创建支持多层的交换链。并且通过不同的层为每一个图像创建多个视图，以满足不同层的图像在左右眼渲染时对视图的需要
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(device_, &createInfo, nullptr, &swapChainImageViews_[i]))
		{
			throw std::runtime_error("failed to create image view!");
		}
	}
}

void VulkanApplication::createGraphPipeline()
{
	//shader
	auto vertShaderCode = readFile("shaders/vert.spv");
	auto fragShaderCode = readFile("shaders/frag.spv");

	VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
	VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	//vertex
	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	auto bindingDescription = Vertex::getBindingDescription();
	auto attributeDescriptions = Vertex::getAttributeDescriptions();

	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	//vertex assembly(顶点装配)
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
	inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

	//viewport
	VkViewport viewport{};
	viewport.x = 0.f;
	viewport.y = 0.f;
	viewport.width = (float)swapChainExtent_.width;
	viewport.height = (float)swapChainExtent_.height;
	viewport.minDepth = 0.f;
	viewport.maxDepth = 1.f;
	//scissor(裁剪矩形)
	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = swapChainExtent_;
	VkPipelineViewportStateCreateInfo viewportInfo{};
	viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportInfo.viewportCount = 1;
	viewportInfo.pViewports = &viewport;
	viewportInfo.scissorCount = 1;
	viewportInfo.pScissors = &scissor;
	
	//Rasterizer(光栅化，设置填充模式)
	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;	//若设置为VK_TRUE，超过远近裁剪面的片元会进行收敛，而不是丢弃它们
	rasterizer.rasterizerDiscardEnable = VK_FALSE;	//若设置为VK_TRUE，那么几何图元永远不会传递到光栅化阶段。这是基本的禁止任何输出到framebuffer帧缓冲区的方法。
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;	//填充模式
	rasterizer.lineWidth = 1.0f;	//任何大于1.0的线宽需要开启GPU的wideLines特性支持。
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;	//描述front-facing面的顶点的顺序

	rasterizer.depthBiasEnable = VK_FALSE;	//若设置为VK_TRUE，光栅化可以通过添加常量或者基于片元的斜率来更改深度值。一些时候对于阴影贴图是有用的
	rasterizer.depthBiasConstantFactor = 0.0f; // Optional
	rasterizer.depthBiasClamp = 0.0f; // Optional
	rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

	//Multisampling
	VkPipelineMultisampleStateCreateInfo multisamplingInfo = {};
	multisamplingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisamplingInfo.sampleShadingEnable = VK_FALSE;
	multisamplingInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisamplingInfo.minSampleShading = 1.0f; // Optional
	multisamplingInfo.pSampleMask = nullptr; // Optional
	multisamplingInfo.alphaToCoverageEnable = VK_FALSE; // Optional
	multisamplingInfo.alphaToOneEnable = VK_FALSE; // Optional

	//Depth and stencil testing

	//Color blending
	//有两个结构体用于配置颜色混合。第一个结构体VkPipelineColorBlendAttachmentState包括了每个附加到帧缓冲区的配置。第二个结构体VkPipelineColorBlendStateCreateInfo包含了全局混色的设置
	//伪代码说明，和gl保持逻辑一致
	//if (blendEnable) {
	//	finalColor.rgb = (srcColorBlendFactor * newColor.rgb) < colorBlendOp > (dstColorBlendFactor * oldColor.rgb);
	//	finalColor.a = (srcAlphaBlendFactor * newColor.a) < alphaBlendOp > (dstAlphaBlendFactor * oldColor.a);
	//}
	//else {
	//	finalColor = newColor;
	//}
	//finalColor = finalColor & colorWriteMask;
	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	//Pipeline layout(uniform)
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0; // Optional
	pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
	pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
	pipelineLayoutInfo.pPushConstantRanges = 0; // Optional
	if (vkCreatePipelineLayout(device_, &pipelineLayoutInfo, nullptr, &pipelineLayout_) != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}

	//GraphicsPipeline
	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
	pipelineInfo.pViewportState = &viewportInfo;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisamplingInfo;
	pipelineInfo.pDepthStencilState = nullptr; // Optional
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = nullptr; // Optional
	pipelineInfo.layout = pipelineLayout_;
	pipelineInfo.renderPass = renderPass_;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = -1;

	if (vkCreateGraphicsPipelines(device_, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline_) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	vkDestroyShaderModule(device_, fragShaderModule, nullptr);
	vkDestroyShaderModule(device_, vertShaderModule, nullptr);
}

/*
*vkCreateShaderModule
*/
VkShaderModule VulkanApplication::createShaderModule(const std::vector<char>& code)
{
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();

	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(device_, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("failed to create shader module!");
	}

	return shaderModule;
}

/*
*vkCreateRenderPass 需要colorAttachment, subpass
*/
void VulkanApplication::createRenderPass()
{
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = swapChainImageFormat_;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 0;
	renderPassInfo.pDependencies = nullptr;

	if (vkCreateRenderPass(device_, &renderPassInfo, nullptr, &renderPass_) != VK_SUCCESS) {
		throw std::runtime_error("failed to create render pass!");
	}
}

void VulkanApplication::createFramebuffer()
{
	swapChainFramebuffers_.resize(swapChainImageViews_.size());
	for (size_t i = 0; i < swapChainFramebuffers_.size(); i++)
	{
		VkImageView attachments[] = { swapChainImageViews_[i] };

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass_;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = swapChainExtent_.width;
		framebufferInfo.height = swapChainExtent_.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(device_, &framebufferInfo, nullptr, &swapChainFramebuffers_[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
}

void VulkanApplication::createCommandPool()
{
	QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice_);

	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
	poolInfo.flags = 0;	//Optional

	if (vkCreateCommandPool(device_, &poolInfo, nullptr, &commandPool_) != VK_SUCCESS) {
		throw std::runtime_error("failed to create command pool!");
	}
}

void VulkanApplication::createCommandBuffers()
{
	commandBuffers_.resize(swapChainFramebuffers_.size());

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool_;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)commandBuffers_.size();

	if (vkAllocateCommandBuffers(device_, &allocInfo, commandBuffers_.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate command buffers!");
	}

	for (size_t i = 0; i < commandBuffers_.size(); i++) {
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;	//命令缓冲区也可以重新提交，同时它也在等待执行

		vkBeginCommandBuffer(commandBuffers_[i], &beginInfo);	//开始记录

		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass_;
		renderPassInfo.framebuffer = swapChainFramebuffers_[i];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = swapChainExtent_;

		VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(commandBuffers_[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);	//开启渲染通道

		vkCmdBindPipeline(commandBuffers_[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline_);	//绑定图形管线graphicsPipeline_

		VkBuffer vertexBuffers[] = { vertexBuffer_ };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffers_[i], 0, 1, vertexBuffers, offsets);	//绑定顶点

		vkCmdDraw(commandBuffers_[i], static_cast<uint32_t>(g_vertices.size()), 1, 0, 0);	//绘制命令
		//vkCmdDraw(commandBuffers_[i], 3, 1, 0, 0);

		vkCmdEndRenderPass(commandBuffers_[i]);	//结束渲染cmd
		
		//结束记录
		if (vkEndCommandBuffer(commandBuffers_[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}
	}
}

void VulkanApplication::createSemaphores()
{
	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	if (vkCreateSemaphore(device_, &semaphoreInfo, nullptr, &imageAvailableSemaphore_) != VK_SUCCESS ||
		vkCreateSemaphore(device_, &semaphoreInfo, nullptr, &renderFinishedSemaphore_) != VK_SUCCESS) {

		throw std::runtime_error("failed to create semaphores!");
	}
}

/*
*1.	从交换链中获取一个图像
*2.	在帧缓冲区中，使用作为附件的图像来执行命令缓冲区中的命令
*3.	为了最终呈现，将图像返还到交换链
*/
void VulkanApplication::drawFrame()
{
	//从交换链中获取一个图像	
	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(device_, swapChain_, std::numeric_limits<uint64_t>::max(), imageAvailableSemaphore_, VK_NULL_HANDLE, &imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		recreateSwapChain();
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	//在帧缓冲区中，使用作为附件的图像来执行命令缓冲区中的命令
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { imageAvailableSemaphore_ };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffers_[imageIndex];

	VkSemaphore signalSemaphores[] = { renderFinishedSemaphore_ };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (vkQueueSubmit(graphicsQueue_, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { swapChain_ };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;

	presentInfo.pImageIndices = &imageIndex;

	//提交请求呈现交换链中的图像
	vkQueuePresentKHR(presentQueue_, &presentInfo);
	//等待presentQueue_操作完成。
	vkQueueWaitIdle(presentQueue_);
}

void VulkanApplication::recreateSwapChain()
{
	vkDeviceWaitIdle(device_);

	cleanupSwapChain();

	createSwapChain();
	createImagesViews();
	createRenderPass();
	createGraphPipeline();
	createFramebuffer();
	createCommandBuffers();
}

void VulkanApplication::cleanupSwapChain()
{
	for (size_t i = 0; i < swapChainFramebuffers_.size(); i++) {
		vkDestroyFramebuffer(device_, swapChainFramebuffers_[i], nullptr);
	}

	vkFreeCommandBuffers(device_, commandPool_, static_cast<uint32_t>(commandBuffers_.size()), commandBuffers_.data());

	vkDestroyPipeline(device_, graphicsPipeline_, nullptr);
	vkDestroyPipelineLayout(device_, pipelineLayout_, nullptr);
	vkDestroyRenderPass(device_, renderPass_, nullptr);

	for (size_t i = 0; i < swapChainImageViews_.size(); i++) {
		vkDestroyImageView(device_, swapChainImageViews_[i], nullptr);
	}

	vkDestroySwapchainKHR(device_, swapChain_, nullptr);
}

void VulkanApplication::createVertexBuffer2()
{
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = sizeof(g_vertices[0]) * g_vertices.size();
	bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(device_, &bufferInfo, nullptr, &vertexBuffer_) != VK_SUCCESS) {
		throw std::runtime_error("failed to create vertex buffer!");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device_, vertexBuffer_, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	if (vkAllocateMemory(device_, &allocInfo, nullptr, &vertexBufferMemory_) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate vertex buffer memory!");
	}

	vkBindBufferMemory(device_, vertexBuffer_, vertexBufferMemory_, 0);

	void* data;
	vkMapMemory(device_, vertexBufferMemory_, 0, bufferInfo.size, 0, &data);
	memcpy(data, g_vertices.data(), (size_t)bufferInfo.size);
	vkUnmapMemory(device_, vertexBufferMemory_);
}

/*
*1.	createBuffer，包括分配显存
*2.	内存映射到显存
*/
void VulkanApplication::createVertexBuffer()
{
	VkDeviceSize bufferSize = sizeof(g_vertices[0]) * g_vertices.size();

	//临时缓冲区
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	//VK_BUFFER_USAGE_TRANSFER_SRC_BIT：缓冲区可以用于源内存传输操作。	
	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	//内存映射到显存,此处可以从显存传输数据到内存data
	void* data;
	vkMapMemory(device_, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, g_vertices.data(), (size_t)bufferSize);
	vkUnmapMemory(device_, stagingBufferMemory);
	
	//最终的顶点缓冲区
	//VK_BUFFER_USAGE_TRANSFER_DST_BIT：缓冲区可以用于目标内存传输操作
	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer_, vertexBufferMemory_);

	//从一个缓冲区拷贝数据到另一个缓冲区
	copyBuffer(stagingBuffer, vertexBuffer_, bufferSize);

	//清理临时缓冲区，释放显存
	vkDestroyBuffer(device_, stagingBuffer, nullptr);
	vkFreeMemory(device_, stagingBufferMemory, nullptr);
}

/*
*1.	vkCreateBuffer,创建一个buffer句柄
*2.	vkAllocateMemory，给buffer内容分配显存
*3.	vkBindBufferMemory,绑定buffer与显存
*/
void VulkanApplication::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(device_, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to create buffer!");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device_, buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(device_, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate buffer memory!");
	}

	vkBindBufferMemory(device_, buffer, bufferMemory, 0);
}
/*
*
*/
void VulkanApplication::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	//使用命令缓冲区执行内存传输的操作命令，就像绘制命令一样:
	//需要分配一个临时命令缓冲区commandBuffer
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = commandPool_;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(device_, &allocInfo, &commandBuffer);

	//立即使用命令缓冲进行记录
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;	//只需要使用一次命令缓冲区
	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	//缓冲区内容使用vkCmdCopyBuffer命令传输，拷贝命令
	VkBufferCopy copyRegion = {};
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
	//停止记录
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	//执行命令缓冲区完成传输,提交队列
	vkQueueSubmit(graphicsQueue_, 1, &submitInfo, VK_NULL_HANDLE);
	//这里有同样有两个方式等待传输命令完成。
	//我们可以使用vkWaitForFences等待栅栏fence
	//或者只是使用vkQueueWaitIdle等待传输队列状态变为idle
	vkQueueWaitIdle(graphicsQueue_);

	//清理命令缓冲区
	vkFreeCommandBuffers(device_, commandPool_, 1, &commandBuffer);
}

uint32_t VulkanApplication::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice_, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type!");
}

void VulkanApplication::setupDebugCallback()
{
	if (!enableValidationLayers)
		return;

	VkDebugReportCallbackCreateInfoEXT createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
	createInfo.pfnCallback = debugCallback;

	if (CreateDebugReportCallbackEXT(instance_, &createInfo, nullptr, &callback_) != VK_SUCCESS) {
		throw std::runtime_error("failed to set up debug callback!");
	}
}
