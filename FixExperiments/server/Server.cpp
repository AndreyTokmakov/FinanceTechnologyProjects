/**============================================================================
Name        : Server.cpp
Created on  : 04.05.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Server.h
============================================================================**/

#include "Server.h"
#include "Application.h"

#include "quickfix/config.h"
#include "quickfix/Session.h"
#include "quickfix/FileStore.h"
#include "quickfix/SocketAcceptor.h"

#ifdef HAVE_SSL
#include "quickfix/SSLSocketAcceptor.h"
#include "quickfix/ThreadedSSLSocketAcceptor.h"
#endif

#include "quickfix/Log.h"
#include "quickfix/Utility.h"

namespace
{
    void wait()
    {
        std::cout << "Type Ctrl-C to quit" << std::endl;
        while (true) {
            FIX::process_sleep(1);
        }
    }

    const std::string confFile {
        R"(/home/andtokm/Projects/FinanceTechnologyProjects/FixExperiments/config/acceptor_settings.conf)"
    };

}

namespace FixTests::Server
{
    void runAcceptor()
    {
        try
        {
            FIX::SessionSettings settings(confFile);
            Application application;
            FIX::FileStoreFactory storeFactory(settings);
            FIX::ScreenLogFactory logFactory(settings);

            std::unique_ptr<FIX::Acceptor> acceptor = std::make_unique<FIX::SocketAcceptor>(
                application, storeFactory, settings, logFactory);

            acceptor->start();
            wait();
            acceptor->stop();

        }
        catch (const std::exception& exc)
        {
            std::cerr << exc.what() << std::endl;
        }
    }
}

void FixTests::Server::TestAll()
{
    runAcceptor();
}

