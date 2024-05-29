#pragma once

#include <iostream>
#include <vector>

#include <Onyx/Renderable.h>
#include <Onyx/TextRenderable.h>
#include <Onyx/Camera.h>
#include <Onyx/Math.h>

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
		void mouseClick();

		const Onyx::Math::Vec2& getMid() const;

	private:
		std::string name, imagePath;
		void (*launchFunc)();
		Onyx::Math::Vec2 mid;

		Onyx::Renderable background, image;
		Onyx::TextRenderable3D text;
	};

	class GameHub
	{
	public:

		static void Launch();

	private:
		static void Dispose();

		static void LaunchSpikeDodge();
		static void LaunchMathGates();
		static void LaunchConnectFour();
		static void LaunchCannon();

		static Onyx::Window window;
		static Onyx::Renderer renderer;
		static Onyx::Cursor arrowCursor, handCursor;
		static std::vector<GameWidget> widgets;
	};
}
