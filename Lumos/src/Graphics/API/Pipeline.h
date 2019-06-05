#pragma once
#include "LM.h"

namespace Lumos
{
	namespace Graphics
	{
		class Shader;
		class RenderPass;
		class CommandBuffer;
		class DescriptorSet;
		struct VertexInputDescription;
		struct DescriptorLayout;
		struct DescriptorPoolInfo;

		enum class CullMode
		{
			FRONT,
			BACK,
			FRONTANDBACK,
			NONE
		};

		struct PipelineInfo
		{
			Shader* shader;
			RenderPass* vulkanRenderpass;
			VertexInputDescription* vertexLayout;

			uint32_t numVertexLayout;
			size_t strideSize;
			DescriptorPoolInfo* typeCounts;

			std::vector<DescriptorLayout> descriptorLayouts;
			uint numLayoutBindings;

			CullMode cullMode;
			String pipelineName;
			int numColorAttachments;
			bool wireframeEnabled;
			bool transparencyEnabled;
			bool depthBiasEnabled;
			uint width;
			uint height;
			uint maxObjects;

		};
		class LUMOS_EXPORT Pipeline
		{
		public:
			static Pipeline* Create(const PipelineInfo& pipelineInfo);
			virtual ~Pipeline() {};

			virtual void SetActive(CommandBuffer* cmdBuffer) = 0;

			virtual DescriptorSet* GetDescriptorSet() const = 0;
			virtual Shader* GetShader() const = 0;
		};
	}
}
