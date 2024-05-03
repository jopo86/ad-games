#pragma warning(disable: 4244)

#include "CubeRunnerGame.h"

#include <thread>

#include <Onyx/Core.h>
#include <Onyx/Window.h>
#include <Onyx/InputHandler.h>
#include <Onyx/Math.h>
#include <Onyx/FileUtils.h>

using Onyx::Math::Vec2, Onyx::Math::Vec3, Onyx::Math::Vec4, Onyx::Math::IVec2;

bool collision(const Onyx::ModelRenderable& player, const Onyx::ModelRenderable& spike);

void CubeRunnerGame::Run()
{
	Onyx::Window window(
		Onyx::WindowProperties{
			.title = "Cube Runner Game",
			.width = 1280,
			.height = 720,
			.position = IVec2(2560 / 2 - 1280 / 2, 1440 / 2 - 720 / 2), // fix in rc4
			.nSamplesMSAA = 4
		}
	);

	window.init();
	window.setBackgroundColor(Vec3::LightBlue());
	window.setPosition(IVec2(2560 / 2 - 1280 / 2, 1440 / 2 - 720 / 2));

	Onyx::InputHandler input;
	window.linkInputHandler(input);

	Onyx::Camera cam(Onyx::Projection::Perspective(60.0f, 1280, 720));
	window.linkCamera(cam);
	cam.translateFB(-5.0f);
	cam.translateUD(3.0f);
	cam.pitch(-15.0f);

	Onyx::Lighting lighting(Vec3::White(), 0.3f, Vec3(0.2f, -1.0f, -0.3f));

	Onyx::Renderer renderer(cam, lighting);
	window.linkRenderer(renderer);

	Onyx::Model playerModel = Onyx::Model::LoadOBJ(Onyx::Resources("models/player.obj"));
	Onyx::Model spikeModel = Onyx::Model::LoadOBJ(Onyx::Resources("models/spike.obj"));

	Onyx::Renderable floor = Onyx::Renderable::ColoredQuad(10.0f, 100.0f, Vec4::White());
	floor.rotate(Vec3(90.0f, 0.0f, 0.0f));
	floor.translate(Vec3(0.0f, -0.001f, -40.0f));

	Onyx::ModelRenderable player(playerModel);
	player.translate(Vec3(0.0f, 0.2f, -0.0f));
	player.scale(0.4f);

	srand(time(nullptr));

	std::vector<Onyx::ModelRenderable> spikes;
	for (int i = 0; i < 10; i++)
    {
        Onyx::ModelRenderable spike = Onyx::ModelRenderable(spikeModel);
        spike.translate(Vec3((rand() % 900) / 100.0f - 4.5f, 0.0f, -40.0f - i * 5.0f));
		spike.scale(0.5f);
        spikes.push_back(spike);
    }

	Onyx::Font font = Onyx::Font::Load(Onyx::Resources("fonts/Poppins/Poppins-Regular.ttf"), 48);
	Onyx::TextRenderable scoreText("0", font, Vec3(0.2f));
	scoreText.setPosition(Vec2(20, 720 - 60));

	renderer.add(floor);
	renderer.add(player);
	for (auto& spike : spikes) renderer.add(spike);
	renderer.add(scoreText);


	const double CAM_SPEED = 6.0f;
	const double CAM_SENS = 50.0f;

	float spawnInterval = 1.0f;

	float playerSpeed = 5.0f;
	const float PLAYER_STRAFE_LIMIT = 4.25f;

	float spikeSpeed = 10.0f;

	const bool CAM_MOVEMENT = false;

	float score = 0.0f;

	input.setKeyCooldown(Onyx::Key::F12, 0.5f);
	input.setCursorLock(true);

	while (window.isOpen())
	{
		double dt = window.getDeltaTime();

		input.update();

		if (CAM_MOVEMENT)
		{
			if (input.isKeyDown(Onyx::Key::W)) cam.translateFB(CAM_SPEED * dt);
			if (input.isKeyDown(Onyx::Key::A)) cam.translateLR(-CAM_SPEED * dt);
			if (input.isKeyDown(Onyx::Key::S)) cam.translateFB(-CAM_SPEED * dt);
			if (input.isKeyDown(Onyx::Key::D)) cam.translateLR(CAM_SPEED * dt);
			if (input.isKeyDown(Onyx::Key::Space)) cam.translateUD(CAM_SPEED * dt);
			if (input.isKeyDown(Onyx::Key::C)) cam.translateUD(-CAM_SPEED * dt);

			cam.rotate(input.getMouseDeltas().getX() / 200.0f * CAM_SENS, input.getMouseDeltas().getY() / 200.0f * CAM_SENS);
		}
		else
		{
			if (input.isKeyDown(Onyx::Key::A) || input.isKeyDown(Onyx::Key::ArrowLeft)) {
				player.translate(Vec3(-playerSpeed * dt, 0.0f, 0.0f));
				if (player.getPosition().getX() < -PLAYER_STRAFE_LIMIT) player.setPosition(Vec3(-PLAYER_STRAFE_LIMIT, player.getPosition().getY(), player.getPosition().getZ()));
			}
			if (input.isKeyDown(Onyx::Key::D) || input.isKeyDown(Onyx::Key::ArrowRight)) {
				player.translate(Vec3(playerSpeed * dt, 0.0f, 0.0f));
				if (player.getPosition().getX() > PLAYER_STRAFE_LIMIT) player.setPosition(Vec3(PLAYER_STRAFE_LIMIT, player.getPosition().getY(), player.getPosition().getZ()));
			}
		}
		cam.update();

		if (input.isKeyDown(Onyx::Key::Escape)) window.close();
		if (input.isKeyDown(Onyx::Key::F12)) window.toggleFullscreen(1280, 720, IVec2(2560 / 2 - 1280 / 2, 1440 / 2 - 720 / 2));

		for (auto& spike : spikes)
		{
			spike.translate(Vec3(0.0f, 0.0f, spikeSpeed * dt));
			if (spike.getPosition().getZ() > 10.0f)
            {
                spike.setPosition(Vec3((rand() % 900) / 100.0f - 4.5f, 0.0f, -40.0f));
            }

			if (collision(player, spike)) window.close();
		}

		score += dt * 20.0f;
		scoreText.setText(std::to_string((int)score));

		spikeSpeed += dt * 0.1f;
		playerSpeed += dt * 0.05f;

		window.startRender();
		renderer.render();
		window.endRender();
	}

	window.dispose();
	renderer.dispose();
}

bool collision(const Onyx::ModelRenderable& player, const Onyx::ModelRenderable& spike)
{
	float dist = (player.getPosition() - spike.getPosition()).magnitude();
	return dist < (1.0f * player.getScale().getX() + 0.8f * spike.getScale().getX());
}
