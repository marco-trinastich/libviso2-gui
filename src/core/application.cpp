#include "application.h"

using namespace VisoGui::Core;

Application::Application()
	: state(),
	  guiHandler(this->state),
	  coreHandler(this->state) {}

void Application::Start(HINSTANCE hInstance, std::string lpCmdLine)
{
	// Init gui and elaboration thread
	this->guiHandler.Initialize(hInstance);
	this->coreHandler.Run(lpCmdLine);
}
