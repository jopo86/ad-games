#pragma once

#include <Onyx/Core.h>

#include "army-math-game/ArmyMathGame.h"

class Application
{
public:
	Application();
	
	void run();
	void dispose();

private:
	Onyx::ErrorHandler m_errorHandler;
};
