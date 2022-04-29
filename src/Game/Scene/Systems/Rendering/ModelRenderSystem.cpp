#include "ModelRenderSystem.h"

#include "Constants.h"
#include "DirectionComponent.h"
#include "ModelComponent.h"
#include "PositionComponent.h"
#include "Transform2D.h"
#include "AtlasRender/Renderer.h"
#include "AtlasRender/AssetTypes/ModelAsset.h"


void cpp_conv::ModelRenderSystem::Initialise(atlas::scene::EcsManager&, const uint8_t renderMask)
{
    m_RenderMask = renderMask;
}

void cpp_conv::ModelRenderSystem::Update(atlas::scene::EcsManager& ecs)
{
    using namespace components;
    for(auto [entity, model, position] : ecs.IterateEntityComponents<ModelComponent, PositionComponent>())
    {
        if ((model.m_RenderMask & m_RenderMask) == 0 || !model.m_Model || !model.m_Model->GetMesh() || !model.m_Model->GetProgram())
        {
            continue;
        }

        assert(0 != (BGFX_CAPS_INSTANCING & bgfx::getCaps()->supported));

        atlas::maths_helpers::Angle rotation{};
        if (ecs.DoesEntityHaveComponent<DirectionComponent>(entity))
        {
            const auto& direction = ecs.GetComponent<DirectionComponent>(entity);
            rotation = rotationRadiansFromDirection(direction.m_Direction);
        }

        constexpr uint16_t instanceStride = sizeof(Eigen::Matrix4f);

        constexpr uint32_t totalPositions = 1;
        const uint32_t numDrawableInstances = bgfx::getAvailInstanceDataBuffer(totalPositions, instanceStride);

        bgfx::InstanceDataBuffer idb{};
        allocInstanceDataBuffer(&idb, numDrawableInstances, instanceStride);

        // Only recompute these on-dirty, once the dirtying system is added
        Eigen::Affine3f t{Eigen::Translation3f(position.m_Position.cast<float>())};
        Eigen::Affine3f r{Eigen::AngleAxisf(rotation.AsRadians(), Eigen::Vector3f(0, 1, 0))};
        Eigen::Matrix4f m = (t * r).matrix();
        bgfx::setTransform(m.data());

        atlas::render::drawInstanced(
                cpp_conv::constants::render_views::c_geometry,
                model.m_Model,
                model.m_Model->GetProgram(),
                { m },
                ~BGFX_DISCARD_STATE);
    }
}
