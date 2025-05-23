/**============================================================================
Name        : Client.cpp
Created on  : 23.05.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Client.h
============================================================================**/

#include "Client.h"
#include "Application.h"

#include "quickfix/config.h"

#include "quickfix/FileStore.h"
#include "quickfix/SocketInitiator.h"
#ifdef HAVE_SSL
#include "quickfix/SSLSocketInitiator.h"
#include "quickfix/ThreadedSSLSocketInitiator.h"
#endif
#include "Application.h"
#include "quickfix/Log.h"
#include "quickfix/SessionSettings.h"

#include <fstream>
#include <iostream>
#include <memory>
#include <string>

namespace
{
    const std::string confFile {
        R"(/home/andtokm/Projects/FinanceTechnologyProjects/FixExperiments/config/client_settings.conf)"
    };

}

namespace FixTests::Client
{
    void runClient()
    {
        try {
            FIX::SessionSettings settings(confFile);

            Application application;
            FIX::FileStoreFactory storeFactory(settings);
            FIX::ScreenLogFactory logFactory(settings);

            std::unique_ptr<FIX::Initiator> initiator = std::make_unique<FIX::SocketInitiator>(
                application, storeFactory, settings, logFactory);

            initiator->start();
            application.run();
            initiator->stop();
        } catch (std::exception &e) {
            std::cout << e.what();
        }
    }
}

void FixTests::Client::TestAll()
{
    runClient();
}

