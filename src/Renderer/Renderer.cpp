#include "Renderer.h"
#include "RenderContext.h"
#include "SceneContext.h"
#include "SwapChain.h"
#include "Entity.h"

#include <map>
#include <mutex>
#include "TileRenderHandler.h"
#include "ResourceRegistry.h"
#include "ResourceManager.h"
#include "TileAsset.h"
#include "Profiler.h"

using TypeId = size_t;
static std::map<TypeId, std::function<void(cpp_conv::RenderContext&, const cpp_conv::resources::RenderableAsset*, cpp_conv::Transform2D, cpp_conv::Colour)>*> g_typeHandlers;

namespace
{
    std::mutex& getStateMutex()
    {
        static std::mutex s_stateMutex;
        return s_stateMutex;
    }

    std::function<void(cpp_conv::RenderContext&, const cpp_conv::resources::RenderableAsset*, cpp_conv::Transform2D, cpp_conv::Colour)>* getTypeHandler(const std::type_info& type)
    {
        // No need to lock here, this is only called in the context of an existing lock
        auto iter = g_typeHandlers.find(type.hash_code());
        if (iter == g_typeHandlers.end())
        {
            return nullptr;
        }

        return iter->second;
    }
}

void cpp_conv::renderer::init(cpp_conv::RenderContext& kContext, cpp_conv::renderer::SwapChain& rSwapChain)
{    
    ScreenBufferInitArgs kArgs = { };
    rSwapChain.Initialize(kContext, kArgs);
    kContext.m_cameraQuad = { 0, 0, rSwapChain.GetWriteSurface().GetWidth(), rSwapChain.GetWriteSurface().GetHeight() };
    kContext.m_surface = &rSwapChain.GetWriteSurface();
}

void drawPlayer(const cpp_conv::SceneContext& kSceneContext, cpp_conv::RenderContext& kRenderContext)
{
    auto pTile = cpp_conv::resources::resource_manager::loadAsset<cpp_conv::resources::TileAsset>(cpp_conv::resources::registry::visual::Player);
    if (!pTile)
    {
        return;
    }

    cpp_conv::renderer::renderAsset(
        kRenderContext,
        pTile.get(),
        {
            (float)kSceneContext.m_player.GetX() * cpp_conv::renderer::c_gridScale,
            (float)kSceneContext.m_player.GetY() * cpp_conv::renderer::c_gridScale,
            Rotation::DegZero
        },
        { 0xFFFFFFFF });
}

void cpp_conv::renderer::render(const SceneContext& kSceneContext, RenderContext& kContext)
{
    drawBackground(kSceneContext, kContext);

    uint32_t uiPassCount = 0;
    for (auto pEntity : kSceneContext.m_rMap.GetConveyors())
    {
        uiPassCount = std::max(uiPassCount, pEntity->GetDrawPassCount());
    }
    for (auto pEntity : kSceneContext.m_rMap.GetOtherEntities())
    {
        uiPassCount = std::max(uiPassCount, pEntity->GetDrawPassCount());
    }

    kContext.m_iCurrentLayer = kSceneContext.m_player.GetZ();
    for (uint32_t uiPass = 0; uiPass < uiPassCount; uiPass++)
    {
        kContext.m_uiCurrentDrawPass = uiPass;
        for (auto pEntity : kSceneContext.m_rMap.GetConveyors())
        {
            if (pEntity->GetDrawPassCount() < (uiPass + 1) ||
                (pEntity->m_position.GetZ() + pEntity->m_size.GetZ() - 1) < kSceneContext.m_player.GetZ()
                || pEntity->m_position.GetZ() > kSceneContext.m_player.GetZ())
            {
                continue;
            }

            pEntity->Draw(kContext);
        }

        for (auto pEntity : kSceneContext.m_rMap.GetOtherEntities())
        {
            if (pEntity->GetDrawPassCount() < (uiPass + 1) ||
                (pEntity->m_position.GetZ() + pEntity->m_size.GetZ() - 1) < kSceneContext.m_player.GetZ()
                || pEntity->m_position.GetZ() > kSceneContext.m_player.GetZ())
            {
                continue;
            }

            pEntity->Draw(kContext);
        }
    }

    drawPlayer(kSceneContext, kContext);
}

void cpp_conv::renderer::renderAsset(const std::type_info& type, RenderContext& kContext, resources::RenderableAsset* pRenderable, Transform2D transform, Colour kColourOverride)
{
    PROFILE_FUNC();
    std::function<void(cpp_conv::RenderContext&, const resources::RenderableAsset*, cpp_conv::Transform2D, cpp_conv::Colour)>* pHandler = nullptr;
    {
        std::lock_guard<std::mutex> lock(getStateMutex());
        pHandler = getTypeHandler(type);
    }

    if (!pHandler)
    {
        return;
    }

    (*pHandler)(kContext, pRenderable, std::move(transform), kColourOverride);
}

void cpp_conv::renderer::registerTypeHandler(const std::type_info& type, std::function<void(cpp_conv::RenderContext&, const resources::RenderableAsset*, cpp_conv::Transform2D, cpp_conv::Colour)> fHandler)
{
    std::lock_guard<std::mutex> lock(getStateMutex());
    g_typeHandlers[type.hash_code()] = new std::function<void(cpp_conv::RenderContext&, const resources::RenderableAsset*, cpp_conv::Transform2D, cpp_conv::Colour)>(fHandler);
}

void cpp_conv::renderer::drawBackground(const SceneContext& kSceneContext, RenderContext& kContext)
{
    auto pTile = cpp_conv::resources::resource_manager::loadAsset<cpp_conv::resources::TileAsset>(
        cpp_conv::resources::registry::visual::BackgroundRepeating);
    if (!pTile)
    {
        return;
    }

    cpp_conv::renderer::renderAsset(
        kContext,
        pTile.get(),
        {
            (float)kSceneContext.m_player.GetX() * cpp_conv::renderer::c_gridScale,
            (float)kSceneContext.m_player.GetY() * cpp_conv::renderer::c_gridScale,
            Rotation::DegZero,
            true
        },
        { 0xFFFFFFFF });
}
