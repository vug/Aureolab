#include "Surface.h"

#include "Core/Log.h"

namespace vr {
    SurfaceBuilder::SurfaceBuilder(const Instance& instance, const VulkanWindow& win)
        : instance(instance), win(win) {}

    // --------------------------

    Surface::Surface(const SurfaceBuilder& builder)
        : builder(builder), instance(builder.instance), win(builder.win) {
        Log::Debug("Creating Surface...");
        if (win.CreateSurface(instance, &handle) != VK_SUCCESS) {
            Log::Critical("Failed to create Window Surface!");
            exit(EXIT_FAILURE);
        }
    }
    Surface::~Surface() {
        Log::Debug("Destroying Surface...");
        vkDestroySurfaceKHR(instance, handle, nullptr);
    }
}