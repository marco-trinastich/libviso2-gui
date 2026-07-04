#include "viso_core.h"

#include "core_utils.h"
#include "core_handler.h"

using namespace VisoGui::Core::Utils;
using namespace VisoGui::Core::Models;

using namespace VisoGui::Core::Handlers;

CoreHandler::CoreHandler(State &state)
	: state(state)
{
}

void CoreHandler::Run(const std::string &lpCmdLine)
{
	auto mainThreadId = GetCurrentThreadId();

	// Start Viso elaboration in a new thread
	CoreUtils::NewThread([this, lpCmdLine, mainThreadId]()
						 {
	 	if(!Viso::Core::StartViso(this->state, lpCmdLine)){
			    // On failure, post the WM_QUIT message to quit the application
				PostThreadMessage(mainThreadId, WM_QUIT, 0, 0);
		} });
}
