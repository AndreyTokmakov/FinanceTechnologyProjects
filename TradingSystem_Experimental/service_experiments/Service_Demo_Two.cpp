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

namespace
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
}


void Experiments::Service_Demo_Two::TestAll()
{

    std::unique_ptr<IServer> server { std::make_unique<RealServer>() };
    Service service { std::move(server) };

    service.start();
}
