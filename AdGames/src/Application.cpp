#include "Application.h"

Application::Application()
{
	
}

void Application::run()
{
	m_errorHandler = Onyx::ErrorHandler(true, true);
	Onyx::Init(m_errorHandler);

	ArmyMathGame::Run();
}

void Application::dispose()
{
	Onyx::Terminate();
}
