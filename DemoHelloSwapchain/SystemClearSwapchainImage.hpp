#pragma once

#include <Usagi/Entity/EntityDatabase.hpp>
#include <Usagi/Modules/Common/Color/Color.hpp>
#include <Usagi/Runtime/Service/ServiceAccess.hpp>
#include <Usagi/Modules/IO/Graphics/Enum.hpp>
#include <Usagi/Modules/Runtime/Executive/ServiceAsyncWorker.hpp>

#include "ServiceColorChoice.hpp"

namespace usagi
{
struct SystemClearSwapchainImage
{
    using WriteAccess = C<>;
    using ReadAccess = C<>;

    using ServiceAccessT = ServiceAccess<
        ServiceHardwareGraphics,
        ServiceColorChoice,
        ServiceAsyncWorker,
        AppServiceHeapManager
    >;

    void update(ServiceAccessT rt, auto &&db);
};

void SystemClearSwapchainImage::update(ServiceAccessT rt, auto &&db)
{
    // The color used to fill the swapchain image
    static const Color4f Colors[] {
        { 245 / 255.f, 169 / 255.f, 184 / 255.f, 1.f },
        { 91 / 255.f, 206 / 255.f, 250 / 255.f, 1.f },
        { 255 / 255.f, 255 / 255.f, 255 / 255.f, 1.f },
    };

    // todo: need a way to allow different impl while providing an unified service name
    // This service provides access to GPU resources with automatic lifetime
    // management.
    auto &gfx = rt.graphics();
    // This services synchronizes window entities with the operating system.
    const auto &color = rt.color_choice();
    auto &worker = rt.executor();
    auto &heap_mgr = rt.heap_manager();

    // todo this is ugly
    using GpuDevice = std::remove_reference_t<decltype(*&gfx)>;

    // Request the swapchain of the window of the specified id (123). If the
    // window does not exist, it will be created using the default parameters
    // set in the tuple.
    // todo: how to know when the window is destroyed so the resource can be released.
    // todo: how to specify the swapchain spec
    // todo this is hell difficult to debug with when it doesn't compile
    // todo: no type info for returned resource
    // auto swapchain_accessor = heap_mgr.resource<GpuDevice::RbSwapchain>(
    ResourceAccessor<GpuDevice::RbSwapchain> swapchain_accessor = 
        heap_mgr.resource<GpuDevice::RbSwapchain>(
        { },
        &worker,
        [] {
            return std::make_tuple(
                123,
                "main",
                Vector2f(100, 100),
                Vector2f(1280, 720),
                1,
                NativeWindowState::NORMAL
            );
        }
    ).make_request(); // return type of make_request can be deduced if templated param db is removed
    if(!swapchain_accessor.ready()) return;

    // todo help auto completion infer return type
    auto swapchain = swapchain_accessor.get();
    // swapchain->
    // RefCounted<VulkanSwapchain> swapchain = swapchain_accessor.get();

    // prepare synchronization primitives
    auto sem_image_avail = gfx.allocate_semaphore();
    auto sem_render_finished = gfx.allocate_semaphore();
    std::array present_wait { sem_render_finished.get() };

    const auto image_info = swapchain->acquire_next_image(
        sem_image_avail.get());
    
    // The graphics service internally manages a command list allocator
    // for each thread.
    // todo: how to know when the thread is destroyed so we can release the resource (perhaps needs some common mechanism as the window)
    auto cmd_list = gfx.allocate_graphics_command_list();

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
        Colors[color.color_choice]
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
    // todo what if semaphores released before present is done?
    swapchain->present(image_info, present_wait);

    gfx.reclaim_resources();
}
}
