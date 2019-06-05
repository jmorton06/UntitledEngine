#include "LM.h"
#include "GLRenderPass.h"
#include "Graphics/API/Renderer.h"
#include "GLFramebuffer.h"
#include "GLDebug.h"

namespace Lumos
{
    namespace Graphics
    {
        GLRenderPass::GLRenderPass()
        {

        }

        GLRenderPass::~GLRenderPass()
        {

        }

        bool GLRenderPass::Init(const RenderpassInfo &renderpassCI)
        {
			m_Clear = renderpassCI.clear;
            return false;
        }

        void GLRenderPass::Unload() const
        {

        }

        void GLRenderPass::BeginRenderpass(CommandBuffer *commandBuffer, const Maths::Vector4 &clearColour,
                                           Framebuffer *frame, SubPassContents contents, uint32_t width,
                                           uint32_t height) const
        {
            if(frame != nullptr)
            {
                frame->Bind(width, height);
			    frame->SetClearColour(clearColour);
                GLCall(glClearColor(clearColour.GetX(),clearColour.GetY(),clearColour.GetZ(),clearColour.GetW()));
            }
            else
            {
                GLCall(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0));
                GLCall(glClearColor(clearColour.GetX(),clearColour.GetY(),clearColour.GetZ(),clearColour.GetW()));
                GLCall(glViewport(0, 0, width, height));
            }

			if(m_Clear)
				Renderer::Clear(RENDERER_BUFFER_COLOUR | RENDERER_BUFFER_DEPTH | RENDERER_BUFFER_STENCIL);
        }

        void GLRenderPass::EndRenderpass(CommandBuffer *commandBuffer)
        {
#ifdef LUMOS_DEBUG
			GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
#endif
        }
    }
}

