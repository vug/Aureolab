#include "DebugMessenger.h"

#include "Core/Log.h"

namespace vr {
    DebugMessengerBuilder::DebugMessengerBuilder(const Instance& instance)
        : instance(instance),
        vkCreateDebugUtilsMessengerEXT(initCreateFunc()),
        vkDestroyDebugUtilsMessengerEXT(initDestroyFunc()),
        debugCreateInfo({
            VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            nullptr,
            0,
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT,
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
            DebugCallback
            })
    {
    }

    PFN_vkCreateDebugUtilsMessengerEXT DebugMessengerBuilder::initCreateFunc() {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance.handle, "vkCreateDebugUtilsMessengerEXT");
        if (func == nullptr) {
            Log::Critical("Failed to create vkCreateDebugUtilsMessengerEXT function!"
                "(probably {} extension was not loaded)", VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            exit(EXIT_FAILURE);
        }
        return func;
    }

    PFN_vkDestroyDebugUtilsMessengerEXT DebugMessengerBuilder::initDestroyFunc() {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance.handle, "vkDestroyDebugUtilsMessengerEXT");
        if (func == nullptr) {
            Log::Critical("Failed to create vkCreateDebugUtilsMessengerEXT function!"
                "(probably {} extension was not loaded)", VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            exit(EXIT_FAILURE);
        }
        return func;
    }

    VKAPI_ATTR VkBool32 VKAPI_CALL DebugMessengerBuilder::DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData) {

        std::string msgType;
        switch (messageType) {
        case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
            msgType = "General";
            break;
        case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
            msgType = "Validation";
            break;
        case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
            msgType = "Performance";
            break;
        }

        switch (messageSeverity) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            Log::Trace("[ValidationLayer, {}] {}", msgType, pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            Log::Info("[ValidationLayer, {}] {}", msgType, pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            Log::Warning("[ValidationLayer, {}] {}", msgType, pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            Log::Error("[ValidationLayer, {}] {}", msgType, pCallbackData->pMessage);
            __debugbreak();
            break;
        }

        return VK_FALSE;
    }

    // --------------------------

    DebugMessenger::DebugMessenger(const DebugMessengerBuilder& builder)
        : builder(builder), instance(builder.instance) {
        Log::Debug("Creating Debug Messenger...");
        builder.vkCreateDebugUtilsMessengerEXT(instance, &builder.debugCreateInfo, nullptr, &handle);
    }

    DebugMessenger::~DebugMessenger() {
        Log::Debug("Destroying Debug Messenger...");
        builder.vkDestroyDebugUtilsMessengerEXT(instance, handle, nullptr);
    }
}