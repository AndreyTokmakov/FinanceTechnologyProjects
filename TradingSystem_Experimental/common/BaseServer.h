/**============================================================================
Name        : BaseServer.h
Created on  : 13.08.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : BaseServer.h
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_BASESERVER_H
#define FINANCETECHNOLOGYPROJECTS_BASESERVER_H

#include "Buffer.h"

namespace Common
{
    using Common::Buffer;

    template<typename ServiceType>
    struct BaseServer
    {
        virtual void run() = 0;
        virtual ~BaseServer() = default;

        void setService(ServiceType* const serviceImpl) noexcept {
            service = serviceImpl;
        }

    protected:
        void process(Buffer& buffer) noexcept {
            service->handle(buffer);
        }

        ServiceType* service { nullptr };
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_BASESERVER_H
