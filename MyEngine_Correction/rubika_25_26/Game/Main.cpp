#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Rect.hpp>

#include <Engine/Render/Ressource/TextureMgr.h>
#include <Engine/Console/LogConsole.h>
#include <Engine/Globals.h>
#include <Engine/Gameplay/GameMgr.h>

#include <Engine/Gameplay/Entity/Entity.h>
#include <Engine/Gameplay/Component/Transform/Transform.h>
#include <Engine/Gameplay/Component/Renderer/Renderer.h>
#include <Engine/Render/Drawable/Sprite/Sprite.h>

#ifdef _USE_IMGUI
#include "Imgui/imgui.h"
#include "Imgui/imgui-SFML.h"
#endif

#include <Engine/Profiler.h>

Entity* CreateEntity()
{
    Entity* e = new Entity();

    Transform* TransformComp = e->AddComponent<Transform>();
    Renderer* RendererComp = e->AddComponent<Renderer>();

    Sprite* Body = RendererComp->AddNewDrawable<Sprite>("Body", sf::Vector2f(2, 0), 0, sf::Vector2f(1, 1));
    Body->SetVisibility(true);
    Body->SetTexture("../Ressources/IsaacSprite.png");
    Body->SetAnimation("Body_Vertical");

    Sprite* Head = RendererComp->AddNewDrawable<Sprite>("Head", sf::Vector2f(-2.5f, -20), 0, sf::Vector2f(1, 1));
    Head->SetVisibility(true);
    Head->SetTexture("../Ressources/IsaacSprite.png");
    Head->SetAnimation("Head_Down");

    TransformComp->SetWorldPosition(sf::Vector2f(150.f, 150.f));

    return e;
}

int main()
{
    gData.Init();

    sf::RenderWindow window(sf::VideoMode({ 1280, 720 }), "MyEngine - Rubika 2025 2026");
    
#ifdef _USE_IMGUI
    if (!ImGui::SFML::Init(window))
    {
        return -1;
    }
#endif

    if (!gData.TextureMgr->LoadTexture("../Ressources/IsaacSprite.png"))
    {
        return -1;
    }
    
    if (!gData.TextureMgr->LoadTexture("../Ressources/Basement.png"))
    {
        return -2;
    }

    if (!gData.TextureMgr->LoadTexture("../Ressources/Tear.png"))
    {
        return -2;
    }

    if (!gData.TextureMgr->LoadTexture("../Ressources/Rocks.png"))
    {
        return -3;
    }

    if (!gData.TextureMgr->LoadTexture("../Ressources/Doors.png"))
    {
        return -4;
    }

    Entity* entity = CreateEntity();

    gData.GameMgr->AddEntity(entity);

    sf::Clock clock;
    clock.restart();

    while (window.isOpen() && !gData.ExipApp)
    {
        PROFILER_EVENT_BEGIN(PROFILER_COLOR_BLACK, "Frame %llu", gData.FrameCount);
        {
            int deltaTimeMS = clock.getElapsedTime().asMilliseconds();
            float fDeltaTimeS = (float)deltaTimeMS / 1000.f;
            sf::Time imGuiTime = clock.restart();

            PROFILER_EVENT_BEGIN(PROFILER_COLOR_BLUE, "Event & Input");
            {
                while (const std::optional event = window.pollEvent())
                {

                    if (event->is<sf::Event::Closed>() || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape))
                    {
                        window.close();
                        gData.ExipApp = true;
                    }

#ifdef _USE_IMGUI
                    ImGui::SFML::ProcessEvent(window, *event);
#endif
                }
            }
            PROFILER_EVENT_END();

            PROFILER_EVENT_BEGIN(PROFILER_COLOR_RED, "Update");
            {
#ifdef _USE_IMGUI
                //Sleep(2000);
                ImGui::SFML::Update(window, imGuiTime);
#endif

                gData.GameMgr->Update(fDeltaTimeS);
            }
            PROFILER_EVENT_END();

            PROFILER_EVENT_BEGIN(PROFILER_COLOR_GREEN, "Draw");
            {
                PROFILER_EVENT_BEGIN(PROFILER_COLOR_BROWN, "Debug Draw");
                //Sleep(2000);
                gData.DebugMgr->Draw();
                PROFILER_EVENT_END();

                window.clear();

                PROFILER_EVENT_BEGIN(PROFILER_COLOR_PURPLE, "Game Draw");
                gData.GameMgr->Draw(window);
                PROFILER_EVENT_END();

#ifdef _USE_IMGUI
                PROFILER_EVENT_BEGIN(PROFILER_COLOR_ORANGE, "ImGui Draw");
                ImGui::SFML::Render(window);
                PROFILER_EVENT_END();
#endif
                PROFILER_EVENT_BEGIN(PROFILER_COLOR_CYAN, "Window Display");
                window.display();
                PROFILER_EVENT_END();

            }
            PROFILER_EVENT_END();
        }
        PROFILER_EVENT_END();
        ++gData.FrameCount;
    }

    gData.ExipApp = true;

    gData.Shut();
    gData.Destroy();
#ifdef _USE_IMGUI
    ImGui::SFML::Shutdown();
#endif

    return 0;
}