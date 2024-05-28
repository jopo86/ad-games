#pragma once

#include <initializer_list>
#include <Onyx/Core.h>
#include <Onyx/Renderer.h>
#include <Onyx/Math.h>

using Onyx::Renderable, Onyx::Camera, Onyx::Math::Vec2, Onyx::Math::Vec3, Onyx::TextRenderable3D, Onyx::Font;

namespace CannonGame
{
	void Run();

	class CannonBall
	{
	public:
		CannonBall();
		CannonBall(Vec2 vel, Vec2 pos, float rot, float rotStep, Renderable* renderable);

		void update(float dt);
		void render(const Camera& cam);

		Vec2 vel, pos;
		float rot, rotStep;

	private:
		Renderable* renderable;
	};

	class Boulder
	{
	public:
		Boulder();
		Boulder(Vec2 vel, Vec2 pos, float rot, float rotStep, float radius, int nSegments, int health, Vec3 color, Font* font);

		void update(float dt);
		void render(const Camera& cam);
		void damage(int amount);
		bool collision(const CannonBall& ball);
		void dispose();

		Vec2 vel, pos;
		float rot, rotStep, radius;
		int nSegments, health;
		Vec3 color;
		Font* font;
		bool destroyed;

	private:
		Renderable outer, inner;
		TextRenderable3D text;
	};
};
