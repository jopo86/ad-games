#pragma warning(disable: 4244)

#include "MathGates.h"

#include <Onyx/Core.h>
#include <Onyx/Window.h>
#include <Onyx/InputHandler.h>
#include <Onyx/Math.h>
#include <Onyx/FileUtils.h>
#include <Onyx/Monitor.h>

using Onyx::Math::Vec2, Onyx::Math::Vec3, Onyx::Math::Vec4, Onyx::Math::IVec2;

void onyx_add_malloc(void*, bool);

void MathGates::Run()
{
	Onyx::Init();

	Onyx::Window window(
		Onyx::WindowProperties{
			.title = "Math Gates",
			.width = 1280,
			.height = 720,
			.position = IVec2(2560 / 2 - 1280 / 2, 1440 / 2 - 720 / 2),
			.nSamplesMSAA = 16
		}
	);

	window.init();
	window.setBackgroundColor(Vec3::LightBlue());
	Onyx::Monitor monitor = Onyx::Monitor::GetPrimary();
	window.setPosition(IVec2(monitor.getDimensions().getX() / 2 - window.getWidth() / 2, monitor.getDimensions().getY() / 2 - window.getHeight() / 2));

	Onyx::InputHandler input;
	window.linkInputHandler(input);

	Onyx::Camera cam(Onyx::Projection::Perspective(60.0f, 1280, 720));
	window.linkCamera(cam);
	cam.translate(Vec3(1.0f, 0.2f, -2.0f));

	Onyx::Lighting lighting(Vec3::White(), 0.3f, Vec3(0.2f, -1.0f, -0.3f));
	Onyx::Fog fog(Vec3::LightBlue(), 20.0f, 40.0f);

	Onyx::Renderer renderer(cam, lighting, fog);
	window.linkRenderer(renderer);

	Gate::Operator ops[5] = {
		Gate::Operator::Add, Gate::Operator::Subtract, Gate::Operator::Multiply, Gate::Operator::Divide, Gate::Operator::Power
	};

	Onyx::Renderable floor = Onyx::Renderable::ColoredRectPrism(5.0f, 0.2f, 200.0f, Vec4::White());
	floor.translate(Vec3(1.1f, -0.9f, -90.0f));
	renderer.add(floor);

	Onyx::Font poppins = Onyx::Font::Load(Onyx::Resources("fonts/Poppins/Poppins-Regular.ttf"), 32);

	Onyx::TextRenderable scoreText = Onyx::TextRenderable("Score: 0", poppins, Vec4::White());

	srand(time(nullptr));

	std::vector<Gate*> pGates;
	for (int i = 20; i >= 0; i--)
	{
		for (int j = 0; j < 2; j++)
		{
			Gate::Operator op = ops[rand() % 5];
			int num;
			if (op != Gate::Operator::Power) num = rand() % 100;
			else num = rand() % 5;
			Vec3 color;
			if (op == Gate::Operator::Add || op == Gate::Operator::Multiply || op == Gate::Operator::Power) color = Vec3::Green();
			else color = Vec3::Red();
			Gate* gate = new Gate(num, op, color);
			onyx_add_malloc(gate, false);
			gate->translate(Vec3(j * 2.0f, 0.0f, -i * 7.5f));
			gate->addToRenderer(renderer);
			pGates.push_back(gate);
		}
	}

	renderer.add(scoreText);

	const double CAM_SPEED = 6.0f;
	const double PLAYER_SPEED = 1.0f;
	const double CAM_SENS = 50.0f;

	const double PLAYER_STRAFE_LIMIT = 3.0f;

	const bool ALLOW_FULL_MOVEMENT = false;

	float score = 0.0f;

	input.setCursorLock(true);

	while (window.isOpen())
	{
		double dt = window.getDeltaTime();

		input.update();

		if (input.isKeyTapped(Onyx::Key::Escape)) window.close();
		if (input.isKeyTapped(Onyx::Key::F1)) Onyx::Renderer::ToggleWireframe();
		if (input.isKeyTapped(Onyx::Key::F12)) window.toggleFullscreen();

		if (ALLOW_FULL_MOVEMENT)
		{
			if (input.isKeyDown(Onyx::Key::W)) cam.translateFB(CAM_SPEED * dt * PLAYER_SPEED);
			if (input.isKeyDown(Onyx::Key::A)) cam.translateLR(-CAM_SPEED * dt * PLAYER_SPEED);
			if (input.isKeyDown(Onyx::Key::S)) cam.translateFB(-CAM_SPEED * dt * PLAYER_SPEED);
			if (input.isKeyDown(Onyx::Key::D)) cam.translateLR(CAM_SPEED * dt * PLAYER_SPEED);
			if (input.isKeyDown(Onyx::Key::Space)) cam.translateUD(CAM_SPEED * dt * PLAYER_SPEED);
			if (input.isKeyDown(Onyx::Key::C)) cam.translateUD(-CAM_SPEED * dt * PLAYER_SPEED);

			cam.rotate(input.getMouseDeltas().getX() / 200.0f * CAM_SENS, input.getMouseDeltas().getY() / 200.0f * CAM_SENS);
		}
		else
		{
			cam.translateFB(CAM_SPEED * dt * PLAYER_SPEED);
			if (input.isKeyDown(Onyx::Key::A)) cam.translateLR(-CAM_SPEED * dt);
			if (input.isKeyDown(Onyx::Key::D)) cam.translateLR(CAM_SPEED * dt);

			if (cam.getPosition().getX() < 0.2f - PLAYER_STRAFE_LIMIT) cam.setPosition(Vec3(0.2f - PLAYER_STRAFE_LIMIT, cam.getPosition().getY(), cam.getPosition().getZ()));
			else if (cam.getPosition().getX() > 0.2f + PLAYER_STRAFE_LIMIT) cam.setPosition(Vec3(0.2f + PLAYER_STRAFE_LIMIT, cam.getPosition().getY(), cam.getPosition().getZ()));
		}

		cam.update();

		for (Gate* gate : pGates)
		{
			if (gate->collision(cam.getPosition()))
			{
				gate->changeScore(&score);
			}
		}

		scoreText.setText("Score: " + std::to_string((int)score));
		float w = window.getBufferWidth(), h = window.getBufferHeight();
		float tw = scoreText.dimensions().getX(), th = scoreText.dimensions().getY();
		scoreText.setPosition(Vec2((w - tw) / 2.0f, h - th - 50.0f));

		window.startRender();
		renderer.render();
		window.endRender();
	}

	renderer.dispose();
	window.dispose();

	Onyx::Terminate();
}

Onyx::Font MathGates::Gate::sm_font;

MathGates::Gate::Gate()
{
	m_collided = false;
	m_val = 0;
	m_op = Operator::Null;
}

MathGates::Gate::Gate(int val, Operator op, Vec3 color)
{
	m_collided = false;
	m_val = val;
	m_op = op;
	m_color = color;

	m_text = "";
	switch (op)
	{
		case Operator::Null: break;
		case Operator::Add: m_text += "+"; break;
		case Operator::Subtract: m_text += "-"; break;
		case Operator::Multiply: m_text += "x"; break;
		case Operator::Divide: m_text += "/"; break;
		case Operator::Power: m_text += "^"; break;
	}

	m_text += std::to_string(val);

#ifndef GATE_FONT_CREATED
#define GATE_FONT_CREATED
	sm_font = Onyx::Font::Load(Onyx::Resources("fonts/Poppins/Poppins-Bold.ttf"), 512);
#endif

	m_textRenderable = Onyx::TextRenderable3D(m_text, sm_font, Vec4::White());
	m_textRenderable.scale(0.0019f);

	if (m_text.length() > 2)
	{
		float h = sm_font.getStringDimensions("A").getY() * m_textRenderable.getScale().getY();
		m_textRenderable.scale(2.0f / m_text.length());
		m_textRenderable.translate(Vec3(0.0f, (h - sm_font.getStringDimensions("A").getY() * m_textRenderable.getScale().getY()) / 2.0f, 0.0f));
	}

	m_textRenderable.translate(Vec3(-m_textRenderable.dimensions().getX() / 2.0f, -0.22f, 0.055f));

	m_leftPost = Onyx::Renderable::ColoredRectPrism(0.2f, 1.5f, 0.2f, Vec4::LightGray());
	m_leftPost.translate(Vec3(-1.0f, 0.0f, 0.0f));

	m_rightPost = Onyx::Renderable::ColoredRectPrism(0.2f, 1.5f, 0.2f, Vec4::LightGray());
	m_rightPost.translate(Vec3(1.0f, 0.0f, 0.0f));

	m_screen = Onyx::Renderable::ColoredRectPrism(1.8f, 1.2f, 0.1f, Vec4(m_color, 0.5f));
	m_screen.translate(Vec3(0.0f, 0.14f, 0.0f));
}

void MathGates::Gate::translate(const Vec3& translation)
{
	m_leftPost.translate(translation);
	m_rightPost.translate(translation);
	m_screen.translate(translation);
	m_textRenderable.translate(translation);
}

void MathGates::Gate::addToRenderer(Onyx::Renderer& renderer)
{
	renderer.add(m_textRenderable);
	renderer.add(m_leftPost);
	renderer.add(m_rightPost);
	renderer.add(m_screen);
}

bool MathGates::Gate::collision(const Onyx::Math::Vec3& camPos)
{
	if (m_collided) return false;

	Vec3 pos = m_screen.getPosition();

	if (camPos.getX() > pos.getX() - 0.9f && camPos.getX() < pos.getX() + 0.9f
		&& camPos.getZ() > pos.getZ() - 0.05f && camPos.getZ() < pos.getZ() + 0.05f)
	{
		m_collided = true;
		return true;
	}

	return false;
}

void MathGates::Gate::changeScore(float* score)
{
	switch(m_op)
	{
		case Operator::Add: *score += m_val; break;
		case Operator::Subtract: *score -= m_val; break;
		case Operator::Multiply: *score *= m_val; break;
		case Operator::Divide: *score /= m_val; break;
		case Operator::Power: *score = pow(*score, m_val); break;
	}
}

int MathGates::Gate::getVal() const
{
	return m_val;
}

MathGates::Gate::Operator MathGates::Gate::getOp() const
{
	return m_op;
}

const Onyx::Math::Vec3& MathGates::Gate::getColor() const
{
	return m_color;
}

void MathGates::Gate::setVal(int val)
{
	m_val = val;
}

void MathGates::Gate::setOp(Operator op)
{
	m_op = op;
}

void MathGates::Gate::setColor(const Vec3& color)
{
	m_color = color;
}

void MathGates::Gate::refresh()
{
	m_textRenderable.dispose();
	m_leftPost.dispose();
	m_rightPost.dispose();
	m_screen.dispose();

	m_text = "";
	switch (m_op)
	{
		case Operator::Null: break;
		case Operator::Add: m_text += "+"; break;
		case Operator::Subtract: m_text += "-"; break;
		case Operator::Multiply: m_text += "x"; break;
		case Operator::Divide: m_text += "/"; break;
		case Operator::Power: m_text += "^"; break;
	}

	m_text += std::to_string(m_val);

	if (!sm_fontCreated)
	{
		sm_fontCreated = true;
		sm_font = Onyx::Font::Load(Onyx::Resources("fonts/Poppins/Poppins-Bold.ttf"), 512);
	}

	m_textRenderable = Onyx::TextRenderable3D(m_text, sm_font, Vec4::White());
	m_textRenderable.scale(0.0019f);

	if (m_text.length() > 2)
	{
		float h = sm_font.getStringDimensions("A").getY() * m_textRenderable.getScale().getY();
		m_textRenderable.scale(2.0f / m_text.length());
		m_textRenderable.translate(Vec3(0.0f, (h - sm_font.getStringDimensions("A").getY() * m_textRenderable.getScale().getY()) / 2.0f, 0.0f));
	}

	m_textRenderable.translate(Vec3(-m_textRenderable.dimensions().getX() / 2.0f, -0.22f, 0.055f));

	m_leftPost = Onyx::Renderable::ColoredRectPrism(0.2f, 1.5f, 0.2f, Vec4::LightGray());
	m_leftPost.translate(Vec3(-1.0f, 0.0f, 0.0f));

	m_rightPost = Onyx::Renderable::ColoredRectPrism(0.2f, 1.5f, 0.2f, Vec4::LightGray());
	m_rightPost.translate(Vec3(1.0f, 0.0f, 0.0f));

	m_screen = Onyx::Renderable::ColoredRectPrism(1.8f, 1.2f, 0.1f, Vec4(m_color, 0.5f));
	m_screen.translate(Vec3(0.0f, 0.14f, 0.0f));
}
