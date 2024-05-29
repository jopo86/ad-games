#pragma once

#include <iostream>
#include <vector>

#include <Onyx/Renderable.h>
#include <Onyx/TextRenderable.h>
#include <Onyx/Camera.h>

namespace Launcher
{
	class GameWidget
	{
	public:
		GameWidget();
		GameWidget(const std::string& name, const std::string& imagePath, int index, void(*launchFunc)(), Onyx::Font* font);

		void addToRenderer(Onyx::Renderer& renderer);
		void mouseEnter();
		void mouseExit();

	private:
		std::string name, imagePath;
		void (*launchFunc)();

		Onyx::Renderable background, image;
		Onyx::TextRenderable3D text;
	};

	class GameHub
	{
	public:

		GameHub();

		void launch();

	private:
		void launchSpikeDodge();
		void launchMathGates();
		void launchConnectFour();
		void launchCannon();

		std::vector<GameWidget> widgets;
	};
}
