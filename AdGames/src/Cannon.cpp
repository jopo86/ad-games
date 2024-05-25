#include "Cannon.h"

#include <list>

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
const int BALL_RADIUS = 5;

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

    Renderable cannonBody = Renderable::ColoredQuad(100, 50, Vec3::Brown());
    cannonBody.translate(Vec3(SCR_WIDTH / 2, FLOOR_HEIGHT + CANNON_BODY_HEIGHT / 2, 0.0f));
    renderer.add(cannonBody);

    float vertices[12] = {
        -CANNON_BARREL_WIDTH / 2, -CANNON_BARREL_HEIGHT / 4.0f, 0.0f,
         CANNON_BARREL_WIDTH / 2, -CANNON_BARREL_HEIGHT / 4.0f, 0.0f,
         CANNON_BARREL_WIDTH / 2, 3 * CANNON_BARREL_HEIGHT / 4.0f, 0.0f,
        -CANNON_BARREL_WIDTH / 2, 3 * CANNON_BARREL_HEIGHT / 4.0f, 0.0f
    };

    Renderable cannonBarrel(
        Mesh(
            VertexBuffer(vertices, sizeof(vertices), VertexFormat::P),
            IndexBuffer::Quad()
        ),
        Shader::P_Color(Vec4(Vec3::LightGray() * 0.65f, 1.0f))
    );
    cannonBarrel.translate(Vec3(SCR_WIDTH / 2, FLOOR_HEIGHT + CANNON_BODY_HEIGHT, 0.0f));
    renderer.add(cannonBarrel);

    Renderable cannonBall = Renderable::ColoredCircle(BALL_RADIUS, BALL_RADIUS, Vec4::Black());

    std::list<std::pair<Vec2, Vec2>> cannonBalls;

    input.setMouseButtonCooldown(MouseButton::Left, 0.1f);

    while (window.isOpen())
    {
        double dt = window.getDeltaTime();

        double deg = Degrees(atan2(input.getMousePos().getY() - cannonBarrel.getPosition().getY(), input.getMousePos().getX() - cannonBarrel.getPosition().getX()));
        if (deg < 0) deg += 360;
        deg -= 90;
        if (deg > 180 && deg < 270) deg = -90;

        input.update();
        if (input.isKeyTapped(Key::Escape)) window.close();
        if (input.isKeyTapped(Key::F1)) Renderer::ToggleWireframe();
        if (input.isMouseButtonPressed(MouseButton::Left))
        {
            Vec2 pos(cannonBarrel.getPosition().getX(), cannonBarrel.getPosition().getY());
			Vec2 dir = Vec2(cos(Radians(Clamp(deg, -60, 60) + 90)), sin(Radians(Clamp(deg, -60, 60) + 90))).getNormalized();
            pos += dir *  (3.0f * CANNON_BARREL_HEIGHT / 4.0f);
			cannonBalls.push_back({ pos, dir * 500.0f });
        }

        cam.update();

        cannonBarrel.setRotation(Vec3(0.0f, 0.0f, Clamp(deg, -60, 60)));

        window.startRender();
        renderer.render(); 
        
        std::list<std::pair<Vec2, Vec2>>::iterator it = cannonBalls.begin();
        while (it != cannonBalls.end())
        {
            auto& ball = *it;
            if (ball.first.getX() > SCR_WIDTH + BALL_RADIUS || ball.first.getX() < -BALL_RADIUS || ball.first.getY() > SCR_HEIGHT + BALL_RADIUS)
            {
                cannonBalls.erase(it++);
            }

            ball.first += ball.second * dt;
            cannonBall.setPosition(Vec3(ball.first.getX(), ball.first.getY(), 0.0f));
            cannonBall.render(cam.getViewMatrix(), cam.getProjectionMatrix(), cam.getPosition());

            it++;
        }
        window.endRender();
    }

    window.dispose();
    renderer.dispose();
}
