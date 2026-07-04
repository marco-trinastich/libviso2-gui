#ifndef APPLICATION_H
#define APPLICATION_H

/******************************************************************
 *                                                                 *
 *  Application                                                     *
 *                                                                 *
 ******************************************************************/

#include "state.h"
#include "gui_handler.h"
#include "core_handler.h"

namespace VisoGui
{
    namespace Core
    {
        class Application
        {
        public:
            Application();
            void Start(HINSTANCE hInstance, std::string lpCmdLine);

        private:
            VisoGui::Core::Models::State state;
            VisoGui::Core::Handlers::GuiHandler guiHandler;
            VisoGui::Core::Handlers::CoreHandler coreHandler;
        };
    }
}

#endif // APPLICATION_H
