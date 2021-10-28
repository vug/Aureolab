#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
// Looks like GLFW comes with Vulkan headers. No need for: #include <vulkan/vulkan.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

class HelloTriangleApplication {
public:
    void run() {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

private:
    GLFWwindow* window;

    VkInstance instance;

    void initWindow() {
        glfwInit();

        // Do not create OpenGL context.
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        // Resizing windows will require special case. Will handle later.
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
    }

    void initVulkan() {
        createInstance();
        listExtensions();
    }

    void mainLoop() {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
        }
    }

    void cleanup() {
        vkDestroyInstance(instance, nullptr);

        glfwDestroyWindow(window);

        glfwTerminate();
    }

    void createInstance() {
        // https://khronos.org/registry/vulkan/specs/1.2-extensions/html/chap4.html#VkApplicationInfo
        VkApplicationInfo appInfo{}; // value initialization leaves un-set values as nullptr
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Hello Triangle";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_2;

        // https://khronos.org/registry/vulkan/specs/1.2-extensions/html/chap4.html#VkInstanceCreateInfo
        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        // Vulkan extensions required by GLFW
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        createInfo.enabledExtensionCount = glfwExtensionCount;
        createInfo.ppEnabledExtensionNames = glfwExtensions;
        // Will enable "global validation layers" later
        createInfo.enabledLayerCount = 0;

        std::cout << "glfw required extensions:" << std::endl;
        for (int i = 0; i < glfwExtensionCount; i++) {
            std::cout << "\t" << glfwExtensions[i] << std::endl;
        }

        // https://khronos.org/registry/vulkan/specs/1.2-extensions/html/chap4.html#vkCreateInstance
        // Creation Parameters, Allocator Callbacks, Created/Returned Object
        if (vkCreateInstance(&createInfo, nullptr, &instance)) { // returns VkResult 
            throw std::runtime_error("failed to create Vulkan instance!");
        }
    }

    void listExtensions() {
        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> extensions(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
        
        std::cout << "available Vulkan extensions:\n";
        for (const auto& extension : extensions) {
            std::cout << '\t' << extension.extensionName << '\n';
        }
    }
};

int main() {
    HelloTriangleApplication app;

    try {
        app.run();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}