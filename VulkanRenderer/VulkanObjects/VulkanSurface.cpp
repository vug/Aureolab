#include "VulkanSurface.h"

#include "Core/Log.h"

namespace vr {
    SurfaceBuilder::SurfaceBuilder(const Instance& instance, const VulkanWindow& win)
        : instance(instance), win(win) {}

    // --------------------------

    Surface::Surface(const SurfaceBuilder& builder)
        : builder(builder) {
        Log::Debug("Creating Surface...");
        if (builder.win.CreateSurface(builder.instance, &handle) != VK_SUCCESS) {
            Log::Critical("Failed to create Window Surface!");
            exit(EXIT_FAILURE);
        }
    }
    Surface::~Surface() {
        Log::Debug("Destroying Surface...");
        vkDestroySurfaceKHR(builder.instance, handle, nullptr);
    }
}