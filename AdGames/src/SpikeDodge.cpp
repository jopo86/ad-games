#pragma warning(disable: 4244)

#include "SpikeDodge.h"

#include "Launcher.h"

#include <Onyx/Core.h>
#include <Onyx/Window.h>
#include <Onyx/InputHandler.h>
#include <Onyx/Math.h>
#include <Onyx/FileUtils.h>
#include <Onyx/Monitor.h>

using Onyx::Math::Vec2, Onyx::Math::Vec3, Onyx::Math::Vec4, Onyx::Math::IVec2;

bool collision(const Onyx::ModelRenderable& player, const Onyx::ModelRenderable& spike);

void SpikeDodge::Run()
{
	Onyx::Init();

	Onyx::Window window(
		Onyx::WindowProperties{
			.title = "Spike Dodge",
			.width = 1280,
			.height = 720,
			.position = IVec2(2560 / 2 - 1280 / 2, 1440 / 2 - 720 / 2),
			.nSamplesMSAA = 4
		}
	);

	window.init();
	window.setBackgroundColor(Vec3::LightBlue());
	Onyx::Monitor monitor = Onyx::Monitor::GetPrimary();
	window.setPosition(IVec2(monitor.getDimensions().getX() / 2 - window.getWidth() / 2, monitor.getDimensions().getY() / 2 - window.getHeight() / 2));

	Onyx::WindowIcon icon = Onyx::WindowIcon::Load({
		Onyx::Resources("icons/icon-16x.png"),
		Onyx::Resources("icons/icon-24x.png"),
		Onyx::Resources("icons/icon-32x.png"),
		Onyx::Resources("icons/icon-48x.png"),
		Onyx::Resources("icons/icon-256x.png")
	});
	window.setIcon(icon);
	icon.dispose();

	Onyx::InputHandler input;
	window.linkInputHandler(input);

	Onyx::Camera cam(Onyx::Projection::Perspective(60.0f, 1280, 720));
	window.linkCamera(cam);
	cam.translateFB(-5.0f);
	cam.translateUD(3.0f);
	cam.pitch(-15.0f);

	Onyx::Lighting lighting(Vec3::White(), 0.3f, Vec3(0.2f, -1.0f, -0.3f));
	Onyx::Fog fog(Vec3::LightBlue(), 40.0f, 90.0f);

	Onyx::Renderer renderer(cam, lighting, fog);
	window.linkRenderer(renderer);

	Onyx::Model playerModel = Onyx::Model::LoadOBJ(Onyx::Resources("models/capsule.obj"));
	Onyx::Model spikeModel = Onyx::Model::LoadOBJ(Onyx::Resources("models/spike.obj"));

	Onyx::Renderable floor = Onyx::Renderable::ColoredQuad(10.0f, 1000.0f, Vec4::White());
	floor.rotate(Vec3(90.0f, 0.0f, 0.0f));
	floor.translate(Vec3(0.0f, -0.001f, -40.0f));

	Onyx::ModelRenderable player(playerModel);
	player.translate(Vec3(0.0f, 0.2f, -0.0f));
	player.scale(0.5f);


	srand(time(nullptr));

	std::vector<Onyx::ModelRenderable> spikes;
	for (int i = 0; i < 20; i++)
    {
        Onyx::ModelRenderable spike = Onyx::ModelRenderable(spikeModel);
        spike.translate(Vec3((rand() % 900) / 100.0f - 4.5f, 0.0f, -20.0f - i * 5.0f));
		spike.scale(0.5f);
		spike.rotate(Vec3(0, rand() % 360, 0));
        spikes.push_back(spike);
    }

	Onyx::Font fontReg = Onyx::Font::Load(Onyx::Resources("fonts/Poppins/Poppins-Regular.ttf"), 96);
	Onyx::Font fontBold = Onyx::Font::Load(Onyx::Resources("fonts/Poppins/Poppins-Bold.ttf"), 96);
	Onyx::TextRenderable scoreText("0", fontReg, Vec3(0.2f));
	scoreText.setScale(0.5f);
	scoreText.setPosition(Vec2(20, 720 - 60));

	Onyx::TextRenderable highScoreText("High Score: 0", fontReg, Vec4(Vec3(0.2f), 0.5f));
	highScoreText.setScale(0.3f);
	highScoreText.setPosition(Vec2(20, 720 - 20 - highScoreText.getHeight()));

	Onyx::TextRenderable gameOverText("GAME OVER", fontBold, Vec4::Red());
	Onyx::TextRenderable gameOverSubText("[R] to restart, [ESC] to exit", fontReg, Vec4::Red());
	gameOverText.setScale(0.6f);
	gameOverSubText.setScale(0.3f);
	gameOverText.hide();
	gameOverSubText.hide();

	renderer.add(floor);
	renderer.add(player);
	for (auto& spike : spikes) renderer.add(spike);
	renderer.add(scoreText);
	renderer.add(highScoreText);
	renderer.add(gameOverText);
	renderer.add(gameOverSubText);

	const double CAM_SPEED = 6.0f;
	const double CAM_SENS = 50.0f;

	float spawnInterval = 1.0f;

	float playerSpeed = 5.0f;
	const float PLAYER_STRAFE_LIMIT = 4.25f;

	float spikeSpeed = 10.0f;

	const bool CAM_MOVEMENT = false; 
	
	const float DEAD_ZONE_LEFT = 0.1f;
	const float DEAD_ZONE_RIGHT = 0.1f;

	float score = 0.0f;
	float highScore;
	if (Onyx::FileUtils::FileExists("data.txt"))
	{
		std::vector<std::string> data = Onyx::FileUtils::ReadLines("data.txt");
		highScore = std::stof(data[0]);
	}
	else highScore = 0.0f;

	input.setCursorLock(true);

	bool dead = false;

	bool restart = false;
	while (window.isOpen())
	{
		double dt = window.getDeltaTime();
		float lsx = 0.0f, lsy = 0.0f, rsx = 0.0f, rsy = 0.0f;
		bool a = false, b = false, x = false, y = false, rs = false;

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
		else if (!dead)
		{
			if (input.isKeyDown(Onyx::Key::A) || input.isKeyDown(Onyx::Key::ArrowLeft)) {
				player.translate(Vec3(-playerSpeed * dt, 0.0f, 0.0f));
			}
			if (input.isKeyDown(Onyx::Key::D) || input.isKeyDown(Onyx::Key::ArrowRight)) {
				player.translate(Vec3(playerSpeed * dt, 0.0f, 0.0f));
			}

			for (const Onyx::Gamepad& gp : input.getGamepads())
			{
				if (abs(gp.getAxis(Onyx::GamepadAxis::LeftX)) > lsx) lsx = gp.getAxis(Onyx::GamepadAxis::LeftX);
				if (abs(gp.getAxis(Onyx::GamepadAxis::LeftY)) > lsy) lsy = gp.getAxis(Onyx::GamepadAxis::LeftY);
				if (abs(gp.getAxis(Onyx::GamepadAxis::RightX)) > rsx) rsx = gp.getAxis(Onyx::GamepadAxis::RightX);
				if (abs(gp.getAxis(Onyx::GamepadAxis::RightY)) > rsy) rsy = gp.getAxis(Onyx::GamepadAxis::RightY);
				if (gp.isButtonDown(Onyx::GamepadButton::A)) a = true;
				if (gp.isButtonDown(Onyx::GamepadButton::B)) b = true;
				if (gp.isButtonDown(Onyx::GamepadButton::X)) x = true;
				if (gp.isButtonDown(Onyx::GamepadButton::Y)) y = true;
				if (gp.isButtonDown(Onyx::GamepadButton::RightStick)) rs = true;
			}

			lsx = abs(lsx) < DEAD_ZONE_LEFT ? 0.0f : lsx;
			lsy = abs(lsy) < DEAD_ZONE_LEFT ? 0.0f : lsy;
			rsx = abs(rsx) < DEAD_ZONE_RIGHT ? 0.0f : rsx;
			rsy = abs(rsy) < DEAD_ZONE_RIGHT ? 0.0f : rsy;

			player.translate(Vec3(lsx * playerSpeed * dt, 0.0f, 0.0f));

			if (player.getPosition().getX() < -PLAYER_STRAFE_LIMIT) player.setPosition(Vec3(-PLAYER_STRAFE_LIMIT, player.getPosition().getY(), player.getPosition().getZ()));
			else if (player.getPosition().getX() > PLAYER_STRAFE_LIMIT) player.setPosition(Vec3(PLAYER_STRAFE_LIMIT, player.getPosition().getY(), player.getPosition().getZ()));
		}
		else if (input.isKeyDown(Onyx::Key::R))
		{
			restart = true;
			window.close();
		}
		cam.update();

		if (input.isKeyTapped(Onyx::Key::Escape)) window.close();
		if (input.isKeyTapped(Onyx::Key::F12))
		{
			window.toggleFullscreen(1280, 720, IVec2(monitor.getDimensions().getX() / 2 - 1280 / 2, monitor.getDimensions().getY() / 2 - 720 / 2));
			if (window.isFullscreen())
			{
				scoreText.setScale(window.getBufferWidth() / 1280.0f * 0.5f);
				highScoreText.setScale(window.getBufferWidth() / 1280.0f * 0.3f);
			}
			else
			{
				scoreText.setScale(0.5f);
				highScoreText.setScale(0.3f);
			}
		}
		if (input.isKeyTapped(Onyx::Key::F1)) renderer.ToggleWireframe();

		if (!dead)
		{
			for (auto& spike : spikes)
			{
				spike.translate(Vec3(0.0f, 0.0f, spikeSpeed * dt));
				if (spike.getPosition().getZ() > 10.0f)
				{
					spike.setPosition(Vec3((rand() % 900) / 100.0f - 4.5f, 0.0f, -90.0f));
				}

				if (collision(player, spike))
				{
					dead = true;
					gameOverText.show();
					gameOverSubText.show();
				}
			}

			score += dt * 20.0f;
			if (score > highScore) highScore = score;
		}

		scoreText.setText(std::to_string((int)score));
		scoreText.setPosition(Vec2(window.getBufferWidth() / 2 - scoreText.getWidth() / 2, window.getBufferHeight() - 100.0f - scoreText.getHeight()));

		highScoreText.setText("High Score: " + std::to_string((int)highScore));
		highScoreText.setPosition(Vec2(20.0f, window.getBufferHeight() - 20.0f - highScoreText.getHeight()));

		gameOverText.setPosition(Vec2(window.getBufferWidth() / 2 - gameOverText.getWidth() / 2, window.getBufferHeight() / 2 - gameOverText.getHeight() / 2));
		gameOverSubText.setPosition(Vec2(window.getBufferWidth() / 2 - gameOverSubText.getWidth() / 2, gameOverText.getPosition().getY() - 50.0f));

		spikeSpeed += dt * 0.1f;
		playerSpeed += dt * 0.05f;

		window.startRender();
		renderer.render();
		window.endRender();
	}

	window.dispose();
	renderer.dispose();

	Onyx::FileUtils::Write("data.txt", std::to_string(highScore), false);

	Onyx::Terminate();
	if (restart) Run();

	Launcher::GameHub::Launch();
}

bool collision(const Onyx::ModelRenderable& player, const Onyx::ModelRenderable& spike)
{
	float dist = (player.getPosition() - spike.getPosition()).magnitude();
	return dist < (1.0f * player.getScale().getX() + 0.8f * spike.getScale().getX());
	//             ^0.5 for diglet
}
