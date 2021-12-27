#pragma once

#include <vulkan/vulkan.h>

#include <vector>

namespace vr {
	class InstanceBuilder {
	public:
		// Defaults for a windowed Windows app with validation layers
		struct Params {
			bool headless = false;
			bool validation = true;
			std::vector<const char*> requestedLayers = {};
			std::vector<const char*> requestedExtensions = {};
		};

		InstanceBuilder(const Params& params = Params());
		operator const VkInstanceCreateInfo* ();

		void foo() {
			VkDebugUtilsMessageSeverityFlagsEXT severity =
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
				| VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		}

	private:
		std::vector<const char*> initLayers(const Params& params);
		std::vector<const char*> initExtensions(const Params& params);

	public:
		std::vector<const char*> layers;
		std::vector<const char*> extensions;
		VkApplicationInfo appInfo = {};
		VkInstanceCreateInfo info = {};
	};

	struct Instance {
		Instance(const InstanceBuilder& builder);
		~Instance();
		InstanceBuilder builder = {};
		VkInstance handle = VK_NULL_HANDLE;

		operator VkInstance () const { return handle; }
		operator VkInstance* () { return &handle; }
		operator VkInstance& () { return handle; }
	};
}