#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <vector>
#include <iostream>
#include <fstream>

#include <glm/glm.hpp>
#include <array>

//�豸��չ�嵥
const std::vector<const char*> validationLayers = {
	"VK_LAYER_LUNARG_standard_validation" };	//validation layers���б�

const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME				//VK_KHR_swapchain�豸�������չ
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
	VkSurfaceCapabilitiesKHR capabilities;	//������surface��������(min/max number of images in swap chain, min/max width and height of images)
	std::vector<VkSurfaceFormatKHR> formats;	//Surface��ʽ(pixel format, color space)
	std::vector<VkPresentModeKHR> presentModes;	//��Ч��presentationģʽ
};

//����
struct Vertex {
	glm::vec2 pos;
	glm::vec3 color;

	static VkVertexInputBindingDescription getBindingDescription() 
	{
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() 
	{
		std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, pos);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		return attributeDescriptions;
	}
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
	*@brief Creating an instance, ����һ��intanceȥ��ʼ��Vulkan library
	*/
	void createInstance();

	std::vector<const char*> getRequiredExtensions();

	void setupDebugCallback();

	bool checkValidationLayerSupport();

	/*
	*@brief Selecting a physical device
	*/
	void pickPhtsicalDevice();

	/*
	*@brief ��������豸�Ƿ�������ǵĹ�������
	*/
	bool isDeviceSuitable(VkPhysicalDevice device);

	/*
	*@brief ͨ���������Ȩֵ��ȡ���ƻ���ͼ���豸�������û��һ�����õ��豸�����Իع�������ͼ���豸
	*/
	int rateDeviceSuitability(VkPhysicalDevice device);
	/*
	*@brief ����������Ҫ�Ķ��д�
	*/
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
	
	/*
	*@brief �����߼��豸
	*/
	void createLogicalDevice();

	/*
	*@brief ����Surface
	*/
	void createSurface();

	/*
	*@brief ����������
	*/
	void createSwapChain();

	bool checkDeviceExtensionSupport(VkPhysicalDevice device);

	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes);

	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

	/*
	*@brief ������ͼ��
	*/
	void createImagesViews();

	/*
	*@brief ����ͼ����ˮ��
	*/
	void createGraphPipeline();

	/*
	*@brief ������ɫ��ģ��
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
	*@brief ѭ������
	*/
	void drawFrame();

	/*
	*@brief ���´���������
	*/
	void recreateSwapChain();

	/*
	*@brief ��ս�����
	*/
	void cleanupSwapChain();

	/*
	*@brief �������㻺����
	*/
	void createVertexBuffer();

	/*
	*@brief
	*/
	void createVertexBuffer2();

	/*
	*@brief
	*/
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

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
	VkSurfaceKHR		surface_;	//������

	VkPhysicalDevice	physicalDevice_ = VK_NULL_HANDLE;	//�����豸���
	VkDevice			device_;	//�߼��豸���
	VkQueue				graphicsQueue_;	//ͼ�ζ��о��
	VkQueue				presentQueue_;	//�������о��
	
	VkSwapchainKHR swapChain_;	//����������
	std::vector<VkImage> swapChainImages_;	//VkImage�����
	VkFormat swapChainImageFormat_;		//��������ʽ
	VkExtent2D swapChainExtent_;		//��������Χ

	std::vector<VkImageView> swapChainImageViews_;//ͼ����ͼ�ľ����

	VkPipelineLayout pipelineLayout_;
	VkRenderPass	 renderPass_;
	VkPipeline		 graphicsPipeline_;

	std::vector<VkFramebuffer>	swapChainFramebuffers_;

	VkCommandPool commandPool_;
	std::vector<VkCommandBuffer> commandBuffers_;

	VkSemaphore imageAvailableSemaphore_;	//׼��������Ⱦ�ź���
	VkSemaphore renderFinishedSemaphore_;	//��Ⱦ�����ź���

	VkBuffer vertexBuffer_;
	VkDeviceMemory vertexBufferMemory_;
};

