/**============================================================================
Name        : DebugApplication.h
Created on  : 26.05.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : DebugApplication.h
============================================================================**/

#include "DebugApplication.h"

#include "quickfix/Application.h"
#include "quickfix/MessageCracker.h"

#include <thread>

/*
#include "quickfix/Mutex.h"
#include "quickfix/Values.h"

#include "quickfix/fix40/ExecutionReport.h"
#include "quickfix/fix40/NewOrderSingle.h"
#include "quickfix/fix40/OrderCancelReject.h"
#include "quickfix/fix40/OrderCancelReplaceRequest.h"
#include "quickfix/fix40/OrderCancelRequest.h"

#include "quickfix/fix41/ExecutionReport.h"
#include "quickfix/fix41/NewOrderSingle.h"
#include "quickfix/fix41/OrderCancelReject.h"
#include "quickfix/fix41/OrderCancelReplaceRequest.h"
#include "quickfix/fix41/OrderCancelRequest.h"

#include "quickfix/fix42/ExecutionReport.h"
#include "quickfix/fix42/NewOrderSingle.h"
#include "quickfix/fix42/OrderCancelReject.h"
#include "quickfix/fix42/OrderCancelReplaceRequest.h"
#include "quickfix/fix42/OrderCancelRequest.h"

#include "quickfix/fix43/ExecutionReport.h"
#include "quickfix/fix43/MarketDataRequest.h"
#include "quickfix/fix43/NewOrderSingle.h"
#include "quickfix/fix43/OrderCancelReject.h"
#include "quickfix/fix43/OrderCancelReplaceRequest.h"
#include "quickfix/fix43/OrderCancelRequest.h"

#include "quickfix/fix50/ExecutionReport.h"
#include "quickfix/fix50/MarketDataRequest.h"
#include "quickfix/fix50/NewOrderSingle.h"
#include "quickfix/fix50/OrderCancelReject.h"
#include "quickfix/fix50/OrderCancelReplaceRequest.h"
#include "quickfix/fix50/OrderCancelRequest.h"
*/

#include "quickfix/fix44/ExecutionReport.h"
#include "quickfix/fix44/MarketDataRequest.h"
#include "quickfix/fix44/NewOrderSingle.h"
#include "quickfix/fix44/OrderCancelReject.h"
#include "quickfix/fix44/OrderCancelReplaceRequest.h"
#include "quickfix/fix44/OrderCancelRequest.h"

namespace
{
    namespace Fix44
    {
        std::unique_ptr<FIX::Message> buildOrder(const std::string& orderId,
                                                 const std::string& symbol,
                                                 const double price,
                                                 const int64_t quantity)
        {
            std::unique_ptr<FIX44::NewOrderSingle> order { std::make_unique<FIX44::NewOrderSingle>(
                FIX::ClOrdID(orderId), FIX::Side(FIX::Side_BUY), FIX::TransactTime(), FIX::OrdType(FIX::OrdType_MARKET))
            };

            order->set(FIX::Symbol(symbol));
            order->set( FIX::OrderQty(quantity));
            order->set(FIX::TimeInForce(FIX::TimeInForce_DAY));
            order->set(FIX::Price(price));

            return order;
        }
    }

}

namespace FixTests::Client
{
    void DebugApplication::onCreate(const FIX::SessionID &)
    {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
    }

    void DebugApplication::toAdmin(FIX::Message &, const FIX::SessionID &)
    {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
    }

    void DebugApplication::onLogon(const FIX::SessionID &sessionID)
    {
        std::cout << std::endl << "Logon - " << sessionID << std::endl;
    }

    void DebugApplication::onLogout(const FIX::SessionID &sessionID)
    {
        std::cout << std::endl << "Logout - " << sessionID << std::endl;
    }

    void DebugApplication::fromApp(const FIX::Message &message,
                              const FIX::SessionID &sessionID)
        EXCEPT(FIX::FieldNotFound, FIX::IncorrectDataFormat, FIX::IncorrectTagValue, FIX::UnsupportedMessageType)
    {
        crack(message, sessionID);
        std::cout << std::endl << "IN: " << message << std::endl;
    }

    void DebugApplication::toApp(FIX::Message &message,
                            const FIX::SessionID &sessionID) EXCEPT(FIX::DoNotSend)
    {
        try {
            FIX::PossDupFlag possDupFlag;
            message.getHeader().getField(possDupFlag);
            if (possDupFlag) {
                throw FIX::DoNotSend();
            }
        } catch (FIX::FieldNotFound &) {}

        std::cout << std::endl << "OUT: " << message << std::endl;
    }

    void DebugApplication::onMessage(const FIX40::ExecutionReport &, const FIX::SessionID &) {
    }

    void DebugApplication::onMessage(const FIX40::OrderCancelReject &, const FIX::SessionID &) {
    }

    void DebugApplication::onMessage(const FIX41::ExecutionReport &, const FIX::SessionID &) {
    }

    void DebugApplication::onMessage(const FIX41::OrderCancelReject &, const FIX::SessionID &) {
    }

    void DebugApplication::onMessage(const FIX42::ExecutionReport &, const FIX::SessionID &) {
    }

    void DebugApplication::onMessage(const FIX42::OrderCancelReject &, const FIX::SessionID &) {
    }

    void DebugApplication::onMessage(const FIX43::ExecutionReport &, const FIX::SessionID &) {
    }

    void DebugApplication::onMessage(const FIX43::OrderCancelReject &, const FIX::SessionID &) {
    }

    void DebugApplication::onMessage(const FIX44::ExecutionReport &, const FIX::SessionID &) {
    }

    void DebugApplication::onMessage(const FIX44::OrderCancelReject &, const FIX::SessionID &) {
    }

    void DebugApplication::onMessage(const FIX50::ExecutionReport &, const FIX::SessionID &) {
    }

    void DebugApplication::onMessage(const FIX50::OrderCancelReject &, const FIX::SessionID &) {
    }

    void DebugApplication::run()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds (250U));

        std::unique_ptr<FIX::Message> order = Fix44::buildOrder("123345441", "APPL", 10, 10);

        order->getHeader().setField(FIX::SenderCompID("1002"));
        order->getHeader().setField(FIX::TargetCompID("1001"));

        FIX::Session::sendToTarget(*order);

        return;
    }
}

