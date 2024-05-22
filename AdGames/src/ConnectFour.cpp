#pragma warning(disable: 4244)

#include "ConnectFour.h"

#include <Onyx/Core.h>
#include <Onyx/Window.h>
#include <Onyx/InputHandler.h>
#include <Onyx/Math.h>

void onyx_add_malloc(void*, bool);

using namespace Onyx;
using namespace Onyx::Math;

enum class Space
{
	Empty,
	Red,
	Yellow
};

enum class Player
{
	Red,
	Yellow
};

const int SCR_SIZE = 1000;

const int BOARD_WIDTH = 7, BOARD_HEIGHT = 6;
Space board[BOARD_WIDTH][BOARD_HEIGHT];

const float DISC_RADIUS = SCR_SIZE / 20;
const float DISC_FALL_SPEED = 30.0f;

Player curPlayer = Player::Red;

Vec2 getSpacePosition(int i, int j)
{
	return Vec2(SCR_SIZE / BOARD_WIDTH * i + SCR_SIZE / BOARD_WIDTH / 2, (SCR_SIZE - 150) / BOARD_HEIGHT * j + SCR_SIZE / BOARD_HEIGHT / 2);
}

void render(Camera& cam, Renderable& emptyDisc, Renderable& redDiscOuter, Renderable& redDiscInner, Renderable& yellowDiscOuter, Renderable& yellowDiscInner, int hoveredColumn);
bool isMouseOnSpace(Vec2 mousePos, int* i, int* j);
bool isMouseOnColumn(Vec2 mousePos, int* i);
bool checkWinner(Player* player);

void ConnectFour::Run()
{
	for (int i = 0; i < BOARD_WIDTH; i++)
	{
		for (int j = 0; j < BOARD_HEIGHT; j++)
		{
			board[i][j] = Space::Empty;
		}
	}

	Monitor monitor = Monitor::GetPrimary();

	Window window(
		WindowProperties{
			.title = "Connect Four",
			.width = SCR_SIZE,
			.height = SCR_SIZE,
			.position = IVec2(monitor.getDimensions().getX() / 2 - SCR_SIZE / 2, monitor.getDimensions().getY() / 2 - SCR_SIZE / 2),
			.resizable = false,
			.decorated = true,
			.nSamplesMSAA = 16,
			.backgroundColor = Vec3::NavyBlue() * 0.8f
		}
	);

	window.init();
	
	InputHandler input;
	window.linkInputHandler(input);

	Camera cam(Projection::Orthographic(SCR_SIZE, SCR_SIZE));
	window.linkCamera(cam);

	Renderer renderer(cam);
	window.linkRenderer(renderer);

	Renderable emptyDisc = Renderable::ColoredCircle(DISC_RADIUS, 40, Vec4::Black(0.4f));
	Renderable redDiscOuter = Renderable::ColoredCircle(DISC_RADIUS, 40, Vec4::Red());
	Renderable redDiscInner = Renderable::ColoredCircle(DISC_RADIUS * 0.7f, 40, Vec4::Red() * 0.7f);
	Renderable yellowDiscOuter = Renderable::ColoredCircle(DISC_RADIUS, 40, Vec4::Yellow());
	Renderable yellowDiscInner = Renderable::ColoredCircle(DISC_RADIUS * 0.7f, 40, Vec4::Yellow() * 0.7f);

	Cursor arrowCursor = Cursor::Standard(CursorType::Arrow);
	Cursor handCursor = Cursor::Standard(CursorType::Hand);
	window.setCursor(arrowCursor);

	Font font = Font::Load(Resources("fonts/Poppins/Poppins-Bold.ttf"), 72);

	Vec2 discFallingPos;

	bool over = false;
	bool discFalling = false;

	int queuedI = -1, queuedJ = -1;

	while (window.isOpen())
	{
		input.update();
		
		if (input.isKeyTapped(Key::Escape)) window.close();
		if (input.isKeyTapped(Key::F1)) Renderer::ToggleWireframe();

		cam.update();

		int i = -1;
		if (!discFalling)
		{

			if (!over)
			{
				bool mouseOnColumn = isMouseOnColumn(input.getMousePos(), &i);

				if (mouseOnColumn)
				{
					window.setCursor(handCursor);
				}
				else window.setCursor(arrowCursor);
				if (input.isMouseButtonTapped(MouseButton::Left) && mouseOnColumn)
				{
					for (int j = 0; j < BOARD_HEIGHT; j++)
					{
						if (board[i][j] == Space::Empty)
						{
							queuedI = i;
							queuedJ = j;
							discFalling = true;
							discFallingPos = getSpacePosition(i, BOARD_HEIGHT);
							break;
						}
					}
				}
			}

			if (!over)
			{
				Player winner;
				if (checkWinner(&winner))
				{
					TextRenderable* text = new TextRenderable(winner == Player::Red ? "Red Wins!" : "Yellow Wins!", font, winner == Player::Red ? Vec4::Red() : Vec4::Yellow());
					text->setPosition(Vec2(SCR_SIZE / 2 - text->getWidth() / 2, SCR_SIZE - 50.0f - text->getHeight()));
					onyx_add_malloc(text, false);
					renderer.add(*text);
					over = true;
					window.setCursor(arrowCursor);
				}
				else
				{
					bool draw = true;
					for (int i = 0; i < BOARD_WIDTH; i++)
					{
						for (int j = 0; j < BOARD_HEIGHT; j++)
						{
							if (board[i][j] == Space::Empty)
							{
								draw = false;
								break;
							}
						}
					}

					if (draw)
					{
						TextRenderable* text = new TextRenderable("Draw!", font, Vec4::White());
						text->setPosition(Vec2(SCR_SIZE / 2 - text->getWidth() / 2, SCR_SIZE - 50.0f - text->getHeight()));
						onyx_add_malloc(text, false);
						renderer.add(*text);
						over = true;
						window.setCursor(arrowCursor);
					}
				}
			}
		}

		window.startRender();
		renderer.render();
		render(cam, emptyDisc, redDiscOuter, redDiscInner, yellowDiscOuter, yellowDiscInner, i);
		if (discFalling && curPlayer == Player::Red)
		{
			redDiscOuter.setPosition(Vec3(discFallingPos, 0));
			redDiscInner.setPosition(Vec3(discFallingPos, 1));
			redDiscOuter.render(cam.getViewMatrix(), cam.getProjectionMatrix(), cam.getPosition());
			redDiscInner.render(cam.getViewMatrix(), cam.getProjectionMatrix(), cam.getPosition());
			discFallingPos.setY(discFallingPos.getY() - DISC_FALL_SPEED);
			if (discFallingPos.getY() < getSpacePosition(queuedI, queuedJ).getY())
			{
				discFalling = false;
				discFallingPos = getSpacePosition(i, BOARD_HEIGHT);
				board[queuedI][queuedJ] = curPlayer == Player::Red ? Space::Red : Space::Yellow;
				curPlayer = curPlayer == Player::Red ? Player::Yellow : Player::Red;
			}
		}
		else if (discFalling && curPlayer == Player::Yellow)
		{
			yellowDiscOuter.setPosition(Vec3(discFallingPos, 0));
			yellowDiscInner.setPosition(Vec3(discFallingPos, 1));
			yellowDiscOuter.render(cam.getViewMatrix(), cam.getProjectionMatrix(), cam.getPosition());
			yellowDiscInner.render(cam.getViewMatrix(), cam.getProjectionMatrix(), cam.getPosition());
			discFallingPos.setY(discFallingPos.getY() - DISC_FALL_SPEED);
			if (discFallingPos.getY() < getSpacePosition(queuedI, queuedJ).getY())
			{
				discFalling = false;
				discFallingPos = getSpacePosition(i, BOARD_HEIGHT);
				board[queuedI][queuedJ] = curPlayer == Player::Red ? Space::Red : Space::Yellow;
				curPlayer = curPlayer == Player::Red ? Player::Yellow : Player::Red;
			}
		}
		window.endRender();
	}

	window.dispose();
	renderer.dispose();
	emptyDisc.dispose();
	redDiscOuter.dispose();
	redDiscInner.dispose();
	yellowDiscOuter.dispose();
	yellowDiscInner.dispose();
	arrowCursor.dispose();
	handCursor.dispose();
}

void render(Camera& cam, Renderable& emptyDisc, Renderable& redDiscOuter, Renderable& redDiscInner, Renderable& yellowDiscOuter, Renderable& yellowDiscInner, int hoveredColumn)
{
	for (int i = 0; i < BOARD_WIDTH; i++)
	{
		for (int j = 0; j < BOARD_HEIGHT; j++)
		{
			emptyDisc.setPosition(Vec3(getSpacePosition(i, j), -1));
			emptyDisc.render(cam.getViewMatrix(), cam.getProjectionMatrix(), cam.getPosition());

			if (board[i][j] == Space::Red)
			{
				redDiscOuter.setPosition(Vec3(getSpacePosition(i, j), 0));
				redDiscInner.setPosition(Vec3(getSpacePosition(i, j), 1));
				redDiscOuter.render(cam.getViewMatrix(), cam.getProjectionMatrix(), cam.getPosition());
				redDiscInner.render(cam.getViewMatrix(), cam.getProjectionMatrix(), cam.getPosition());
			}
			else if (board[i][j] == Space::Yellow)
			{
				yellowDiscOuter.setPosition(Vec3(getSpacePosition(i, j), 0));
				yellowDiscInner.setPosition(Vec3(getSpacePosition(i, j), 1));
				yellowDiscOuter.render(cam.getViewMatrix(), cam.getProjectionMatrix(), cam.getPosition());
				yellowDiscInner.render(cam.getViewMatrix(), cam.getProjectionMatrix(), cam.getPosition());
			}
			else if (i == hoveredColumn)
			{
				if (curPlayer == Player::Red)
				{
					redDiscOuter.getShader()->use();
					redDiscOuter.getShader()->setVec4("u_color", Vec4::Red() * 0.9f);
					redDiscInner.getShader()->use();
					redDiscInner.getShader()->setVec4("u_color", Vec4::Red() * 0.7f * 0.9f);
					redDiscOuter.setPosition(Vec3(getSpacePosition(i, BOARD_HEIGHT), 0));
					redDiscInner.setPosition(Vec3(getSpacePosition(i, BOARD_HEIGHT), 1));
					redDiscOuter.render(cam.getViewMatrix(), cam.getProjectionMatrix(), cam.getPosition());
					redDiscInner.render(cam.getViewMatrix(), cam.getProjectionMatrix(), cam.getPosition());
					redDiscOuter.getShader()->use();
					redDiscOuter.getShader()->setVec4("u_color", Vec4::Red());
					redDiscInner.getShader()->use();
					redDiscInner.getShader()->setVec4("u_color", Vec4::Red() * 0.7f);
				}
				else
				{
					yellowDiscOuter.getShader()->use();
					yellowDiscOuter.getShader()->setVec4("u_color", Vec4::Yellow() * 0.9f);
					yellowDiscInner.getShader()->use();
					yellowDiscInner.getShader()->setVec4("u_color", Vec4::Yellow() * 0.7f * 0.9f);
					yellowDiscOuter.setPosition(Vec3(getSpacePosition(i, BOARD_HEIGHT), 0));
					yellowDiscInner.setPosition(Vec3(getSpacePosition(i, BOARD_HEIGHT), 1));
					yellowDiscOuter.render(cam.getViewMatrix(), cam.getProjectionMatrix(), cam.getPosition());
					yellowDiscInner.render(cam.getViewMatrix(), cam.getProjectionMatrix(), cam.getPosition());
					yellowDiscOuter.getShader()->use();
					yellowDiscOuter.getShader()->setVec4("u_color", Vec4::Yellow());
					yellowDiscInner.getShader()->use();
					yellowDiscInner.getShader()->setVec4("u_color", Vec4::Yellow() * 0.7f);
				}
			}
		}
	}
}

bool isMouseOnSpace(Vec2 mousePos, int* i, int* j)
{
	for (int x = 0; x < BOARD_WIDTH; x++)
	{
		for (int y = 0; y < BOARD_HEIGHT; y++)
		{
			Vec2 spacePos = getSpacePosition(x, y);
			if ((mousePos - spacePos).magnitude() < DISC_RADIUS)
			{
				*i = x;
				*j = y;
				return true;
			}
		}
	}

	*i = -1;
	*j = -1;
	return false;
}

bool isMouseOnColumn(Vec2 mousePos, int* i)
{
	for (int x = 0; x < BOARD_WIDTH; x++)
	{
		Vec2 spacePos = getSpacePosition(x, 0);
		if (abs(mousePos.getX() - spacePos.getX()) < SCR_SIZE / BOARD_WIDTH / 2)
		{
			*i = x;
			return true;
		}
	}

	*i = -1;
	return false;
}

bool checkWinner(Player* player)
{
	for (int i = 0; i < BOARD_WIDTH; i++)
	{
		for (int j = 0; j < BOARD_HEIGHT; j++)
		{
			if (board[i][j] == Space::Empty) continue;

			if (i + 3 < BOARD_WIDTH)
			{
				if (board[i][j] == board[i + 1][j] && board[i][j] == board[i + 2][j] && board[i][j] == board[i + 3][j])
				{
					*player = board[i][j] == Space::Red ? Player::Red : Player::Yellow;
					return true;
				}
			}

			if (j + 3 < BOARD_HEIGHT)
			{
				if (board[i][j] == board[i][j + 1] && board[i][j] == board[i][j + 2] && board[i][j] == board[i][j + 3])
				{
					*player = board[i][j] == Space::Red ? Player::Red : Player::Yellow;
					return true;
				}
			}

			if (i + 3 < BOARD_WIDTH && j + 3 < BOARD_HEIGHT)
			{
				if (board[i][j] == board[i + 1][j + 1] && board[i][j] == board[i + 2][j + 2] && board[i][j] == board[i + 3][j + 3])
				{
					*player = board[i][j] == Space::Red ? Player::Red : Player::Yellow;
					return true;
				}
			}

			if (i - 3 >= 0 && j + 3 < BOARD_HEIGHT)
			{
				if (board[i][j] == board[i - 1][j + 1] && board[i][j] == board[i - 2][j + 2] && board[i][j] == board[i - 3][j + 3])
				{
					*player = board[i][j] == Space::Red ? Player::Red : Player::Yellow;
					return true;
				}
			}
		}
	}

	return false;
}
