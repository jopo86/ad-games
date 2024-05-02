#include "ArmyMathGame.h"

#include <thread>

#include <Onyx/Core.h>
#include <Onyx/Window.h>
#include <Onyx/InputHandler.h>
#include <Onyx/Math.h>

using Onyx::Math::Vec2, Onyx::Math::Vec3, Onyx::Math::Vec4;

void ArmyMathGame::Run()
{
	Onyx::Window window(
		Onyx::WindowProperties{
			.title = "Army Math Game",
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

	Onyx::Lighting lighting(Vec3::White(), 0.3f, Vec3(0.2, -1.0, -0.3));

	Onyx::Renderer renderer(cam, lighting);
	window.linkRenderer(renderer);

	while (window.isOpen())
	{
		input.update();

		if (input.isKeyDown(Onyx::Key::Escape)) window.close();

		window.startRender();
		renderer.render();
		window.endRender();
	}

	window.dispose();
}
