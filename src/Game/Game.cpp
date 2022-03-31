#include "Game.h"
#include "FrameLimiter.h"
#include "RenderContext.h"
#include "Renderer.h"
#include "SelfRegistration.h"
#include "SwapChain.h"

#include <chrono>
#include <tuple>
#include "FactoryComponent.h"
#include "GameMapLoadInterstitialScene.h"
#include "SequenceComponent.h"
#include "SpriteLayerComponent.h"
#include "AtlasAppHost/Application.h"
#include "AtlasAppHost/Main.h"
#include "AtlasScene/SceneManager.h"

#undef max
#undef min

using namespace cpp_conv::resources;

cpp_conv::RenderContext* g_renderContext;

int gameMain(int argc, char* argv[])
{
    logStartUp();
    srand(static_cast<unsigned>(time(nullptr)));
    auto [iWidth, iHeight] = atlas::app_host::Application::Get().GetAppDimensions();

    registration::processSelfRegistrations();

    atlas::scene::ComponentRegistry::RegisterComponent<cpp_conv::components::NameComponent>();
    atlas::scene::ComponentRegistry::RegisterComponent<cpp_conv::components::DescriptionComponent>();
    atlas::scene::ComponentRegistry::RegisterComponent<cpp_conv::components::ConveyorComponent>();
    atlas::scene::ComponentRegistry::RegisterComponent<cpp_conv::components::IndividuallyProcessableConveyorComponent>();
    atlas::scene::ComponentRegistry::RegisterComponent<cpp_conv::components::DirectionComponent>();
    atlas::scene::ComponentRegistry::RegisterComponent<cpp_conv::components::FactoryComponent>();
    atlas::scene::ComponentRegistry::RegisterComponent<cpp_conv::components::PositionComponent>();
    atlas::scene::ComponentRegistry::RegisterComponent<cpp_conv::components::SequenceComponent>();
    atlas::scene::ComponentRegistry::RegisterComponent<cpp_conv::components::SpriteLayerComponent<1>>();
    atlas::scene::ComponentRegistry::RegisterComponent<cpp_conv::components::SpriteLayerComponent<2>>();
    atlas::scene::ComponentRegistry::RegisterComponent<cpp_conv::components::SpriteLayerComponent<3>>();
    atlas::scene::ComponentRegistry::RegisterComponent<cpp_conv::components::WorldEntityInformationComponent>();

    atlas::scene::SceneManager sceneManager;
    sceneManager.TransitionTo<cpp_conv::GameMapLoadInterstitialScene>(registry::maps::c_bigmap);

    cpp_conv::FrameLimiter frameLimiter(120);
    frameLimiter.Start();

    // TODO REMOVE THIS
    cpp_conv::RenderContext kRenderContext =
    {
        0,
        {0xFFFFFFFF},
        0,
        nullptr,
        0.8f
    };
    g_renderContext = &kRenderContext;

    cpp_conv::renderer::SwapChain swapChain(kRenderContext, iWidth, iHeight);
    init(kRenderContext, swapChain);

    while (true)
    {
        kRenderContext.m_uiDrawnItems = 0;

        sceneManager.Update();
        frameLimiter.Limit();

        swapChain.SwapAndPresent();
        frameLimiter.EndFrame();
    }

    /*cpp_conv::WorldMap worldMap;
    {
        const AssetPtr<Map> map = resource_manager::loadAssetUncached<Map>(registry::maps::c_simple);
        worldMap.Consume(map);
        worldMap.PopulateCorners();
    }

    std::vector<cpp_conv::Sequence*> sequences = initializeSequences(worldMap, worldMap.GetConveyors());
    cpp_conv::SceneContext kSceneContext =
    {
        worldMap,
        sequences,
        { std::chrono::high_resolution_clock::now() },
        {
            0,
            Direction::Right,
            true
        }
    };



    cpp_conv::renderer::SwapChain swapChain(kRenderContext, iWidth, iHeight);
    init(kRenderContext, swapChain);

    cpp_conv::FrameLimiter frameLimiter(60);
    std::queue<cpp_conv::commands::CommandType> commands;

    cpp_conv::ui::initializeGuiSystem();


    frameLimiter.Start();
    float fCurrentZoom = kRenderContext.m_fZoom;
    while (true)
    {
        kRenderContext.m_uiDrawnItems = 0;

        PROFILE(Input, cpp_conv::input::receiveInput(kSceneContext, kRenderContext, commands));
        PROFILE(CommandProcess, cpp_conv::command::processCommands(kSceneContext, kRenderContext, commands));
        PROFILE(Simulation, cpp_conv::simulation::simulate(kSceneContext));
        PROFILE(ResizeSwap, [&]()
        {
            int iNewWidth;
            int iNewHeight;
            std::tie(iNewWidth, iNewHeight) = atlas::app_host::Application::Get().GetAppDimensions();
            if (swapChain.RequiresResize(kRenderContext, iNewWidth, iNewHeight))
            {
                swapChain.ResizeBuffers(kRenderContext, iNewWidth, iNewHeight);
            }
        }());

        PROFILE(Render, cpp_conv::renderer::render(kSceneContext, kRenderContext));
        PROFILE(DrawUI, cpp_conv::ui::drawUI(kSceneContext, kRenderContext));
        PROFILE(Present, swapChain.SwapAndPresent());

        PROFILE(FrameCapSleep, frameLimiter.Limit());
        PROFILE(UpdatePersistence, cpp_conv::resources::resource_manager::updatePersistenceStore());
        frameLimiter.EndFrame();
    }

    cpp_conv::ui::shutdown();*/
    return 0;
}
