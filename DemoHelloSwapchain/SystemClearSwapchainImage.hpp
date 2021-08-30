#pragma once

#include <Usagi/Entity/EntityDatabase.hpp>
#include <Usagi/Module/Common/Color/Color.hpp>
#include <Usagi/Runtime/Service.hpp>

#include <Usagi/Module/Platform/Vulkan/VulkanGpuDevice.hpp>
#include <Usagi/Module/Service/Graphics/Enum.hpp>
#include <Usagi/Module/Service/Windowing/ServiceNativeWindowManager.hpp>

namespace usagi
{
USAGI_DECL_ALIAS_SERVICE(
    ServiceHardwareGraphics,
    LazyInitService<VulkanGpuDevice>
);

struct SystemClearSwapchainImage
{
    using WriteAccess = C<>;
    using ReadAccess = C<>;

    void update(auto &&rt, auto &&db)
    {
        // The color used to fill the swapchain image
        Color4f fill_color { 245/255.f, 169/255.f, 184/255.f, 1.f };
        // todo: need a way to allow different impl while providing an unified service name
        // This service provides access to GPU resources with automatic lifetime
        // management.
        auto &gfx = USAGI_SERVICE(rt, ServiceHardwareGraphics);
        // This services synchronizes window entities with the operating system.
        auto &wnd_mgr = USAGI_SERVICE(rt, ServiceNativeWindowManager);

        // Suppose the main window is already created by some initialization
        // system and is called _main_.
        NativeWindow * const wnd = wnd_mgr.window("main");
        // Request the swapchain of the window from the graphics service.
        // If the swapchain is already created, the cached swapchain will be
        // returned. When not or the swapchain is not in a usable state,
        // a new one will be created and returned.
        // todo: ensure that the cached one has the same native window handle
        // todo: how to know when the window is destroyed so the resource can be released.
        // todo: how to specify the swapchain spec
        auto &swapchain = gfx.swapchain(wnd);
        // this will return a semaphore that can be waited on GPU. see later
        // submission commands for synchronization details.
        std::array image_sem { swapchain.acquire_next_image() };

        // The graphics service internally manages a command list allocator
        // for each thread.
        // todo: how to know when the thread is destroyed so we can release the resource (perhaps needs some common mechanism as the window)
        std::array cmd_lists { gfx.thread_allocate_graphics_command_list() };
        auto &cmd_list = cmd_lists.front();

        cmd_list.begin_recording();
        cmd_list.image_transition(
            swapchain.current_image(),
            // end of pipeline of the last usage of the swapchain image
            GpuPipelineStage::AFTER_ALL_COMMANDS,
            GpuAccessMask::NONE,
            GpuImageLayout::UNDEFINED,
            // clear color image is performed in transfer stage
            GpuPipelineStage::TRANSFER_CLEAR,
            GpuAccessMask::TRANSFER_WRITE,
            GpuImageLayout::TRANSFER_DST
        );
        cmd_list.clear_color_image(
            swapchain.current_image(),
            GpuImageLayout::TRANSFER_DST,
            fill_color
        );
        cmd_list.image_transition(
            swapchain.current_image(),
            GpuPipelineStage::TRANSFER_CLEAR,
            GpuAccessMask::TRANSFER_WRITE,
            GpuImageLayout::TRANSFER_DST,
            GpuPipelineStage::AFTER_ALL_COMMANDS,
            // Presentation automatically performs a visibility operation
            // https://github.com/KhronosGroup/Vulkan-LoaderAndValidationLayers/issues/1717
            GpuAccessMask::NONE,
            GpuImageLayout::PRESENT
        );
        cmd_list.end_recording();

        // todo semaphore pool
        std::array rendering_finished_sem {
            swapchain.semaphore_rendering_finished()
        };

        std::array wait_stages { GpuPipelineStage::COLOR_ATTACHMENT_OUTPUT };
        std::array signal_stages { GpuPipelineStage::AFTER_ALL_COMMANDS };
        // todo new submission command
        // https://github.com/KhronosGroup/Vulkan-Guide/blob/master/chapters/extensions/VK_KHR_synchronization2.md
        gfx.submit_graphics_jobs(
            cmd_lists,
            image_sem,
            wait_stages,
            rendering_finished_sem,
            signal_stages
        );
        swapchain.present(rendering_finished_sem);
    }
};
}
