#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <vector>
#include <iostream>
#include <fstream>

#include <glm/glm.hpp>
#include <array>

//设备扩展清单
const std::vector<const char*> c_validationLayers = {
	"VK_LAYER_LUNARG_standard_validation" };	//validation layers(校验层)的列表

const std::vector<const char*> c_deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME				//VK_KHR_swapchain设备级别的扩展
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

struct QueueFamilyIndices {
	int graphicsFamily = -1;
	int presentFamily = -1;

	bool isComplete() {
		return graphicsFamily >= 0 && presentFamily >= 0;
	}
};

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;	//基本的surface功能属性(min/max number of images in swap chain, min/max width and height of images)
	std::vector<VkSurfaceFormatKHR> formats;	//Surface格式(pixel format, color space)
	std::vector<VkPresentModeKHR> presentModes;	//有效的presentation模式
};

//顶点
struct Vertex {
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 texCoord;

	static VkVertexInputBindingDescription getBindingDescription() 
	{
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() 
	{
		std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, pos);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

		return attributeDescriptions;
	}
};

//uniform
struct UniformBufferObject
{
	glm::mat4	model;
	glm::mat4	view;
	glm::mat4	project;
};

class VulkanApplication
{
public:
	VulkanApplication();
	~VulkanApplication();

public:
	void run();

private:
	void initVulkan();

	void initWindow(int width, int height);

	void frame();

	void uninit();

	/*
	*@brief Creating an instance, 创建一个intance去初始化Vulkan library
	*/
	void createInstance();

	/*
	*@brief show vkExtensions
	*/
	void showVkExtensions();

	std::vector<const char*> getRequiredExtensions();

	void setupDebugCallback();

	bool checkValidationLayerSupport();

	/*
	*@brief Selecting a physical device
	*/
	void pickPhtsicalDevice();

	/*
	*@brief 检查物理设备是否符合我们的功能需求
	*/
	bool isDeviceSuitable(VkPhysicalDevice device);

	/*
	*@brief 通过给予更高权值获取定制化的图形设备，但如果没有一个可用的设备，可以回滚到集成图形设备
	*/
	int rateDeviceSuitability(VkPhysicalDevice device);
	/*
	*@brief 查找我们需要的队列簇
	*/
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
	
	/*
	*@brief 创建逻辑设备
	*/
	void createLogicalDevice();

	/*
	*@brief 创建Surface
	*/
	void createSurface();

	/*
	*@brief 创建交换链
	*/
	void createSwapChain();

	bool checkDeviceExtensionSupport(VkPhysicalDevice device);

	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes);

	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

	/*
	*@brief 创建视图集
	*/
	void createImagesViews();

	/*
	*@brief
	*/
	void createTextureImageView();

	/*
	*@brief 配置纹理采样器
	*/
	void createTextureSampler();

	/*
	*@brief 创建layout（uniform）
	*/
	void createDescriptorSetLayout();

	/*
	*@brief 创建图形流水线
	*/
	void createGraphPipeline();

	/*
	*@brief 创建着色器模块
	*/
	VkShaderModule createShaderModule(const std::vector<char>& code);

	/*
	*@brief 
	*/
	void createRenderPass();

	/*
	*@brief
	*/
	void createFramebuffer();

	/*
	*@
	*/
	void createCommandPool();

	/*
	*@brief
	*/
	void createCommandBuffers();

	/*
	*@brief
	*/
	void createSemaphores();

	/*
	*@brief 更新Uniform
	*/
	void updateUniformBuffer();

	/*
	*@brief 循环绘制
	*/
	void drawFrame();

	/*
	*@brief 重新创建交换链
	*/
	void recreateSwapChain();

	/*
	*@brief 清空交换链
	*/
	void cleanupSwapChain();

	/*
	*@brief 配置深度图像资源
	*/
	void createDepthResources();

	/*
	*@brief 加载图片和提交到Vulkan图像对象
	*/
	void createTextureImage();

	/*
	*@brief 图像创建函数
	*/
	void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

	/*
	*@brief image布局变换:	用于处理图像变换是使用 image memory barrier
	*/
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

	/*
	*@brief 
	*/
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

	/*
	*@brief 创建顶点缓冲区
	*/
	void createVertexBuffer();

	/*
	*@brief 创建索引缓冲区
	*/
	void createIndexBuffer();

	/*
	*@brief 创建Uniform缓冲区
	*/
	void createUniformBuffer();

	/*
	*@brief 创建描述符对象池
	*/
	void createDescriptorPool();

	/*
	*@brief	从对象池中分配描述符集合
	*/
	void createDescriptorSet();
	
	/*
	*@brief
	*/
	void createVertexBuffer2();

	/*
	*@brief 创建缓冲区
	*@param:[in] size:	缓冲区大小
	*		[in] usage
	*		[in] properties:	内存属性
	*		[out]	buffer:	buffer句柄
	*		[out]	bufferMemory：	显存句柄
	*/
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

	/*
	*@brief 从一个缓冲区拷贝数据到另一个缓冲区
	*/
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

	/*
	*@brief 开始一次commandBuffer执行
	*/
	VkCommandBuffer beginSingleTimeCommands();

	/*
	*@brief 一次commandBuffer执行完毕
	*/
	void endSingleTimeCommands(VkCommandBuffer commandBuffer);

	/*
	*@brief 创建imageview
	*/
	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

	/*
	*@
	*/
	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

	/*
	*@
	*/
	VkFormat findDepthFormat();

	bool hasStencilComponent(VkFormat format) {
		return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
	}

	static std::vector<char> readFile(const std::string& filename)
	{
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open()) {
			throw std::runtime_error("failed to open file!");
		}

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;
	}

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, int32_t code, const char* layerPrefix, const char* msg, void* userData)
	{
		std::cerr << "validation layer: " << msg << std::endl;

		return VK_FALSE;
	}

	static void onWindowResized(GLFWwindow* window, int width, int height) {
		if (width == 0 || height == 0) return;

		VulkanApplication* app = reinterpret_cast<VulkanApplication*>(glfwGetWindowUserPointer(window));
		app->recreateSwapChain();
	}
private:
	GLFWwindow* window_;
	VkInstance instance_;

	VkDebugReportCallbackEXT callback_;
	VkSurfaceKHR		surface_;	//窗体句柄

	VkPhysicalDevice	physicalDevice_ = VK_NULL_HANDLE;	//物理设备句柄
	VkDevice			device_;	//逻辑设备句柄
	VkQueue				graphicsQueue_;	//图形队列句柄
	VkQueue				presentQueue_;	//呈现队列句柄
	
	VkSwapchainKHR swapChain_;	//交换链对象
	std::vector<VkImage> swapChainImages_;	//VkImage句柄集
	VkFormat swapChainImageFormat_;		//交换链格式
	VkExtent2D swapChainExtent_;		//交换链范围

	std::vector<VkImageView> swapChainImageViews_;//图像视图的句柄集
		
	VkRenderPass	 renderPass_;

	VkDescriptorSetLayout descriptorSetLayout_;
	VkPipelineLayout pipelineLayout_;

	VkPipeline		 graphicsPipeline_;

	std::vector<VkFramebuffer>	swapChainFramebuffers_;

	VkCommandPool commandPool_;
	std::vector<VkCommandBuffer> commandBuffers_;

	VkDescriptorPool descriptorPool_;
	VkDescriptorSet descriptorSet_;

	VkSemaphore imageAvailableSemaphore_;	//准备进行渲染信号量
	VkSemaphore renderFinishedSemaphore_;	//渲染结束信号量

	VkBuffer vertexBuffer_;
	VkDeviceMemory vertexBufferMemory_;

	VkBuffer indexBuffer_;
	VkDeviceMemory indexBufferMemory_;

	VkBuffer uniformBuffer_;
	VkDeviceMemory uniformBufferMemory_;

	VkImage textureImage_;
	VkDeviceMemory textureImageMemory_;
	VkImageView textureImageView_;	//贴图图像
	VkSampler textureSampler_;	//纹理采样器

	VkImage	depthImage_;
	VkDeviceMemory depthImageMemory_;
	VkImageView depthImageView_;
};

