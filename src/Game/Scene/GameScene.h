#pragma once

#include <AtlasScene/Scene.h>

#include "CameraRenderSystem.h"
#include "ClippedSurfaceRenderSystem.h"
#include "ConveyorRenderingSystem.h"
#include "EntityLookupGrid.h"
#include "FactoryDefinition.h"
#include "LightingRenderSystem.h"
#include "Map.h"
#include "ModelRenderSystem.h"
#include "PostProcessSystem.h"
#include "ShadowMappingSystem.h"
#include "UIControllerSystem.h"
#include "AtlasRender/Renderer.h"
#include "AtlasRender/Types/FrameBuffer.h"

namespace cpp_conv
{
    class GameScene : public atlas::scene::EcsScene
    {
    public:
        explicit GameScene(atlas::resource::AssetPtr<resources::Map> map)
            : m_InitialisationData{std::move(map)}
        {
        }

        void OnEntered(atlas::scene::SceneManager& sceneManager) override;

        void ConstructSystems(atlas::scene::SystemsBuilder& builder) override;

        void OnUpdate(atlas::scene::SceneManager& sceneManager) override
        {
            EcsScene::OnUpdate(sceneManager);
        }

        void OnRender(atlas::scene::SceneManager& sceneManager) override
        {
            EcsScene::OnRender(sceneManager);
        }

        void OnExited(atlas::scene::SceneManager& sceneManager) override
        {
            EcsScene::OnExited(sceneManager);
        }

    private:
        void ConstructFrameGraph();

        template<typename TRenderPass, typename ... TInitArgs>
        void AddToFrameGraph(std::string_view viewName, TRenderPass* renderPass, TInitArgs... args)
        {
            atlas::render::addToFrameGraph(
                viewName,
                [&, args..., renderPass]()
                {
                    renderPass->Initialise(GetEcsManager(), args...);
                },
                [&, renderPass]()
                {
                    renderPass->Update(GetEcsManager());
                });
        }

        struct InitialisationData
        {
            atlas::resource::AssetPtr<resources::Map> m_Map;
        } m_InitialisationData;

        struct SceneData
        {
            EntityLookupGrid m_LookupGrid;
        } m_SceneData;

        struct RenderSystems
        {
            struct ShadowPass
            {
                ShadowMappingSystem m_ShadowMapping;
                ModelRenderSystem m_ModelRenderer;
                ConveyorRenderingSystem m_ConveyorRenderer;

                void Initialise(atlas::scene::EcsManager& ecsManager);
                void Update(atlas::scene::EcsManager& ecsManager);
            } m_ShadowPass;

            struct GeometryPass
            {
                CameraRenderSystem m_CameraRenderSystem;
                LightingRenderSystem m_LightingSystem;
                ClippedSurfaceRenderSystem m_ClippedSurfaceRenderSystem;
                ModelRenderSystem m_ModelRenderer;
                ConveyorRenderingSystem m_ConveyorRenderer;

                void Initialise(atlas::scene::EcsManager& ecsManager);
                void Update(atlas::scene::EcsManager& ecsManager);
            } m_GeometryPass;

            struct PostGeometry
            {
                UIControllerSystem m_UIController;
                PostProcessSystem m_PostProcess;

                void Initialise(atlas::scene::EcsManager& ecsManager, const atlas::render::FrameBuffer* gbuffer);
                void Update(atlas::scene::EcsManager& ecsManager);
            } m_PostGeometry;

            atlas::render::FrameBuffer m_GBuffer;
        } m_RenderSystems;
    };
}
