/**============================================================================
Name        : IService.h
Created on  : 30.03.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : IService.h
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_ISERVICE_H
#define FINANCETECHNOLOGYPROJECTS_ISERVICE_H

#include <array>
#include <vector>

namespace Service_Experimental
{
    struct Buffer
    {
        using size_type = int;

        std::array<unsigned char, std::hardware_destructive_interference_size * 16 - sizeof(size_type)> data {};
        size_type size { 0 };
    };

    struct BufferVec
    {
        std::vector<unsigned char> data;
    };

    // TODO: Rename Server to 'Channel'

    // TODO: Concepts on ServiceType type:
    //    - handle()
    template<typename ServiceType>
    struct ServerBase
    {
        virtual void run() = 0;
        virtual ~ServerBase() = default;

        inline void setService(ServiceType* const serviceImpl) noexcept {
            service = serviceImpl;
        }

    protected:

        inline void handle(BufferVec& buffer) noexcept
        {
            // NOTE: No Virtual dispatch
            service->handle(buffer);
        }

    private:
        ServiceType* service { nullptr };
    };

    template<typename Derived>
    struct ServiceBase
    {
        using ServerType = ServerBase<Derived>;

        explicit ServiceBase(ServerType& serverImpl): server { serverImpl } {
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

    void TestAll();
}

#endif //FINANCETECHNOLOGYPROJECTS_ISERVICE_H
