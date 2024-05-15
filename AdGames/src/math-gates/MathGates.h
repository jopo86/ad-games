#pragma once

#include <Onyx/Renderer.h>

namespace MathGates
{
	void Run();

	class Gate
	{
	public:
		enum class Operator
		{
			Null,
			Add,
			Subtract,
			Multiply,
			Divide,
			Power
		};

		Gate();
		Gate(int val, Operator op, Onyx::Math::Vec3 color);

		void translate(const Onyx::Math::Vec3& translation);
		void addToRenderer(Onyx::Renderer& renderer);

	private:
		std::string m_text;
		int m_val;
		Operator m_op;
		Onyx::Math::Vec3 m_color;

		Onyx::TextRenderable3D m_textRenderable;
		Onyx::Renderable m_leftPost;
		Onyx::Renderable m_rightPost;
		Onyx::Renderable m_screen;

		static Onyx::Font sm_font;
	};

	class GunGrid
	{
	public:

	};
}
