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

        // prepare synchronization primitives
        auto sem_image_avail = gfx.allocate_semaphore();
        auto sem_render_finished = gfx.allocate_semaphore();
        std::array present_wait { sem_render_finished.get() };

        const auto image_info = swapchain.acquire_next_image(
            sem_image_avail.get());

        // The graphics service internally manages a command list allocator
        // for each thread.
        // todo: how to know when the thread is destroyed so we can release the resource (perhaps needs some common mechanism as the window)
        auto cmd_list = gfx.allocate_graphics_command_list(0);

        // https://github.com/KhronosGroup/Vulkan-Guide/blob/master/chapters/extensions/VK_KHR_synchronization2.md#top_of_pipe-and-bottom_of_pipe-deprecation
        // https://github.com/philiptaylor/vulkan-sxs/blob/master/04-clear/README.md
        cmd_list.begin_recording();
        cmd_list.image_transition(
            image_info.image,
            // end of pipeline of the last usage of the swapchain image
            // GpuPipelineStage::ALL_COMMANDS,
            GpuPipelineStage::TOP_OF_PIPE,
            GpuAccessMask::NONE,
            GpuImageLayout::UNDEFINED,
            // clear color image is performed in transfer stage
            GpuPipelineStage::TRANSFER_CLEAR,
            GpuAccessMask::TRANSFER_WRITE,
            GpuImageLayout::TRANSFER_DST
        );
        cmd_list.clear_color_image(
            image_info.image,
            GpuImageLayout::TRANSFER_DST,
            fill_color
        );
        cmd_list.image_transition(
            image_info.image,
            GpuPipelineStage::TRANSFER_CLEAR,
            GpuAccessMask::TRANSFER_WRITE,
            GpuImageLayout::TRANSFER_DST,
            // GpuPipelineStage::NONE,
            GpuPipelineStage::BOTTOM_OF_PIPE,
            // Presentation automatically performs a visibility operation
            // https://github.com/KhronosGroup/Vulkan-LoaderAndValidationLayers/issues/1717
            GpuAccessMask::NONE,
            GpuImageLayout::PRESENT
        );
        cmd_list.end_recording();

        auto cmd_submission_list = gfx.create_command_buffer_list();
        cmd_submission_list.add(std::move(cmd_list));

        auto wait_sem = gfx.create_semaphore_info();
        wait_sem.add(
            std::move(sem_image_avail),
            GpuPipelineStage::TRANSFER_CLEAR
        );
        auto signal_sem = gfx.create_semaphore_info();
        signal_sem.add(
            std::move(sem_render_finished),
            GpuPipelineStage::BOTTOM_OF_PIPE
        );

        // https://github.com/KhronosGroup/Vulkan-Guide/blob/master/chapters/extensions/VK_KHR_synchronization2.md
        gfx.submit_graphics_jobs(
            std::move(cmd_submission_list),
            std::move(wait_sem),
            // render finished sem signaled here
            std::move(signal_sem)
        );

        // wait on render finished sem
        swapchain.present(image_info, present_wait);

        gfx.reclaim_resources();
    }
};
}
