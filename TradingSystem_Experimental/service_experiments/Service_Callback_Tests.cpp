/**============================================================================
Name        : Service_Callback_Tests.cpp
Created on  : 13.04.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Service_Callback_Tests.cpp
============================================================================**/

#include "Experiments.h"
#include "Buffer.h"

#include <iostream>
#include <string_view>
#include <memory>
#include <concepts>
#include <functional>

namespace ExampleOne
{
    struct Event {};

    template<typename Service>
    concept Handler = requires(Service service, Event event) {
        { service.handle(event) } -> std::same_as<bool>;
    };

    struct IService
    {
        virtual bool handle(Event event){
            std::cout << "IService::handle()" << std::endl;
            return true;
        }

        virtual ~IService() = default;
    };

    struct Server
    {
        using methodPtr = bool (IService::*)(Event);

        explicit Server(std::unique_ptr<IService>&& obj):
                serviceImpl { std::move(obj) }, callback { &IService::handle } {
        }

        bool call(Event event) {
            return std::invoke(callback, serviceImpl, event);
        }

        std::unique_ptr<IService> serviceImpl { nullptr };
        methodPtr callback;
    };

    struct Service: IService
    {
        bool handle(Event event) override {
            std::cout << "Service::handle()" << std::endl;
            return true;
        }
    };

    void demo()
    {
        std::unique_ptr<IService> service { std::make_unique<Service>() };

        Server server { std::move(service) };
        server.call(Event{});
    }
}


namespace ExampleTwo
{
    struct Event {};

    template<typename Service>
    concept Handler = requires(Service service, Event event) {
        { service.handle(event) } -> std::same_as<bool>;
    };

    template<Handler Service>
    struct Server
    {
        using methodPtr = bool (Service::*)(Event);

        explicit Server(Service& service):
                serviceImpl { service }, callback { &Service::handle } {
        }

        bool call(Event event) {
            return std::invoke(callback, serviceImpl, event);
        }

        Service& serviceImpl;
        methodPtr callback;
    };

    struct Service
    {
        bool handle(Event event) {
            std::cout << "Service::handle()" << std::endl;
            return true;
        }
    };

    void demo()
    {
        Service service;

        Server<Service> server { service };
        server.call(Event{});
    }
}

namespace ExampleThree
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

    void demo()
    {
        RealServer server;
        RealService service { server };
        service.start();
    }
}


void Experiments::Service_Callback_Tests::TestAll()
{
    // ExampleOne::demo();
    // ExampleTwo::demo();
    ExampleThree::demo();
}
