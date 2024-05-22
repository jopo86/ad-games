#include "Application.h"

#include "SpikeDodge.h"
#include "MathGates.h"
#include "ConnectFour.h"
#include "Cannon.h"

Application::Application()
{
	
}

void Application::run()
{
	m_errorHandler = Onyx::ErrorHandler(true, true);
	Onyx::Init(m_errorHandler);

	Cannon::Run();
}

void Application::dispose()
{
	Onyx::Terminate();
}
