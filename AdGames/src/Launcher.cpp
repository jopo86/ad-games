#pragma warning(disable: 4244)

#include "Launcher.h"

#include "SpikeDodge.h"
#include "MathGates.h"
#include "ConnectFour.h"
#include "CannonGame.h"
#include "Launcher.h"

using namespace Onyx;
using namespace Onyx::Math;

const int WIDGET_WIDTH = 300, WIDGET_HEIGHT = 270, WIDGET_PADDING = 20;
const int WIDGETS_PER_ROW = 2, N_WIDGETS = 4, N_ROWS = Ceil((float)N_WIDGETS / WIDGETS_PER_ROW);

const int SCR_WIDTH = WIDGET_WIDTH * WIDGETS_PER_ROW + WIDGET_PADDING * (WIDGETS_PER_ROW + 1);
const int SCR_HEIGHT = WIDGET_HEIGHT * N_ROWS + WIDGET_PADDING * (N_ROWS + 1);

const int IMAGE_WIDTH = WIDGET_WIDTH - 2 * WIDGET_PADDING;
const int IMAGE_HEIGHT = 3 * IMAGE_WIDTH / 4;

void Launcher::GameHub::Launch()
{
	Onyx::Init();

	Monitor monitor = Monitor::GetPrimary();

	window = Window(
		WindowProperties{
			.title = "Game Hub",
			.width = SCR_WIDTH,
			.height = SCR_HEIGHT,
			.position = Vec2(monitor.getWidth() / 2 - SCR_WIDTH / 2, monitor.getHeight() / 2 - SCR_HEIGHT / 2),
			.nSamplesMSAA = 16,
			.backgroundColor = Vec3::DarkGray() * 0.6f,
		}
	);

	window.init();

	Onyx::WindowIcon icon = Onyx::WindowIcon::Load({
		Onyx::Resources("icons/icon-16x.png"),
		Onyx::Resources("icons/icon-24x.png"),
		Onyx::Resources("icons/icon-32x.png"),
		Onyx::Resources("icons/icon-48x.png"),
		Onyx::Resources("icons/icon-256x.png")
	});
	window.setIcon(icon);
	icon.dispose();

	arrowCursor = Cursor::Standard(CursorType::Arrow);
	handCursor = Cursor::Standard(CursorType::Hand);

	InputHandler input;
	window.linkInputHandler(input);

	Camera cam(Projection::Orthographic(SCR_WIDTH, SCR_HEIGHT));
	window.linkCamera(cam);

	renderer = Renderer(cam);
	window.linkRenderer(renderer);

	Font font = Font::Load(Resources("fonts/Roboto/Roboto-Regular.ttf"), 18);

	widgets.push_back(GameWidget("Spike Dodge", Resources("textures/spike_dodge_image.png"), 0, LaunchSpikeDodge, &font));
	widgets.push_back(GameWidget("Math Gates", Resources("textures/math_gates_image.png"), 1, LaunchMathGates, &font));
	widgets.push_back(GameWidget("Connect Four", Resources("textures/connect_four_image.png"), 2, LaunchConnectFour, &font));
	widgets.push_back(GameWidget("Cannon", Resources("textures/cannon_image.png"), 3, LaunchCannon, &font));

	for (GameWidget& widget : widgets) widget.addToRenderer(renderer);

	while (window.isOpen())
	{
		input.update();

		if (input.isKeyPressed(Key::Escape)) window.close();
		bool hovered = false;
		for (GameWidget& widget : widgets)
		{
			Vec2 pos = input.getMousePos();
			if (pos.getX() >= widget.getMid().getX() - WIDGET_WIDTH / 2 && pos.getX() <= widget.getMid().getX() + WIDGET_WIDTH / 2 &&
				pos.getY() >= widget.getMid().getY() - WIDGET_HEIGHT / 2 && pos.getY() <= widget.getMid().getY() + WIDGET_HEIGHT / 2)
			{
				widget.mouseEnter();
				hovered = true;
				if (input.isMouseButtonTapped(MouseButton::Left)) widget.mouseClick();
			}
			else widget.mouseExit();

		}
		if (hovered) window.setCursor(handCursor);
		else window.setCursor(arrowCursor);

		cam.update();

		window.startRender();
		renderer.render();
		window.endRender();
	}

	window.dispose();
	renderer.dispose();
	arrowCursor.dispose();
	handCursor.dispose();
	widgets.clear();

	Onyx::Terminate();
	exit(0);
}

void Launcher::GameHub::Dispose()
{
	window.close();
	window.dispose();
	renderer.dispose();
	arrowCursor.dispose();
	handCursor.dispose();
	widgets.clear();

	Onyx::Terminate();
}

void Launcher::GameHub::LaunchSpikeDodge()
{
	Dispose();
	SpikeDodge::Run();
}

void Launcher::GameHub::LaunchMathGates()
{
	Dispose();
	MathGates::Run();
}

void Launcher::GameHub::LaunchConnectFour()
{
	Dispose();
	ConnectFour::Run();
}

void Launcher::GameHub::LaunchCannon()
{
	Dispose();
	CannonGame::Run();
}

Onyx::Window Launcher::GameHub::window;
Onyx::Renderer Launcher::GameHub::renderer;
Onyx::Cursor Launcher::GameHub::arrowCursor;
Onyx::Cursor Launcher::GameHub::handCursor;
std::vector<Launcher::GameWidget> Launcher::GameHub::widgets;

Launcher::GameWidget::GameWidget()
{
	launchFunc = nullptr;
}

Launcher::GameWidget::GameWidget(const std::string& name, const std::string& imagePath, int index, void (*launchFunc)(), Font* font)
{
	this->name = name;
	this->imagePath = imagePath;
	this->launchFunc = launchFunc;

	background = Renderable::ColoredQuad(WIDGET_WIDTH, WIDGET_HEIGHT, Vec4::LightGray() * 0.5f);
	image = Renderable::TexturedQuad(IMAGE_WIDTH, IMAGE_HEIGHT, Texture::Load(imagePath));
	text = TextRenderable3D(name, *font, Vec4::White());

	Vec2 mid(WIDGET_PADDING + WIDGET_WIDTH / 2, SCR_HEIGHT - WIDGET_PADDING - WIDGET_HEIGHT / 2);
	if (index % WIDGETS_PER_ROW != 0)
		for (int i = 0; i < index % WIDGETS_PER_ROW; i++) mid.setX(mid.getX() + WIDGET_PADDING + WIDGET_WIDTH);

	if (index >= WIDGETS_PER_ROW)
		for (int i = 0; i < index / WIDGETS_PER_ROW; i++) mid.setY(mid.getY() - WIDGET_PADDING - WIDGET_HEIGHT);

	this->mid = mid;

	background.setPosition(Vec3(mid, -1.0f));
	image.setPosition(Vec3(mid.getX(), mid.getY() + (WIDGET_HEIGHT - IMAGE_HEIGHT) / 2 - WIDGET_PADDING, -0.9f));
	text.setPosition(Vec3(mid.getX() - text.getWidth() / 2, mid.getY() - WIDGET_HEIGHT / 2 + WIDGET_PADDING, -0.8f));
}

void Launcher::GameWidget::addToRenderer(Renderer& renderer)
{
	renderer.add(background);
	renderer.add(image);
	renderer.add(text);
}

void Launcher::GameWidget::mouseEnter()
{
	Shader* shader = background.getShader();
	shader->use();
	shader->setVec4("u_color", Vec4::LightGray() * 0.6f);
}

void Launcher::GameWidget::mouseExit()
{
	Shader* shader = background.getShader();
	shader->use();
	shader->setVec4("u_color", Vec4::LightGray() * 0.5f);
}

void Launcher::GameWidget::mouseClick()
{
	launchFunc();
}

const Vec2& Launcher::GameWidget::getMid() const
{
	return mid;
}
