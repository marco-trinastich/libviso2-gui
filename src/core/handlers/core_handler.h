#ifndef CORE_HANDLER_H
#define CORE_HANDLER_H

/******************************************************************
 *                                                                 *
 *  Core Handler                                                   *
 *                                                                 *
 ******************************************************************/

#include "state.h"

namespace VisoGui
{
    namespace Core
    {
        namespace Handlers
        {
            class CoreHandler
            {
            public:
                CoreHandler(VisoGui::Core::Models::State &state);
                void Run(const std::string &lpCmdLine);

            private:
                VisoGui::Core::Models::State &state;
            };
        }
    }
}

#endif // CORE_HANDLER_H
