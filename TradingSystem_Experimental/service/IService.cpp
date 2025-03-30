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

namespace Demo_One
{   // TODO:
    //  - Non-copiable ?? Non-Move-able
    //  - CRTP ???
    //  - Concepts on Server  ??
    //  - Concepts on Handler ??

    struct Buffer
    {
        using size_type = int;

        std::array<unsigned char, std::hardware_destructive_interference_size * 16 - sizeof(size_type)> data {};
        size_type size { 0 };
    };

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


void Service::TestAll()
{
    Demo_One::test();
}
