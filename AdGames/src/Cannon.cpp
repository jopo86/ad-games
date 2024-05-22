#include "Cannon.h"

#include <Onyx/Core.h>
#include <Onyx/Window.h>
#include <Onyx/InputHandler.h>
#include <Onyx/Math.h>

using namespace Onyx;
using namespace Onyx::Math;

const int SCR_WIDTH = 1280, SCR_HEIGHT = 720;

const int FLOOR_HEIGHT = 150;
const int CANNON_BODY_WIDTH = 100, CANNON_BODY_HEIGHT = 50;
const int CANNON_BARREL_WIDTH = 30, CANNON_BARREL_HEIGHT = 100;

void Cannon::Run()
{
    Window window(
        WindowProperties{
            .title = "Cannon",
            .width = SCR_WIDTH,
            .height = SCR_HEIGHT,
            .nSamplesMSAA = 16,
            .backgroundColor = Vec3::LightBlue()
        }
    );

    window.init();

    InputHandler input;
    window.linkInputHandler(input);

    Camera cam(Projection::Orthographic(SCR_WIDTH, SCR_HEIGHT));
    window.linkCamera(cam);

    Renderer renderer(cam);
    window.linkRenderer(renderer);

    Renderable floor = Renderable::ColoredQuad(SCR_WIDTH, FLOOR_HEIGHT, Vec3(0.5f, 0.8f, 0.0f));
    floor.translate(Vec3(SCR_WIDTH / 2, FLOOR_HEIGHT / 2, 0.0f));
    renderer.add(floor);

    Renderable cannonBody = Renderable::ColoredQuad(100, 50, Vec3(0.0f, 0.0f, 0.0f));

    while (window.isOpen())
    {
        input.update();
        if (input.isKeyTapped(Key::Escape)) window.close();

        cam.update();

        // Handle any Logic/Transformations

        window.startRender();
        renderer.render();
        window.endRender();
    }

    window.dispose();
    renderer.dispose();
}
