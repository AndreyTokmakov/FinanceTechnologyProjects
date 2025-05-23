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

namespace FixTests::Client
{
    struct Application : FIX::Application,  FIX::MessageCracker
    {
        void run();

    private:
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

        void queryEnterOrder();
        void queryCancelOrder();
        static void queryReplaceOrder();
        static void queryMarketDataRequest();

        static FIX40::NewOrderSingle queryNewOrderSingle40();
        static FIX41::NewOrderSingle queryNewOrderSingle41();
        static FIX42::NewOrderSingle queryNewOrderSingle42();
        static FIX43::NewOrderSingle queryNewOrderSingle43();
        static FIX44::NewOrderSingle queryNewOrderSingle44();
        static FIX50::NewOrderSingle queryNewOrderSingle50();
        static FIX40::OrderCancelRequest queryOrderCancelRequest40();
        static FIX41::OrderCancelRequest queryOrderCancelRequest41();
        static FIX42::OrderCancelRequest queryOrderCancelRequest42();
        static FIX43::OrderCancelRequest queryOrderCancelRequest43();
        static FIX44::OrderCancelRequest queryOrderCancelRequest44();
        static FIX50::OrderCancelRequest queryOrderCancelRequest50();
        static FIX40::OrderCancelReplaceRequest queryCancelReplaceRequest40();
        static FIX41::OrderCancelReplaceRequest queryCancelReplaceRequest41();
        static FIX42::OrderCancelReplaceRequest queryCancelReplaceRequest42();
        static FIX43::OrderCancelReplaceRequest queryCancelReplaceRequest43();
        static FIX44::OrderCancelReplaceRequest queryCancelReplaceRequest44();
        static FIX50::OrderCancelReplaceRequest queryCancelReplaceRequest50();
        static FIX43::MarketDataRequest queryMarketDataRequest43();
        static FIX44::MarketDataRequest queryMarketDataRequest44();
        static FIX50::MarketDataRequest queryMarketDataRequest50();

        static void queryHeader(FIX::Header &header);
        static char queryAction();
        static int queryVersion();
        static bool queryConfirm(const std::string &query);

        static FIX::SenderCompID querySenderCompID();
        static FIX::TargetCompID queryTargetCompID();
        static FIX::TargetSubID queryTargetSubID();
        static FIX::ClOrdID queryClOrdID();
        static FIX::OrigClOrdID queryOrigClOrdID();
        static FIX::Symbol querySymbol();
        static FIX::Side querySide();static
        FIX::OrderQty queryOrderQty();static
        FIX::OrdType queryOrdType();static
        FIX::Price queryPrice();
        static FIX::StopPx queryStopPx();
        static FIX::TimeInForce queryTimeInForce();
    };

}


#endif //QUICK_FIX_SERVER_APPLICATION_H
