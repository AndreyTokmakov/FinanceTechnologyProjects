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
#if 0
    using Common::Buffer;

    template<typename Service>
    concept HasHandleMethod = requires(Service service, Buffer& buffer) {
        { service.handle(buffer) } -> std::same_as<bool>;
    };

    /*
    template<typename Server>
    concept HasRunMethod = requires(Server server, Buffer& buffer) {
        { server.run() } -> std::same_as<void>;
    };*/


    template<HasHandleMethod ServiceType>
    struct ServerBase /** : IServer **/
    {
        // TODO: ???  do we need VIRTUAL ??
        virtual void run() = 0;
        virtual ~ServerBase() = default;

        void setService(ServiceType* const serviceImpl) noexcept {
            service = serviceImpl;
        }

    protected:

        // TODO: Rename
        inline bool process(Buffer& buffer) noexcept
        {
            // NOTE: No Virtual dispatch
            return service->handle(buffer);
        }

    private:
        ServiceType* service { nullptr };
    };


    template<HasHandleMethod ImplType>
    struct ModuleBase
    {
        using ServerType = ServerBase<ImplType>;

        explicit ModuleBase(ServerType& serverImpl): server { serverImpl } {
            server.setService(static_cast<ImplType*>(this));
        }

        void start()
        {
            // NOTE: One virtual dispatch: non critical
            server.run();
        }


    private:
        ServerType& server;
    };


    /*

    template<typename Module>
    struct ServerImpl : ServerBase<Module>
    {
        Common::Buffer buffer;

        void run() override
        {
            for (int i = 0; i < 10; ++i)
            {
                std::string message { "Message-" + std::to_string(i) };
                buffer.validateCapacity(message.size());
                std::copy_n(message.data(), message.size(), buffer.head());
                buffer.incrementLength(message.length());
                ServerBase<Module>::process(buffer);
                buffer.reset();
            }
        }
    };


    struct Service final : public ModuleBase<Service>
    {
        using ModuleBase<Service>::ModuleBase;

        bool handle(Buffer& buffer)
        {
            std::cout << std::string_view(buffer.data<char>(), buffer.size())<< std::endl;
            return true;
        }
    };
     */


    struct ServiceImpl : public ModuleBase<ServiceImpl>
    {

        bool handle(Buffer& buffer){
            return true;
        }
    };

    /*
    template<typename Module>
    struct ServerImpl : ServerBase<Module>
    {
        void run() override { }
    };*/

#endif

    void test()
    {
        // ServerImpl<Service> server;
        // Service module { server };
        // module.start();



        //ServerImpl<ServiceImpl> serverBase;
    }
}

namespace Concepts_CRTP_DeducingThis
{
    struct Event {};

    template<typename Service>
    concept Handler = requires(Service service, Event event) {
        { service.handle(event) } -> std::same_as<bool>;
    };


    template<typename Module>
    struct BaseServer
    {
        inline void setService(Module* const moduleImpl) noexcept {
            module = moduleImpl;
        }

        void process(Event event) noexcept
        {
            // NOTE: No Virtual dispatch
            module->handle(event);
        }

    private:
        Module* module { nullptr };
    };

    struct BaseModule
    {
        /*
        using ServerType = BaseServer<BaseModule>;

        explicit BaseModule(ServerType& serverImpl): server { serverImpl } {
            initialize();
        }
        */

        template <typename Self>
        void initialize(this Self self){
            // self.server.setService(&self);
            self.setService();
        }

        // TODO: Remove ?? Concepts ??
        void setService(){
            std::cout << "ServiceBase::setService()" << '\n';
        }

        // BaseServer<BaseModule>& server;
    };


    template<typename ModuleType>
    struct RealServer: BaseServer<ModuleType>
    {

    };

    struct RealService: BaseModule
    {
        using BaseModule::BaseModule;

        void setService(){
            std::cout << "RealService::setService()" << '\n';
        }
    };

    void test()
    {
        RealServer<RealService> server;
        RealService service;
        service.initialize();
    }
}


void Experiments::Service_Demo_Two::TestAll()
{
    // One::test();
    // Two::test();

    Concepts_CRTP_DeducingThis::test();
}
