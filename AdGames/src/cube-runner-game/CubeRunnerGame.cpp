#include "CubeRunnerGame.h"

#include <thread>

#include <Onyx/Core.h>
#include <Onyx/Window.h>
#include <Onyx/InputHandler.h>
#include <Onyx/Math.h>

using Onyx::Math::Vec2, Onyx::Math::Vec3, Onyx::Math::Vec4;

void CubeRunnerGame::Run()
{
	Onyx::Window window(
		Onyx::WindowProperties{
			.title = "Cube Runner Game",
			.width = 1280,
			.height = 720,
			.nSamplesMSAA = 4
		}
	);

	window.init();
	window.setBackgroundColor(Vec3::LightBlue());

	Onyx::InputHandler input;
	window.linkInputHandler(input);

	Onyx::Camera cam(Onyx::Projection::Perspective(60.0f, 1280, 720));
	window.linkCamera(cam);
	cam.translateFB(-10.0f);
	cam.translateUD(5.0f);

	Onyx::Lighting lighting(Vec3::White(), 0.3f, Vec3(0.2f, -1.0f, -0.3f));

	Onyx::Renderer renderer(cam, lighting);
	window.linkRenderer(renderer);

	Onyx::Renderable floor = Onyx::Renderable::ColoredQuad(10.0f, 100.0f, Vec4::LightGray());
	floor.rotate(Vec3(90.0f, 0.0f, 0.0f));
	floor.translate(Vec3(0.0f, 0.0f, -50.0f));

	Onyx::Renderable player = Onyx::Renderable::ColoredCube(1.0f, Vec4::Red());
	player.translate(Vec3(0.0f, 0.0f, -5.0f));

	renderer.add(floor);

	input.setCursorLock(true);

	const double CAM_SPEED = 6.0f;
	const double CAM_SENS = 50.0f;

	const bool LOCK_MOVEMENT = true;

	while (window.isOpen())
	{
		double dt = window.getDeltaTime();

		input.update();

		if (input.isKeyDown(Onyx::Key::W) && !LOCK_MOVEMENT) cam.translateFB( CAM_SPEED * dt);
		if (input.isKeyDown(Onyx::Key::A) && !LOCK_MOVEMENT) cam.translateLR(-CAM_SPEED * dt);
		if (input.isKeyDown(Onyx::Key::S) && !LOCK_MOVEMENT) cam.translateFB(-CAM_SPEED * dt);
		if (input.isKeyDown(Onyx::Key::D) && !LOCK_MOVEMENT) cam.translateLR( CAM_SPEED * dt);
		if (input.isKeyDown(Onyx::Key::Space) && !LOCK_MOVEMENT) cam.translateUD( CAM_SPEED * dt);
		if (input.isKeyDown(Onyx::Key::C) && !LOCK_MOVEMENT) cam.translateUD(-CAM_SPEED * dt);

		cam.rotate(input.getMouseDeltas().getX() / 200.0f * CAM_SENS, input.getMouseDeltas().getY() / 200.0f * CAM_SENS);
		cam.update();

		if (input.isKeyDown(Onyx::Key::Escape)) window.close();

		window.startRender();
		renderer.render();
		window.endRender();
	}

	window.dispose();
}
