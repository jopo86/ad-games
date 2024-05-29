#pragma warning(disable: 4244; disable: 305)

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

const float BALL_SPAWN_INTERVAL = 0.1f;
const int BALL_RADIUS = 10, BALL_SEGMENTS = 5;
const float BALL_SPEED = 500.0f;
const float BALL_ROT_SPEED_MIN = 50.0f, BALL_ROT_SPEED_MAX = 200.0f;

const float BOULDER_OUTLINE_RATIO = 1.1f;
const float BOULDER_SPAWN_INTERVAL = 2.0f;
const int	BOULDER_STARTING_HEALTH = 5, BOULDER_STARTING_RADIUS = 50, BOULDER_STARTING_SEGMENTS = 5;
const float BOUDLER_HEALTH_RANGE = 5, BOULDER_HEALTH_INC = 5;
const float BOULDER_RADIUS_RANGE = 25, BOULDER_RADIUS_INC = 0.3;
const float BOULDER_SEGMENTS_RANGE = 2.0f, BOULDER_SEGMENTS_INC = 0.02f;
const float BOULDER_VEL_MIN_X = 200.0f, BOULDER_VEL_MAX_X = 400.0f;
const float BOULDER_VEL_MIN_Y = 0.0f, BOULDER_VEL_MAX_Y = 50.0f;
const float BOULDER_ROT_SPEED_MIN = 20.0f, BOULDER_ROT_SPEED_MAX = 100.0f;

const float STRAFE_SPEED = 200.0f, STRAFE_RANGE = SCR_WIDTH / 2.0f - CANNON_BODY_WIDTH / 2.0f - 50.0f;
const float DAMAGE_INC = 0.25f;

const float BL_TEXT_PADDING = 20.0f, BL_TEXT_SCALE = 0.2f;

const float GRAVITY = -100.0f;

void CannonGame::Run()
{
	Monitor monitor = Monitor::GetPrimary();

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
	window.setPosition(Vec2(monitor.getWidth() / 2 - SCR_WIDTH / 2, monitor.getHeight() / 2 - SCR_HEIGHT / 2));

	Cursor crosshair = Cursor::Standard(CursorType::Crosshair);
	window.setCursor(crosshair);

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

	Font font = Font::Load(Resources("fonts/Poppins/Poppins-Bold.ttf"), 256);

	TextRenderable3D nMissedText("0", font, Vec4::Red(0.8f));
	nMissedText.setScale(BL_TEXT_SCALE);
	nMissedText.setPosition(Vec3(BL_TEXT_PADDING, BL_TEXT_PADDING, 0.1f));

	TextRenderable3D nDestroyedText("0", font, Vec4::White(0.8f));
	nDestroyedText.setScale(BL_TEXT_SCALE);
	nDestroyedText.setPosition(Vec3(BL_TEXT_PADDING, BL_TEXT_PADDING + nMissedText.getHeight() + BL_TEXT_PADDING, 0.1f));

	renderer.add(nMissedText);
	renderer.add(nDestroyedText);

	Vec3 colors[] = { Vec3::Red(), Vec3::Orange(), Vec3::Green(), Vec3::Blue(), Vec3::Cyan(), Vec3::Magenta(), Vec3::Pink(), Vec3::Purple(), Vec3::Brown() };

	float boulderSpawnTimer = 0.0f;
	float boulderHealthMin = BOULDER_STARTING_HEALTH, boulderRadiusMin = BOULDER_STARTING_RADIUS, boulderSegmentsMin = BOULDER_STARTING_SEGMENTS;

	float damage = 1.0f;
	bool ballRotEnabled = true;

	int nDestroyed = 0, nMissed = 0;

	float ballSpawnTimer = 0.0f;

	while (window.isOpen())
	{
		double dt = window.getDeltaTime();
		boulderSpawnTimer += dt;
		ballSpawnTimer += dt;

		if (boulderSpawnTimer >= BOULDER_SPAWN_INTERVAL)
		{
			boulderSpawnTimer = BOULDER_SPAWN_INTERVAL - boulderSpawnTimer;

			bool left = Rand<int>(0, 1);
			float rot = Rand<float>(0.0f, 360.0f);
			float rotStep = Rand<float>(BOULDER_ROT_SPEED_MIN, BOULDER_ROT_SPEED_MAX);
			if (left) rotStep = -rotStep;
			float radius = Rand<float>(boulderRadiusMin, boulderRadiusMin + BOULDER_RADIUS_RANGE);
			int nSegments = Rand<int>(boulderSegmentsMin, boulderSegmentsMin + BOULDER_SEGMENTS_RANGE);
			int health = Rand<int>(boulderHealthMin, boulderHealthMin + BOUDLER_HEALTH_RANGE);
			Vec2 vel, pos;
			if (left)
			{
				vel = Vec2(Rand<float>(BOULDER_VEL_MIN_X, BOULDER_VEL_MAX_X), Rand<float>(BOULDER_VEL_MIN_Y, BOULDER_VEL_MAX_Y));
				pos = Vec2(-radius, SCR_HEIGHT - 100.0f);
			}
			else
			{
				vel = Vec2(-Rand<float>(BOULDER_VEL_MIN_X, BOULDER_VEL_MAX_X), Rand<float>(BOULDER_VEL_MIN_Y, BOULDER_VEL_MAX_Y));
				pos = Vec2(SCR_WIDTH + radius, SCR_HEIGHT - 100.0f);
			}
			boulders.push_back(Boulder(vel, pos, rot, rotStep, radius, nSegments, health, colors[Rand<int>(0, sizeof(colors) / sizeof(Vec3) - 1)], &font));

			boulderHealthMin += BOULDER_HEALTH_INC;
			boulderRadiusMin += BOULDER_RADIUS_INC;
			boulderSegmentsMin += BOULDER_SEGMENTS_INC;

			damage += DAMAGE_INC;
		}

		double deg = Degrees(Atan2(input.getMousePos().getY() - cannonBarrelRenderable.getPosition().getY(), input.getMousePos().getX() - cannonBarrelRenderable.getPosition().getX()));
		if (deg < 0) deg += 360;
		deg -= 90;
		if (deg > 180 && deg < 270) deg = -90;

		if (ballSpawnTimer >= BALL_SPAWN_INTERVAL)
		{
			ballSpawnTimer = BALL_SPAWN_INTERVAL - ballSpawnTimer;
			Vec2 pos(cannonBarrelRenderable.getPosition().getX(), cannonBarrelRenderable.getPosition().getY());
			Vec2 dir = Vec2(Cos(Radians(Clamp(deg, -60, 60) + 90)), Sin(Radians(Clamp(deg, -60, 60) + 90))).getNormalized();
			pos += dir * (3.0f * CANNON_BARREL_HEIGHT / 4.0f - BALL_RADIUS);
			cannonBalls.push_back(CannonBall(dir * BALL_SPEED, pos, ballRotEnabled ? Rand<float>(0.0f, 360.0f) : 0.0f, ballRotEnabled ? Rand<float>(BALL_ROT_SPEED_MIN, BALL_ROT_SPEED_MAX) : 0.0f, &cannonBallRenderable));
		}

		input.update();
		if (input.isKeyTapped(Key::Escape)) window.close();
		if (input.isKeyTapped(Key::F1)) Renderer::ToggleWireframe();
		if (input.isKeyDown(Key::A) || input.isKeyDown(Key::ArrowLeft))
		{
			cannonBodyRenderable.translate(Vec3(-STRAFE_SPEED * dt, 0.0f, 0.0f));
			cannonBarrelRenderable.translate(Vec3(-STRAFE_SPEED * dt, 0.0f, 0.0f));
			if (cannonBodyRenderable.getPosition().getX() < CANNON_BODY_WIDTH / 2) 
			{
				cannonBodyRenderable.setPosition(Vec3(CANNON_BODY_WIDTH / 2, cannonBodyRenderable.getPosition().getY(), 0.0f));
				cannonBarrelRenderable.setPosition(Vec3(CANNON_BODY_WIDTH / 2, cannonBarrelRenderable.getPosition().getY(), 0.0f));
			}
		}
		if (input.isKeyDown(Key::D) || input.isKeyDown(Key::ArrowRight))
		{
			cannonBodyRenderable.translate(Vec3(STRAFE_SPEED * dt, 0.0f, 0.0f));
			cannonBarrelRenderable.translate(Vec3(STRAFE_SPEED * dt, 0.0f, 0.0f));
			if (cannonBodyRenderable.getPosition().getX() > SCR_WIDTH - CANNON_BODY_WIDTH / 2)
			{
				cannonBodyRenderable.setPosition(Vec3(SCR_WIDTH - CANNON_BODY_WIDTH / 2, cannonBodyRenderable.getPosition().getY(), 0.0f));
				cannonBarrelRenderable.setPosition(Vec3(SCR_WIDTH - CANNON_BODY_WIDTH / 2, cannonBarrelRenderable.getPosition().getY(), 0.0f));
			}
		}

		cam.update();

		//std::cout << cannonBalls.size() << "\n";

		cannonBarrelRenderable.setRotation(Vec3(0.0f, 0.0f, Clamp(deg, -60, 60)));

		nMissedText.setText(std::to_string(nMissed));
		nDestroyedText.setText(std::to_string(nDestroyed));

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
						boulder.damage((int)damage);
						cannonBalls.erase(ballIt++);
					} 
					else ballIt++;
				}

				if (boulder.destroyed)
				{
					boulder.dispose();
					boulders.erase(boulderIt++);
					nDestroyed++;
				}
				else if (boulder.pos.getY() < FLOOR_HEIGHT - boulder.radius)
				{
					boulder.dispose();
					boulders.erase(boulderIt++);
					nMissed++;
				}
				else
				{
					boulder.update(dt);
					boulder.render(cam);
					boulderIt++;
				}
			}
		}

		window.endRender();
	}

	window.dispose();
	renderer.dispose();
	crosshair.dispose();
	font.dispose();
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
	renderable->setPosition(Vec3(pos.getX(), pos.getY(), -0.5f));
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
	font = nullptr;
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

	outer = Renderable::ColoredCircle(radius * BOULDER_OUTLINE_RATIO, nSegments, Vec3::Black());
	outer.setPosition(Vec3(pos.getX(), pos.getY(), -1.0f));
	outer.setRotation(Vec3(0.0f, 0.0f, rot));

	inner = Renderable::ColoredCircle(radius, nSegments, color);
	inner.setPosition(Vec3(pos.getX(), pos.getY(), -0.9f));
	inner.setRotation(Vec3(0.0f, 0.0f, rot));

	text = TextRenderable3D(std::to_string(health), *font, Vec4::White(0.8f));
	text.setPosition(Vec3(pos.getX() - text.getWidth() / 2.0f, pos.getY() - text.getHeight() / 2.0f, -0.7f));
	text.setScale(radius / 50.0f * 32 / font->getSize());
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
