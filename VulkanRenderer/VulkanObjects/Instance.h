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

	private:
		std::vector<const char*> initLayers(const Params& params);
		std::vector<const char*> initExtensions(const Params& params);
		VkApplicationInfo initAppInfo(const Params& params);
		VkInstanceCreateInfo initInfo(const Params& params);

	public:
		Params params = {};
		std::vector<const char*> layers;
		std::vector<const char*> extensions;
		VkApplicationInfo appInfo = {};
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
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