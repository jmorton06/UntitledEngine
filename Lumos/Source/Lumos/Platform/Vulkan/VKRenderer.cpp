#include "Precompiled.h"
#include "VKRenderer.h"
#include "VKDevice.h"
#include "VKShader.h"
#include "VKDescriptorSet.h"
#include "VKTools.h"
#include "VKPipeline.h"
#include "Core/Engine.h"

namespace Lumos
{
    namespace Graphics
    {
        VkImageMemoryBarrier imageBarrier(VkImage image, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT)
        {
            VkImageMemoryBarrier result = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };

            result.srcAccessMask = srcAccessMask;
            result.dstAccessMask = dstAccessMask;
            result.oldLayout = oldLayout;
            result.newLayout = newLayout;
            result.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            result.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            result.image = image;
            result.subresourceRange.aspectMask = aspectMask;
            result.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
            result.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

            return result;
        }

        void VKRenderer::InitInternal()
        {
            LUMOS_PROFILE_FUNCTION();

            m_RendererTitle = "Vulkan";
        }

        VKRenderer::~VKRenderer()
        {
        }

        void VKRenderer::PresentInternal(CommandBuffer* cmdBuffer)
        {
            LUMOS_PROFILE_FUNCTION();
        }

        void VKRenderer::ClearRenderTarget(Graphics::Texture* texture, Graphics::CommandBuffer* cmdBuffer)
        {
            VkImageSubresourceRange subresourceRange = {}; //TODO: Get from texture
            subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            subresourceRange.baseMipLevel = 0;
            subresourceRange.layerCount = 1;
            subresourceRange.levelCount = 1;

            //TODO: Pass clear Value
            //TODO: Handle Depth/Stencil

            VkClearColorValue clearColourValue = VkClearColorValue({ { 0.0f, 0.0f, 0.0f, 0.0f } });
            vkCmdClearColorImage(((VKCommandBuffer*)cmdBuffer)->GetHandle(), static_cast<VKTexture2D*>(texture)->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, &clearColourValue, 1, &subresourceRange);
        }

        void VKRenderer::ClearSwapchainImage() const
        {
            LUMOS_PROFILE_FUNCTION();

            auto m_Swapchain = VKContext::Get()->GetSwapchain();
            for(int i = 0; i < m_Swapchain->GetSwapchainBufferCount(); i++)
            {
                auto cmd = VKTools::BeginSingleTimeCommands();

                VkImageSubresourceRange subresourceRange = {};
                subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                subresourceRange.baseMipLevel = 0;
                subresourceRange.layerCount = 1;
                subresourceRange.levelCount = 1;

                VkClearColorValue clearColourValue = VkClearColorValue({ { 0.0f, 0.0f, 0.0f, 0.0f } });

                vkCmdClearColorImage(cmd, static_cast<VKTexture2D*>(m_Swapchain->GetImage(i))->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, &clearColourValue, 1, &subresourceRange);

                VKTools::EndSingleTimeCommands(cmd);
            }
        }

        void VKRenderer::OnResize(uint32_t width, uint32_t height)
        {
            LUMOS_PROFILE_FUNCTION();
            if(width == 0 || height == 0)
                return;

            m_Width = width;
            m_Height = height;

            VkSurfaceCapabilitiesKHR capabilities;
            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(VKDevice::Get().GetGPU(), VKContext::Get()->GetSwapchain()->GetSurface(), &capabilities);

            m_Width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, m_Width));
            m_Height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, m_Height));

            VKContext::Get()->OnResize(m_Width, m_Height);
        }

        Swapchain* VKRenderer::GetSwapchainInternal() const
        {
            return VKContext::Get()->GetSwapchain().get();
        }

        void VKRenderer::Begin()
        {
            LUMOS_PROFILE_FUNCTION();
            AcquireNextImage();

            GetSwapchainInternal()->GetCurrentCommandBuffer()->BeginRecording();
        }

        void VKRenderer::AcquireNextImage()
        {
            m_Context = VKContext::Get();
            auto swapchain = m_Context->GetSwapchain();

            auto result = swapchain->AcquireNextImage();
            if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
            {
                LUMOS_LOG_INFO("Acquire Image result : {0}", result == VK_ERROR_OUT_OF_DATE_KHR ? "Out of Date" : "SubOptimal");

                if(result == VK_ERROR_OUT_OF_DATE_KHR)
                    OnResize(m_Width, m_Height);
                return;
            }
            else if(result != VK_SUCCESS)
            {
                LUMOS_LOG_CRITICAL("[VULKAN] Failed to acquire swap chain image!");
            }
        }

        void VKRenderer::PresentInternal()
        {
            LUMOS_PROFILE_FUNCTION();
            VkImageMemoryBarrier presentBarrier = imageBarrier(((VKTexture2D*)GetSwapchainInternal()->GetCurrentImage())->GetImage(), VK_ACCESS_TRANSFER_WRITE_BIT, 0, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
            vkCmdPipelineBarrier(((VKCommandBuffer*)GetSwapchainInternal()->GetCurrentCommandBuffer())->GetHandle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, 0, 0, 0, 1, &presentBarrier);

            GetSwapchainInternal()->GetCurrentCommandBuffer()->EndRecording();
            VKContext::Get()->GetSwapchain()->Present();
        }

        const std::string& VKRenderer::GetTitleInternal() const
        {
            return m_RendererTitle;
        }

        void VKRenderer::BindDescriptorSetsInternal(Graphics::Pipeline* pipeline, Graphics::CommandBuffer* cmdBuffer, uint32_t dynamicOffset, std::vector<Graphics::DescriptorSet*>& descriptorSets)
        {
            LUMOS_PROFILE_FUNCTION();
            uint32_t numDynamicDescriptorSets = 0;
            uint32_t numDesciptorSets = 0;

            for(auto descriptorSet : descriptorSets)
            {
                if(descriptorSet)
                {
                    auto vkDesSet = static_cast<Graphics::VKDescriptorSet*>(descriptorSet);
                    if(vkDesSet->GetIsDynamic())
                        numDynamicDescriptorSets++;

                    m_DescriptorSetPool[numDesciptorSets] = vkDesSet->GetDescriptorSet();

                    numDesciptorSets++;
                }
            }

            vkCmdBindDescriptorSets(static_cast<Graphics::VKCommandBuffer*>(cmdBuffer)->GetHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, static_cast<Graphics::VKPipeline*>(pipeline)->GetPipelineLayout(), 0, numDesciptorSets, m_DescriptorSetPool, numDynamicDescriptorSets, &dynamicOffset);
        }

        void VKRenderer::DrawIndexedInternal(CommandBuffer* commandBuffer, DrawType type, uint32_t count, uint32_t start) const
        {
            LUMOS_PROFILE_FUNCTION();
            Engine::Get().Statistics().NumDrawCalls++;
            vkCmdDrawIndexed(static_cast<VKCommandBuffer*>(commandBuffer)->GetHandle(), count, 1, 0, 0, 0);
        }

        void VKRenderer::DrawInternal(CommandBuffer* commandBuffer, DrawType type, uint32_t count, DataType datayType, void* indices) const
        {
            LUMOS_PROFILE_FUNCTION();
            Engine::Get().Statistics().NumDrawCalls++;
            vkCmdDraw(static_cast<VKCommandBuffer*>(commandBuffer)->GetHandle(), count, 1, 0, 0);
        }

        void VKRenderer::MakeDefault()
        {
            CreateFunc = CreateFuncVulkan;
        }

        Renderer* VKRenderer::CreateFuncVulkan(uint32_t width, uint32_t height)
        {
            return new VKRenderer(width, height);
        }
    }
}
