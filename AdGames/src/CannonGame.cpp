#include "CannonGame.h"

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
const int BALL_RADIUS = 10, BALL_SEGMENTS = 5;

const float GRAVITY = -100.0f;

void CannonGame::Run()
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

	Renderable cannonBodyRenderable = Renderable::ColoredQuad(100, 50, Vec3::Brown());
	cannonBodyRenderable.translate(Vec3(SCR_WIDTH / 2, FLOOR_HEIGHT + CANNON_BODY_HEIGHT / 2, 0.0f));
	renderer.add(cannonBodyRenderable);

	float vertices[12] = {
		-CANNON_BARREL_WIDTH / 2,    -CANNON_BARREL_HEIGHT / 4.0f, 0.0f,
		 CANNON_BARREL_WIDTH / 2,    -CANNON_BARREL_HEIGHT / 4.0f, 0.0f,
		 CANNON_BARREL_WIDTH / 2, 3 * CANNON_BARREL_HEIGHT / 4.0f, 0.0f,
		-CANNON_BARREL_WIDTH / 2, 3 * CANNON_BARREL_HEIGHT / 4.0f, 0.0f
	};

	Renderable cannonBarrelRenderable(
		Mesh(
			VertexBuffer(vertices, sizeof(vertices), VertexFormat::P),
			IndexBuffer::Quad()
		),
		Shader::P_Color(Vec4(Vec3::LightGray() * 0.65f, 1.0f))
	);
	cannonBarrelRenderable.translate(Vec3(SCR_WIDTH / 2, FLOOR_HEIGHT + CANNON_BODY_HEIGHT, 0.0f));
	renderer.add(cannonBarrelRenderable);

	Renderable cannonBallRenderable = Renderable::ColoredCircle(BALL_RADIUS, BALL_SEGMENTS, Vec4::Black());

	std::list<CannonBall> cannonBalls;
	std::list<Boulder> boulders;

	input.setMouseButtonCooldown(MouseButton::Left, 0.1f);

	Font font = Font::Load(Resources("fonts/Poppins/Poppins-Bold.ttf"), 256);

	boulders.push_back(Boulder(Vec2(200.0f, 100.0f), Vec2(0.0f, SCR_HEIGHT - 100.0f), 0.0f, 20.0f, 50.0f, 5, 10, Vec3::Red(), &font));
	boulders.push_back(Boulder(Vec2(-200.0f, 100.0f), Vec2(SCR_WIDTH, SCR_HEIGHT - 100.0f), 0.0f, -20.0f, 20.0f, 6, 50, Vec3::Green(), &font));
	boulders.push_back(Boulder(Vec2(0.0f, 0.0f), Vec2(SCR_WIDTH / 2, SCR_HEIGHT - 100.0f), 0.0f, 0.0f, 80.0f, 5, 20, Vec3::Blue(), &font));

	while (window.isOpen())
	{
		double dt = window.getDeltaTime();

		double deg = Degrees(Atan2(input.getMousePos().getY() - cannonBarrelRenderable.getPosition().getY(), input.getMousePos().getX() - cannonBarrelRenderable.getPosition().getX()));
		if (deg < 0) deg += 360;
		deg -= 90;
		if (deg > 180 && deg < 270) deg = -90;

		input.update();
		if (input.isKeyTapped(Key::Escape)) window.close();
		if (input.isKeyTapped(Key::F1)) Renderer::ToggleWireframe();
		if (input.isMouseButtonPressed(MouseButton::Left))
		{
			Vec2 pos(cannonBarrelRenderable.getPosition().getX(), cannonBarrelRenderable.getPosition().getY());
			Vec2 dir = Vec2(Cos(Radians(Clamp(deg, -60, 60) + 90)), Sin(Radians(Clamp(deg, -60, 60) + 90))).getNormalized();
			pos += dir *  (3.0f * CANNON_BARREL_HEIGHT / 4.0f - BALL_RADIUS);
			cannonBalls.push_back(CannonBall(dir * 500.0f, pos, Math::Rand<float>(0.0f, 360.0f), Math::Rand<float>(50.0f, 360.0f), &cannonBallRenderable));
		}

		cam.update();

		//std::cout << cannonBalls.size() << "\n";

		cannonBarrelRenderable.setRotation(Vec3(0.0f, 0.0f, Clamp(deg, -60, 60)));

		window.startRender();
		renderer.render(); 
		
		auto ballIt = cannonBalls.begin();
		while (ballIt != cannonBalls.end())
		{
			CannonBall& ball = *ballIt;
			if (ball.pos.getX() > SCR_WIDTH + BALL_RADIUS || ball.pos.getX() < -BALL_RADIUS || ball.pos.getY() > SCR_HEIGHT + BALL_RADIUS)
			{
				cannonBalls.erase(ballIt++);
			}
			else
			{
				ball.update(dt);
				ball.render(cam);
				ballIt++;
			}
		}

		auto boulderIt = boulders.begin();
		while (boulderIt != boulders.end())
		{
			Boulder& boulder = *boulderIt;
			if (boulder.destroyed)
			{
				boulder.dispose();
				boulders.erase(boulderIt++);
			}
			else
			{
				ballIt = cannonBalls.begin();
				while (ballIt != cannonBalls.end())
				{
					CannonBall& ball = *ballIt;
					if (boulder.collision(ball))
					{
						boulder.damage(1);
						cannonBalls.erase(ballIt++);
					} 
					ballIt++;
				}

				if (boulder.destroyed)
				{
					boulder.dispose();
					boulders.erase(boulderIt++);
				}
				else if (boulder.pos.getY() < FLOOR_HEIGHT - boulder.radius)
				{
					boulder.dispose();
					boulders.erase(boulderIt++);
				}

				boulder.update(dt);
				boulder.render(cam);
				boulderIt++;
			}
		}

		window.endRender();
	}

	window.dispose();
	renderer.dispose();
}

CannonGame::CannonBall::CannonBall()
{
	rot = rotStep = 0.0f;
	renderable = nullptr;
}

CannonGame::CannonBall::CannonBall(Vec2 vel, Vec2 pos, float rot, float rotStep, Renderable* renderable)
{
	this->vel = vel;
	this->pos = pos;
	this->rot = rot;
	this->rotStep = rotStep;
	this->renderable = renderable;
}

void CannonGame::CannonBall::update(float dt)
{
	pos += vel * dt;
	rot += rotStep * dt;
	renderable->setPosition(Vec3(pos.getX(), pos.getY(), -1.0f));
	renderable->setRotation(Vec3(0.0f, 0.0f, rot));
}

void CannonGame::CannonBall::render(const Camera& cam)
{
	renderable->render(cam.getViewMatrix(), cam.getProjectionMatrix(), cam.getPosition());
}

CannonGame::Boulder::Boulder()
{
	rot = rotStep = radius = 0.0f;
	nSegments = health = 0;
	destroyed = false;
}

CannonGame::Boulder::Boulder(Vec2 vel, Vec2 pos, float rot, float rotStep, float radius, int nSegments, int health, Vec3 color, Font* font)
{
	this->vel = vel;
	this->pos = pos;
	this->rot = rot;
	this->rotStep = rotStep;
	this->radius = radius;
	this->nSegments = nSegments;
	this->health = health;
	this->color = color;
	this->font = font;
	destroyed = false;

	outer = Renderable::ColoredCircle(radius * 1.1f, nSegments, Vec3::Black());
	outer.setPosition(Vec3(pos.getX(), pos.getY(), -1.0f));
	outer.setRotation(Vec3(0.0f, 0.0f, rot));

	inner = Renderable::ColoredCircle(radius, nSegments, color);
	inner.setPosition(Vec3(pos.getX(), pos.getY(), -0.9f));
	inner.setRotation(Vec3(0.0f, 0.0f, rot));

	text = TextRenderable3D(std::to_string(health), *font, Vec3::White());
	text.setPosition(Vec3(pos.getX() - text.getWidth() / 2.0f, pos.getY() - text.getHeight() / 2.0f, -0.7f));
	text.setScale(radius / 50.0f * 48 / font->getSize());
}

void CannonGame::Boulder::update(float dt)
{
	vel += Vec2(0.0f, GRAVITY * dt);
	pos += vel * dt;
	rot += rotStep * dt;

	outer.setPosition(Vec3(pos.getX(), pos.getY(), -1.0f));
	outer.setRotation(Vec3(0.0f, 0.0f, rot));

	inner.setPosition(Vec3(pos.getX(), pos.getY(), -0.9f));
	inner.setRotation(Vec3(0.0f, 0.0f, rot));

	text.setPosition(Vec3(pos.getX() - text.getWidth() / 2.0f, pos.getY() - text.getHeight() / 2.0f, -0.7f));
}

void CannonGame::Boulder::render(const Camera& cam)
{
	outer.render(cam.getViewMatrix(), cam.getProjectionMatrix(), cam.getPosition());
	inner.render(cam.getViewMatrix(), cam.getProjectionMatrix(), cam.getPosition());
	text.render(cam.getViewMatrix(), cam.getProjectionMatrix(), cam.getPosition());
}

void CannonGame::Boulder::damage(int amount)
{
	health -= amount;
	text.setText(std::to_string(health));
	text.setPosition(Vec3(pos.getX() - text.getWidth() / 2.0f, pos.getY() - text.getHeight() / 2.0f, -0.7f));

	if (health <= 0) destroyed = true;
}

bool CannonGame::Boulder::collision(const CannonBall& ball)
{
	return (pos - ball.pos).magnitude() < radius + BALL_RADIUS;
}

void CannonGame::Boulder::dispose()
{
	outer.dispose();
	inner.dispose();
	text.dispose();
}
