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

		bool collision(const Onyx::Math::Vec3& camPos);

		void changeScore(float* score);

		int getVal() const;
		Operator getOp() const;
		const Onyx::Math::Vec3& getColor() const;

		void setVal(int val);
		void setOp(Operator op);
		void setColor(const Onyx::Math::Vec3& color);

		void refresh();

	private:
		std::string m_text;
		int m_val;
		Operator m_op;
		Onyx::Math::Vec3 m_color;

		Onyx::TextRenderable3D m_textRenderable;
		Onyx::Renderable m_leftPost;
		Onyx::Renderable m_rightPost;
		Onyx::Renderable m_screen;

		bool m_collided;

		static bool sm_fontCreated;
		static Onyx::Font sm_font;

	};
}
