#pragma once
#include "LM.h"
#include "LumosComponent.h"
#include "Maths/Transform.h"

namespace Lumos
{
	class LUMOS_EXPORT TransformComponent : public LumosComponent
	{
	public:
		explicit TransformComponent();
		explicit TransformComponent(const Maths::Matrix4& matrix);

        void SetWorldMatrix(const Maths::Matrix4& matrix);
		void OnUpdateComponent(float dt) override;
        void OnIMGUI() override;
		
		Maths::Transform& GetTransform() { return m_Transform; }

    private:
		Maths::Transform m_Transform;
	};
}