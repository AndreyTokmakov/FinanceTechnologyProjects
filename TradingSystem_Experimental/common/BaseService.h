/**============================================================================
Name        : BaseService.h
Created on  : 13.08.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : BaseService.h
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_BASESERVICE_H
#define FINANCETECHNOLOGYPROJECTS_BASESERVICE_H

#include "BaseServer.h"

namespace Common
{
    using Common::Buffer;

    template<typename Derived>
    struct BaseService
    {
        using ServerType = BaseServer<Derived>;

        explicit BaseService(ServerType& serverImpl): server { serverImpl } {
            server.setService(static_cast<Derived*>(this));
        }

        void start()
        {
            // NOTE: One virtual dispatch: non critical
            server.run();
        }

    private:

        ServerType& server;
    };
}


#endif //FINANCETECHNOLOGYPROJECTS_BASESERVICE_H
