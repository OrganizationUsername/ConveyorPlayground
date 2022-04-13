#include "ConveyorItemRenderingSystem.h"
#include <array>
#include <iostream>

#include "ConveyorComponent.h"
#include "ConveyorHelper.h"
#include "ItemDefinition.h"
#include "ItemRegistry.h"
#include "RenderContext.h"
#include "SDL_mouse.h"
#include "SequenceComponent.h"
#include "TileRenderHandler.h"
#include "AtlasRender/Renderer.h"
#include "AtlasRender/AssetTypes/ModelAsset.h"
#include "AtlasRender/AssetTypes/ShaderAsset.h"
#include "AtlasResource/AssetPtr.h"
#include "AtlasResource/ResourceLoader.h"
#include "bgfx/bgfx.h"
#include "bx/bx.h"
#include "bx/hash.h"
#include "bx/math.h"

/*
*
            tileRenderer(
                *g_renderContext,
                sprite.m_pTile.get(),
                {static_cast<float>(position.m_Position.x()) * 4, static_cast<float>(position.m_Position.y()) * 4},
                {0xFFFFFFFF},
                true);
 */

namespace bgfx
{
    extern const char* getAttribName(Attrib::Enum _attr);
}

namespace
{
    void dump(const bgfx::VertexLayout& layout)
    {
        std::cerr << std::format("\nVertexLayout {} ({}), stride {}"
            , layout.m_hash
            , bx::hash<bx::HashMurmur2A>(layout.m_attributes)
            , layout.m_stride
            );

        for (uint32_t attr = 0; attr < bgfx::Attrib::Count; ++attr)
        {
            if (UINT16_MAX != layout.m_attributes[attr])
            {
                uint8_t num;
                bgfx::AttribType::Enum type;
                bool normalized;
                bool asInt;
                layout.decode(static_cast<bgfx::Attrib::Enum>(attr), num, type, normalized, asInt);

                std::cerr << std::format("\n\tattr {}: {} num {}, type {}, norm [{}], asint [{}], offset {}"
                    , attr
                    , bgfx::getAttribName(static_cast<bgfx::Attrib::Enum>(attr) )
                    , num
                    , static_cast<int>(type)
                    , normalized ? 'x' : ' '
                    , asInt      ? 'x' : ' '
                    , layout.m_offset[attr]
                    );
            }
        }
    }
}

struct LerpInformation
{
    Eigen::Vector2f m_PreviousPosition;
    float m_LerpFactor;
};

void drawItem(
    const cpp_conv::resources::TileAsset* pTile,
    Eigen::Vector2f renderPosition,
    const std::optional<LerpInformation> lerp = {})
{
    if (lerp.has_value())
    {
        renderPosition = lerp->m_PreviousPosition + ((renderPosition - lerp->m_PreviousPosition) * lerp->m_LerpFactor);
    }

    /*tileRenderer(
        *g_renderContext,
        pTile,
        { renderPosition.x(), renderPosition.y() },
        {0xFFFFFFFF},
        true);*/
}

void ConveyorItemRenderingSystem::Update(atlas::scene::EcsManager& ecs)
{
    for(const auto entity : ecs.GetEntitiesWithComponents<cpp_conv::components::SequenceComponent>())
    {
        const auto& sequence = ecs.GetComponent<cpp_conv::components::SequenceComponent>(entity);

        const float fLerpFactor = sequence.m_CurrentTick / static_cast<float>(sequence.m_MoveTick);

        for(int channel = 0; channel < cpp_conv::components::c_conveyorChannels; channel++)
        {
            auto lanes = sequence.m_RealizedStates[channel].m_Lanes;
            if (sequence.m_RealizedStates[channel].m_Lanes == 0)
            {
                continue;
            }

            while (lanes != 0)
            {
                const auto nextItemBit = std::countr_zero(lanes);
                const auto nextSlot = (sequence.m_Length * 2) - nextItemBit - 1;
                lanes &= ~(1ULL << nextItemBit);

                const auto sequenceIndex = nextSlot / 2;
                const auto sequenceSlot = nextSlot % 2;

                auto itemSlot = cpp_conv::conveyor_helper::getItemInSlot(
                    sequence,
                    sequenceIndex,
                    channel,
                    sequenceSlot);

                if (!itemSlot.has_value())
                {
                    continue;
                }

                // TODO: This should be cached better, the constant item definition lookups get expensive
                const atlas::resource::AssetPtr<cpp_conv::ItemDefinition> itemAsset = cpp_conv::resources::getItemDefinition(itemSlot->m_Item);
                if (!itemAsset || !itemAsset->GetAssetId().IsValid())
                {
                    continue;
                }

                const auto tileAsset = atlas::resource::ResourceLoader::LoadAsset<cpp_conv::resources::TileAsset>(itemAsset->GetAssetId());
                if (!tileAsset)
                {
                    continue;
                }

                if (itemSlot->m_bIsAnimated)
                {
                    /*drawItem(
                        tileAsset,
                        cpp_conv::conveyor_helper::getSlotPosition(sequence, sequenceIndex, channel, sequenceSlot),
                        {{
                            itemSlot->m_PreviousVisualLocation,
                            fLerpFactor
                        }});*/
                }
                else
                {
                    //drawItem(tileAsset,cpp_conv::conveyor_helper::getSlotPosition(sequence, sequenceIndex, channel, sequenceSlot));
                }
            }
        }
    }

    for(const auto entity : ecs.GetEntitiesWithComponents<cpp_conv::components::ConveyorComponent, cpp_conv::components::IndividuallyProcessableConveyorComponent>())
    {
        const auto& conveyor = ecs.GetComponent<cpp_conv::components::ConveyorComponent>(entity);
        const float fLerpFactor = conveyor.m_CurrentTick / static_cast<float>(conveyor.m_MoveTick);

        for (int channel = 0; channel < cpp_conv::components::c_conveyorChannels; channel++)
        {
            int iChannelLength = cpp_conv::components::c_conveyorChannelSlots;
            if (conveyor.m_bIsCorner)
            {
                iChannelLength += conveyor.m_InnerMostChannel == channel ? -1 : 1;
            }

            for (int slot = 0; slot < iChannelLength; slot++)
            {
                auto itemSlot = cpp_conv::conveyor_helper::getItemInSlot(
                    conveyor,
                    channel,
                    slot);

                if (!itemSlot.has_value())
                {
                    continue;
                }

                // TODO: This should be cached better, the constant item definition lookups get expensive
                const atlas::resource::AssetPtr<cpp_conv::ItemDefinition> itemAsset = cpp_conv::resources::getItemDefinition(itemSlot->m_Item);
                if (!itemAsset || !itemAsset->GetAssetId().IsValid())
                {
                    continue;
                }

                const auto tileAsset = atlas::resource::ResourceLoader::LoadAsset<cpp_conv::resources::TileAsset>(itemAsset->GetAssetId());
                if (!tileAsset)
                {
                    continue;
                }

                const auto& rTargetChannel = conveyor.m_Channels[channel];
                drawItem(
                   tileAsset.get(),
                   rTargetChannel.m_pSlots[slot].m_VisualPosition,
                   {{
                       itemSlot->m_PreviousVisualLocation,
                       fLerpFactor
                   }});
            }
        }
    }

    atlas::render::addToFrameGraph_oneOff("ConveyorRenderer", []()
    {
        const auto modelPtr = atlas::resource::ResourceLoader::LoadAsset<cpp_conv::resources::registry::CoreBundle, atlas::render::ModelAsset>(
            cpp_conv::resources::registry::core_bundle::assets::conveyors::models::c_ConveyorStraight);
        if (!modelPtr || !modelPtr->GetMesh() || !modelPtr->GetProgram())
        {
            return;
        }

        static int prev_mouse_x = 0;
        static int prev_mouse_y = 0;
        static float cam_pitch = 0.0f;
        static float cam_yaw = 0.0f;
        static float rot_scale = 0.01f;
        static const int width = 800;
        static const int height = 600;
        static bool layoutDumped = false;

        if (!layoutDumped)
        {
            dump(modelPtr->GetMesh()->GetLayout());
            layoutDumped = true;
        }

        int mouse_x, mouse_y;
        const int buttons = SDL_GetMouseState(&mouse_x, &mouse_y);
        if ((buttons & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0) {
            int deltaX = mouse_x - prev_mouse_x;
            int deltaY = mouse_y - prev_mouse_y;
            cam_yaw += static_cast<float>(-deltaX) * rot_scale;
            cam_pitch += static_cast<float>(-deltaY) * rot_scale;
        }

        prev_mouse_x = mouse_x;
        prev_mouse_y = mouse_y;

        float cam_rotation[16];
        bx::mtxRotateXYZ(cam_rotation, cam_pitch, cam_yaw, 0.0f);

        float cam_translation[16];
        bx::mtxTranslate(cam_translation, 0.0f, 0.0f, -5.0f);

        float cam_transform[16];
        bx::mtxMul(cam_transform, cam_translation, cam_rotation);

        float view[16];
        bx::mtxInverse(view, cam_transform);

        float proj[16];
        bx::mtxProj(
           proj, 60.0f, static_cast<float>(width) / static_cast<float>(height), 0.1f,
           100.0f, bgfx::getCaps()->homogeneousDepth);

        bgfx::setViewTransform(0, view, proj);

        float model[16];
        bx::mtxIdentity(model);
        bgfx::setTransform(model);

        const auto& mesh = modelPtr->GetMesh();
        const auto& program = modelPtr->GetProgram();

        int textureIndex = 0;
        for(const auto& texture : modelPtr->GetTextures())
        {
            bgfx::setTexture(textureIndex++, texture.m_Sampler, texture.m_Texture->GetHandle());
        }

        for(const auto& segment : mesh->GetSegments())
        {
            bgfx::setVertexBuffer(0, segment.m_VertexBuffer);
            bgfx::setIndexBuffer(segment.m_IndexBuffer);
            bgfx::submit(0, program->GetHandle());
        }
    });
}
