/**============================================================================
Name        : Service_Demo_Two.cpp
Created on  : 30.03.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Service_Demo_Two.cpp
============================================================================**/

#include "Experiments.h"
#include "Buffer.h"

#include <iostream>
#include <string_view>
#include <memory>
#include <concepts>

namespace One
{
    struct IServer
    {
        [[maybe_unused]]
        virtual bool getMessage(Common::Buffer& buffer) = 0;

        virtual ~IServer() = default;
    };

    struct RealServer: IServer
    {
        [[maybe_unused]]
        bool getMessage(Common::Buffer& buffer) override
        {
            for (int i = 'a'; i < 'z'; ++i)
            {
                std::string message(128, (char)i);
                buffer.validateCapacity(128);
                std::copy_n(message.data(), message.size(), buffer.head());
                buffer.incrementLength(message.length());
            }

            return true;
        }
    };


    struct Service
    {
        std::unique_ptr<IServer> server { nullptr };

        explicit Service(std::unique_ptr<IServer>&& serverImpl) : server { std::move(serverImpl) } {
        }

        void start()
        {
            Common::Buffer buffer;
            server->getMessage(buffer);

            std::string result(buffer.data<char>(), buffer.size());
            std::cout << result << std::endl;
            std::cout << result.size() << std::endl;
        }
    };

    void test()
    {

        std::unique_ptr<IServer> server { std::make_unique<RealServer>() };
        Service service { std::move(server) };

        service.start();
    }
}


namespace Two
{
    template<typename ServiceType>
    concept HasHandle = requires(ServiceType service, Common::Buffer& buffer) {
        { service.handle(buffer) } -> std::same_as<bool>;
    };

    template<typename Server>
    concept HasRunMethod = requires(Server server, Common::Buffer& buffer) {
        { server.run() } -> std::same_as<void>;
    };


    template<HasHandle ServiceType>
    struct ServerBase /** : IServer **/
    {
        virtual void run() = 0;
        virtual ~ServerBase() = default;

        void setService(ServiceType* const serviceImpl) noexcept {
            service = serviceImpl;
        }

    private:
        ServiceType* service { nullptr };
    };


    template<HasRunMethod Srv>
    struct ModuleBase
    {
        using ServerType = ServerBase<Srv>;

        explicit ModuleBase(ServerType& serverImpl): server { serverImpl } {
            server.setService(static_cast<Srv*>(this));
        }

        void start()
        {
            // NOTE: One virtual dispatch: non critical
            server.run();
        }

    private:
        ServerType& server;
    };


    template<HasHandle ServiceType>
    struct ServerImpl final : ServerBase<ServiceType>
    {
        void run() override
        {

        }
    };


    struct ModuleImpl final : public ModuleBase<Service>
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

    }
}


void Experiments::Service_Demo_Two::TestAll()
{
    // One::test();
    Two::test();
}
