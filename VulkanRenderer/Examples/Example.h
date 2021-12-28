#pragma once

#include "VulkanContext.h"
#include "VulkanRenderer.h"
#include "VulkanDestroyer.h"

class Example {
public:
    Example(VulkanContext& vc, VulkanRenderer& vr) : vc(vc), vr(vr), destroyer(vc.GetDevice(), vc.GetAllocator()) {}
    virtual void OnRender() = 0;
    virtual ~Example() {
        destroyer.DestroyAll();
    }
protected:
    VulkanContext& vc;
    VulkanRenderer& vr;
    VulkanDestroyer destroyer;
};