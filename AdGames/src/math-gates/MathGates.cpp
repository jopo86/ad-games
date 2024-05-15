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
			.nSamplesMSAA = 8
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
	cam.translateFB(-2.0f);

	Onyx::Lighting lighting(Vec3::White(), 0.3f, Vec3(0.2f, -1.0f, -0.3f));
	Onyx::Fog fog(Vec3::LightBlue(), 40.0f, 90.0f);

	Onyx::Renderer renderer(cam, lighting, fog);
	window.linkRenderer(renderer);

	Gate::Operator ops[5] = {
		Gate::Operator::Add, Gate::Operator::Subtract, Gate::Operator::Multiply, Gate::Operator::Divide, Gate::Operator::Power
	};

	for (int i = 0; i < 10; i++)
	{
		Gate* gate = new Gate(pow(10, i), ops[i % 5], i % 2 == 0 ? Vec3::Green() : Vec3::Red());
		gate->translate(Vec3(i * 2.0f, 0.0f, 0.0f));
		gate->addToRenderer(renderer);
		onyx_add_malloc(gate, false);
	}

	const double CAM_SPEED = 6.0f;
	const double CAM_SENS = 50.0f;

	input.setCursorLock(true);

	while (window.isOpen())
	{
		double dt = window.getDeltaTime();

		input.update();

		if (input.isKeyTapped(Onyx::Key::Escape)) window.close();
		if (input.isKeyTapped(Onyx::Key::F1)) Onyx::Renderer::ToggleWireframe();
		if (input.isKeyTapped(Onyx::Key::F12)) window.toggleFullscreen();

		if (input.isKeyDown(Onyx::Key::W)) cam.translateFB(CAM_SPEED * dt);
		if (input.isKeyDown(Onyx::Key::A)) cam.translateLR(-CAM_SPEED * dt);
		if (input.isKeyDown(Onyx::Key::S)) cam.translateFB(-CAM_SPEED * dt);
		if (input.isKeyDown(Onyx::Key::D)) cam.translateLR(CAM_SPEED * dt);
		if (input.isKeyDown(Onyx::Key::Space)) cam.translateUD(CAM_SPEED * dt);
		if (input.isKeyDown(Onyx::Key::C)) cam.translateUD(-CAM_SPEED * dt);
		cam.rotate(input.getMouseDeltas().getX() / 200.0f * CAM_SENS, input.getMouseDeltas().getY() / 200.0f * CAM_SENS);

		cam.update();

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
	m_val = 0;
	m_op = Operator::Null;
}

MathGates::Gate::Gate(int val, Operator op, Vec3 color)
{
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
	m_textRenderable.scale(0.002f);

	if (m_text.length() > 2)
	{
		float h = sm_font.getStringDimensions("A").getY() * m_textRenderable.getScale().getY();
		m_textRenderable.scale(2.0f / m_text.length());
		m_textRenderable.translate(Vec3(0.0f, (h - sm_font.getStringDimensions("A").getY() * m_textRenderable.getScale().getY()) / 2.0f, 0.0f));
	}

	m_textRenderable.translate(Vec3(-m_textRenderable.dimensions().getX() / 2.0f, -0.22f, 0.051f));

	m_leftPost = Onyx::Renderable::ColoredRectPrism(0.2f, 1.5f, 0.2f, Vec4::LightGray());
	m_leftPost.translate(Vec3(-1.0f, 0.0f, 0.0f));

	m_rightPost = Onyx::Renderable::ColoredRectPrism(0.2f, 1.5f, 0.2f, Vec4::LightGray());
	m_rightPost.translate(Vec3(1.0f, 0.0f, 0.0f));

	m_screen = Onyx::Renderable::ColoredRectPrism(1.8f, 1.2f, 0.1f, Vec4(m_color, 0.8f));
	m_screen.translate(Vec3(0.0f, 0.14f, 0.0f));
}

void MathGates::Gate::translate(const Vec3& translation)
{
	m_textRenderable.translate(translation);
	m_leftPost.translate(translation);
	m_rightPost.translate(translation);
	m_screen.translate(translation);
}

void MathGates::Gate::addToRenderer(Onyx::Renderer& renderer)
{
	renderer.add(m_textRenderable);
	renderer.add(m_leftPost);
	renderer.add(m_rightPost);
	renderer.add(m_screen);
}
