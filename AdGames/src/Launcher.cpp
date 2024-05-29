#include "Launcher.h"

using namespace Onyx;
using namespace Onyx::Math;

const int WIDGET_WIDTH = 200, WIDGET_HEIGHT = 200, WIDGET_PADDING = 20;
const int WIDGETS_PER_ROW = 2, WIDGETS_PER_COL = 2;
const int SCR_WIDTH = WIDGET_WIDTH * WIDGETS_PER_ROW + WIDGET_PADDING * (WIDGETS_PER_ROW + 1);
const int SCR_HEIGHT = WIDGET_HEIGHT * WIDGETS_PER_COL + WIDGET_PADDING * (WIDGETS_PER_COL + 1);
const int IMAGE_WIDTH = WIDGET_WIDTH * 0.9f;
const int IMAGE_HEIGHT = 9 * IMAGE_WIDTH / 16;
// 16/9 = w/h
// 16h = 9w
// h = 9w/16

Launcher::GameHub::GameHub()
{

}

void Launcher::GameHub::launch()
{
    Window window(
        WindowProperties{
            .title = "Onyx Window",
            .width = SCR_WIDTH,
            .height = SCR_HEIGHT,
            .nSamplesMSAA = 16,
            .backgroundColor = Vec3::DarkGray()
        }
    );

    window.init();

    InputHandler input;
    window.linkInputHandler(input);

    Camera cam(Projection::Orthographic(SCR_WIDTH, SCR_HEIGHT));
    window.linkCamera(cam);

    Renderer renderer(cam);
    window.linkRenderer(renderer);

    Font font = Font::Load(Resources("fonts/Poppins/Poppins-Bold.ttf"), 72);

    widgets.push_back(GameWidget("Spike Dodge", Resources("textures/onyx.png"), 0, nullptr, &font));
    widgets.push_back(GameWidget("Math Gates", Resources("textures/onyx.png"), 1, nullptr, &font));
    widgets.push_back(GameWidget("Connect Four", Resources("textures/onyx.png"), 2, nullptr, &font));
    widgets.push_back(GameWidget("Cannon", Resources("textures/onyx.png"), 3, nullptr, &font));

    while (window.isOpen())
    {
        input.update();

        cam.update();


        window.startRender();
        renderer.render();
        window.endRender();
    }

    window.dispose();
    renderer.dispose();
    // Dispose any other disposables created (not including renderables that were added to the renderer, these are disposed automatically)

    Terminate();
}

Launcher::GameWidget::GameWidget()
{
	launchFunc = nullptr;
}

Launcher::GameWidget::GameWidget(const std::string& name, const std::string& imagePath, int index, void (*launchFunc)(), Font* font)
{
	this->name = name;
	this->imagePath = imagePath;
	this->launchFunc = launchFunc;

	background = Renderable::ColoredQuad(WIDGET_WIDTH, WIDGET_HEIGHT, Vec4::LightGray());
	image = Renderable::TexturedQuad(IMAGE_WIDTH, IMAGE_HEIGHT, Texture::Load(imagePath));
	text = TextRenderable3D(name, *font, Vec4::White());

	Vec2 mid(WIDGET_PADDING + WIDGET_WIDTH / 2, SCR_HEIGHT - WIDGET_PADDING - WIDGET_WIDTH / 2);
	if (index % WIDGETS_PER_ROW != 0)
		for (int i = 0; i < index % WIDGETS_PER_ROW; i++) mid.setX(mid.getX() + WIDGET_PADDING + WIDGET_WIDTH);

	if (index >= WIDGETS_PER_ROW)
		for (int i = 0; i < index + 1 / WIDGETS_PER_ROW - 1; i++) mid.setY(mid.getY() - WIDGET_PADDING - WIDGET_HEIGHT);

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
