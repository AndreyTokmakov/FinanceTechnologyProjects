/**============================================================================
Name        : Application.h
Created on  : 23.05.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Application.h
============================================================================**/

#ifndef QUICK_FIX_SERVER_APPLICATION_H
#define QUICK_FIX_SERVER_APPLICATION_H

#include "quickfix/Application.h"
#include "quickfix/MessageCracker.h"
#include "quickfix/Values.h"

namespace FixTests::Server
{
    struct Application : FIX::Application, FIX::MessageCracker
    {
        Application();

        void onCreate(const FIX::SessionID &) override;
        void onLogon(const FIX::SessionID &sessionID) override;
        void onLogout(const FIX::SessionID &sessionID) override;
        void toAdmin(FIX::Message &, const FIX::SessionID &) override;
        void toApp(FIX::Message &, const FIX::SessionID &) override EXCEPT(FIX::DoNotSend);
        void fromAdmin(const FIX::Message &, const FIX::SessionID &) override
            EXCEPT(FIX::FieldNotFound, FIX::IncorrectDataFormat, FIX::IncorrectTagValue, FIX::RejectLogon);
        void fromApp(const FIX::Message &message, const FIX::SessionID &sessionID) override
            EXCEPT(FIX::FieldNotFound, FIX::IncorrectDataFormat, FIX::IncorrectTagValue, FIX::UnsupportedMessageType);

        void onMessage(const FIX40::NewOrderSingle &, const FIX::SessionID &) override;
        void onMessage(const FIX41::NewOrderSingle &, const FIX::SessionID &) override;
        void onMessage(const FIX42::NewOrderSingle &, const FIX::SessionID &) override;
        void onMessage(const FIX43::NewOrderSingle &, const FIX::SessionID &) override;
        void onMessage(const FIX44::NewOrderSingle &, const FIX::SessionID &) override;
        void onMessage(const FIX50::NewOrderSingle &, const FIX::SessionID &) override;

        [[nodiscard]]
        std::string genOrderID();

        [[nodiscard]]
        std::string genExecID();

    private:
        int32_t m_orderID { 0 };
        int32_t m_execID { 0 };
    };
}


#endif //QUICK_FIX_SERVER_APPLICATION_H
