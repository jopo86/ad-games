#pragma once

#include <Onyx/Core.h>

class Application
{
public:
	Application();
	
	void run();
	void dispose();

private:
	Onyx::ErrorHandler m_errorHandler;
};
