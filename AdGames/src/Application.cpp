#include "Application.h"

Application::Application()
{
	
}

void Application::run()
{
	m_errorHandler = Onyx::ErrorHandler(true, true);
	Onyx::Init(m_errorHandler);

	SpikeDodge::Run();
}

void Application::dispose()
{
	Onyx::Terminate();
}
