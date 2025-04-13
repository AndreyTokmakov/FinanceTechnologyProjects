/**============================================================================
Name        : Modular_Service_Final.cpp
Created on  : 13.04.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Modular_Service_Final.cpp
============================================================================**/

#include "Experiments.h"
#include "Buffer.h"

#include <iostream>
#include <string_view>
#include <memory>
#include <concepts>

namespace
{
    using Common::Buffer;

    template<typename ServiceType>
    struct BaseServer
    {
        virtual void run() = 0;
        virtual ~BaseServer() = default;

        inline void setService(ServiceType* const serviceImpl) noexcept {
            service = serviceImpl;
        }

    protected:

        inline void process(Buffer& buffer) noexcept {
            service->handle(buffer);
        }

        ServiceType* service { nullptr };
    };

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

    /// ----------------------------------------------------------------

    struct RealService: BaseService<RealService>
    {
        using BaseService<RealService>::BaseService;
        int counter = 0;

        bool handle(Buffer& buffer)
        {
            std::cout << std::string_view(buffer.data<char>(), buffer.size())<< std::endl;
            ++counter;
            return true;
        }
    };

    struct RealServer: BaseServer<RealService>
    {
        Buffer buffer;

        void run() override
        {
            for (int i = 0; i < 10; ++i)
            {
                std::string message { "Message-" + std::to_string(i) };
                buffer.validateCapacity(message.size());
                std::copy_n(message.data(), message.size(), buffer.head());
                buffer.incrementLength(message.length());
                process(buffer);
                buffer.reset();
            }
        }
    };


}


void Experiments::Modular_Service_Final::TestAll()
{
    RealServer server;
    RealService service { server };
    service.start();
}
