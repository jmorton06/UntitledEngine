#pragma once
#include "LM.h"
#include "Graphics/API/Pipeline.h"

namespace Lumos
{
    namespace Graphics
    {
        class GLRenderPass;
        class CommandBuffer;

        class GLPipeline : public Pipeline
        {
        public:
            GLPipeline();
            GLPipeline(const PipelineInfo& pipelineCI);
            ~GLPipeline();

            bool Init(const PipelineInfo& pipelineCI);

            void SetActive(Graphics::CommandBuffer* cmdBuffer) override;
			
			DescriptorSet* GetDescriptorSet() const override { return m_DescriptorSet; }
			Shader* GetShader() const override { return m_Shader; }

        private:
			DescriptorSet* m_DescriptorSet = nullptr;
			Shader* m_Shader = nullptr;
            GLRenderPass* m_RenderPass;
            std::string pipelineName;
        };
    }
}

