/**============================================================================
Name        : IService.cpp
Created on  : 30.03.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : IService.cpp
============================================================================**/

#include "IService.h"

#include <iostream>
#include <string_view>
#include <array>

namespace
{
    struct Buffer
    {
        using size_type = int;

        std::array<unsigned char, std::hardware_destructive_interference_size * 16 - sizeof(size_type)> data {};
        size_type size { 0 };
    };
}


namespace Demo_One
{   // TODO:
    //  - Non-copiable ?? Non-Move-able
    //  - CRTP ???
    //  - Concepts on Server  ??
    //  - Concepts on Handler ??


    template<typename Server, typename Handler, typename Self>
    struct IService: public Server, public Handler
    {
        IService() = default;

        void run()
        {
            while (running()) {
                getMessage(buffer);
                handle(buffer);
            }
        }

    private:

        [[nodiscard]]
        inline Self& self() noexcept {
            return static_cast<Self&>(*this);
        }

        [[nodiscard]]
        inline bool running() noexcept
        {
            return !self().stop();
        }

    private:

        using Server::getMessage;
        using Handler::handle;

        Buffer buffer;
    };


    struct ServerImpl
    {
        void getMessage(Buffer& buffer)
        {
            std::cout << "ServerImpl::getMessage()" << std::endl;

            buffer.size = 7;
            std::copy_n("Message", buffer.size, buffer.data.begin());
        }
    };

    struct HandlerImpl
    {
        void handle(Buffer& buffer)
        {
            std::cout << "ServerImpl::handle(): message = " <<
                      std::string_view(reinterpret_cast<const char *>(buffer.data.data()), buffer.size)<< std::endl;
        }
    };


    struct OMSService: public IService<ServerImpl, HandlerImpl, OMSService>
    {
        int counter = 10;

        [[nodiscard]]
        bool stop()
        {
            --counter;
            return 0 == counter;
        }
    };


    void test()
    {
        OMSService service;
        service.run();
    }
}

namespace Demo_Two
{

    struct IService
    {
        virtual void handle(Buffer& buffer) = 0;
        virtual ~IService() = default;
    };

    struct MarkerDataServer
    {
        void start()
        {
            for (int i = 0; i < 10; ++i)
            {
                buffer.size = 7;
                std::copy_n("Message", buffer.size, buffer.data.begin());

                service->handle(buffer);
            }
        }

        void setService(IService* serviceImpl) {
            service = serviceImpl;
        }

        Buffer buffer;
        IService* service { nullptr };
    };

    // TODO: Concepts on Server type:
    //    - setService()
    template<typename Server>
    struct Service: public IService
    {
        explicit Service(Server& serverImpl): server { serverImpl } {
        }

        void handle(Buffer& buffer) override
        {
            std::cout << "Service::handle(): message = " << std::string_view(
                    reinterpret_cast<const char *>(buffer.data.data()), buffer.size)<< std::endl;
        }

        void start() {
            server.setService(this);
            server.start();
        }


        Server& server;
    };



    void test()
    {
        MarkerDataServer server;
        Service<MarkerDataServer> service {server};
        service.start();

    }
}


namespace Demo_Three
{
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

        inline void handle(Buffer& buffer) noexcept
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



    template<typename ServiceType>
    struct Server1: ServerBase<ServiceType>
    {
        using ServerBase<ServiceType>::handle;

        void run() override
        {
            Buffer buffer;
            std::string message { "Message_1" };
            for (int i = 0; i < 10; ++i) {
                buffer.size = message.size();
                std::copy_n(message.data(), buffer.size, buffer.data.begin());
                handle(buffer);
            }
        }
    };

    template<typename ServiceType>
    struct Server2: ServerBase<ServiceType>
    {
        using ServerBase<ServiceType>::handle;

        void run() override
        {
            Buffer buffer;
            std::string message { "Message_2" };
            for (int i = 0; i < 10; ++i) {
                buffer.size = message.size();
                std::copy_n(message.data(), buffer.size, buffer.data.begin());
                handle(buffer);
            }
        }
    };


    struct Service final : public ServiceBase<Service>
    {
        using ServiceBase<Service>::ServiceBase;

        void handle(Buffer& buffer)
        {
            std::cout << "Service::handle(): message = " << std::string_view(
                    reinterpret_cast<const char *>(buffer.data.data()), buffer.size)<< std::endl;
        }
    };

    void test()
    {
        Server1<Service> server;
        // Server2<Service> server;

        Service service { server };
        service.start();
    }
}



void Service_Experimental::TestAll()
{
    // Demo_One::test();
    // Demo_Two::test();
    Demo_Three::test();
}
