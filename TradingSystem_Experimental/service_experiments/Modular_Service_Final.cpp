/**============================================================================
Name        : Modular_Service_Final.cpp
Created on  : 13.04.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Modular_Service_Final.cpp
============================================================================**/

#include "Experiments.h"
#include "BaseService.h"

#include <iostream>
#include <string_view>

namespace
{
    using Common::Buffer;
    using Common::BaseServer;
    using Common::BaseService;


    struct RealService: BaseService<RealService>
    {
        using BaseService<RealService>::BaseService;

        bool handle(Buffer& buffer)
        {
            std::cout << std::string_view(buffer.data<char>(), buffer.size())<< std::endl;
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
                BaseServer<RealService>::process(buffer);
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
