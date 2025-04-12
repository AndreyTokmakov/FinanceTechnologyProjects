/**============================================================================
Name        : Module_Impl_Test.cpp
Created on  : 31.03.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Module_Impl_Test.cpp
============================================================================**/

#include "UDSServer.h"
#include "IService.h"
#include "Tests.h"

#include <iostream>
#include <string>

namespace
{
    using namespace Service_Experimental;
    using namespace Gateway_Experimental;

    struct TestService final : public ServiceBase<TestService>
    {
        using ServiceBase<TestService>::ServiceBase;

        void handle(BufferVec& buffer)
        {
            std::cout << "TestService: " << std::string_view(
                    reinterpret_cast<const char *>(buffer.data.data()), buffer.data.size())<< std::endl;
        }
    };

    void test()
    {
        UDSServer<TestService> server( "/tmp/unix_socket");
        TestService service { server };

        const std::expected<bool, std::string> ok = server.init();
        if (!ok.has_value()) {
            std::cerr << "Failed to initialize server. Error: " << ok.error() << std::endl;
            return;
        }


        service.start();
    }
}


void Tests::Module_Impl_Test::TestAll()
{
    test();
}
