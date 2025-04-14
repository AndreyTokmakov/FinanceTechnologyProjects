/**============================================================================
Name        : BookBuilder.cpp
Created on  : 14.04.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : BookBuilder.cpp
============================================================================**/

#include "Modules.h"

#include "BaseService.h"
#include "UdsServerAsynch.h"

#include <iostream>
#include <string_view>

namespace
{

    using Common::Buffer;
    using Common::BaseServer;
    using Common::BaseService;
    using Servers::UdsServerAsynch;

    struct BookBuilderModule: BaseService<BookBuilderModule>
    {
        using BaseService<BookBuilderModule>::BaseService;

        bool handle(Buffer& buffer)
        {
            std::cout << std::string(160, '=') << std::endl;
            std::cout << std::string_view(buffer.data<char>(), buffer.size())<< std::endl;
            return true;
        }
    };
}


void Modules::BookBuilder::TestAll()
{
    UdsServerAsynch<BookBuilderModule> server( "/tmp/unix_socket");
    BookBuilderModule service { server };

    if (const std::expected<bool, std::string> ok = server.init(); !ok.has_value()) {
        std::cerr << "Failed to initialize server. Error: " << ok.error() << std::endl;
        return;
    }

    service.start();
}
