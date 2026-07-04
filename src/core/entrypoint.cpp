#include "settings.h"
#include "core_utils.h"
#include "application.h"

using namespace VisoGui::Core;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{

	if (Models::Settings::Core::isConsoleEnabled)
	{
		Utils::CoreUtils::AttachParentConsole(Models::Settings::Core::maxConsoleLines);
	}

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nCmdShow);

	HeapSetInformation(nullptr, HeapEnableTerminationOnCorruption, nullptr, 0);
	HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

	// Initialize Application
	Application app;
	app.Start(hInstance, lpCmdLine);

	// Main window(s) message loop:
	BOOL fRet;
	MSG msg;
	while ((fRet = GetMessage(&msg, nullptr, 0, 0)) != 0)
	{
		if (fRet == -1)
		{
			break;
		}
		else
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	if (Models::Settings::Core::isConsoleEnabled)
	{
		Utils::CoreUtils::ReleaseConsole();
	}

	CoUninitialize();

	return (int)msg.wParam;
}
