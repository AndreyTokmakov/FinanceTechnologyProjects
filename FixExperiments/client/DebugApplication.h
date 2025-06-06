/**============================================================================
Name        : DebugApplication.h
Created on  : 26.05.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : DebugApplication.h
============================================================================**/

#ifndef QUICK_FIX_CLIENT_DEBUG_APPLICATION_H
#define QUICK_FIX_CLIENT_DEBUG_APPLICATION_H

#include "quickfix/Application.h"
#include "quickfix/MessageCracker.h"
#include "quickfix/Values.h"

namespace FixTests::Client
{
    struct DebugApplication : FIX::Application,  FIX::MessageCracker
    {
        void run();

        void onCreate(const FIX::SessionID &) override;
        void onLogon(const FIX::SessionID &sessionID) override;
        void onLogout(const FIX::SessionID &sessionID) override;
        void toAdmin(FIX::Message &, const FIX::SessionID &) override;
        void toApp(FIX::Message &, const FIX::SessionID &) override EXCEPT(FIX::DoNotSend);
        void fromAdmin(const FIX::Message &, const FIX::SessionID &) override
            EXCEPT(FIX::FieldNotFound, FIX::IncorrectDataFormat, FIX::IncorrectTagValue, FIX::RejectLogon) {}
        void fromApp(const FIX::Message &message, const FIX::SessionID &sessionID) override
            EXCEPT(FIX::FieldNotFound, FIX::IncorrectDataFormat, FIX::IncorrectTagValue, FIX::UnsupportedMessageType);

        void onMessage(const FIX40::ExecutionReport &, const FIX::SessionID &) override;
        void onMessage(const FIX40::OrderCancelReject &, const FIX::SessionID &) override;
        void onMessage(const FIX41::ExecutionReport &, const FIX::SessionID &) override;
        void onMessage(const FIX41::OrderCancelReject &, const FIX::SessionID &) override;
        void onMessage(const FIX42::ExecutionReport &, const FIX::SessionID &) override;
        void onMessage(const FIX42::OrderCancelReject &, const FIX::SessionID &) override;
        void onMessage(const FIX43::ExecutionReport &, const FIX::SessionID &) override;
        void onMessage(const FIX43::OrderCancelReject &, const FIX::SessionID &) override;
        void onMessage(const FIX44::ExecutionReport &, const FIX::SessionID &) override;
        void onMessage(const FIX44::OrderCancelReject &, const FIX::SessionID &) override;
        void onMessage(const FIX50::ExecutionReport &, const FIX::SessionID &) override;
        void onMessage(const FIX50::OrderCancelReject &, const FIX::SessionID &) override;
    };
}

#endif //QUICK_FIX_CLIENT_DEBUG_APPLICATION_H