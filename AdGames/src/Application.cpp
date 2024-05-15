#include "Application.h"

#include "spike-dodge/SpikeDodge.h"
#include "math-gates/MathGates.h"

Application::Application()
{
	
}

void Application::run()
{
	m_errorHandler = Onyx::ErrorHandler(true, true);
	Onyx::Init(m_errorHandler);

	MathGates::Run();
}

void Application::dispose()
{
	Onyx::Terminate();
}
