/**============================================================================
Name        : main.cpp
Created on  : 22.05.2024
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Tests C++ project
============================================================================**/

#include <vector>
#include <iostream>
#include <string_view>

#include "quickfix/config.h"
#include "quickfix/Session.h"
#include "quickfix/FileStore.h"
#include "quickfix/SocketAcceptor.h"
#ifdef HAVE_SSL
#include "quickfix/SSLSocketAcceptor.h"
#include "quickfix/ThreadedSSLSocketAcceptor.h"
#endif
#include "quickfix/Log.h"
#include "quickfix/SessionSettings.h"
#include "quickfix/Application.h"
#include "quickfix/MessageCracker.h"
#include "quickfix/Mutex.h"
#include "quickfix/Utility.h"
#include "quickfix/Values.h"

#include "quickfix/fix40/ExecutionReport.h"
#include "quickfix/fix41/ExecutionReport.h"
#include "quickfix/fix42/ExecutionReport.h"
#include "quickfix/fix43/ExecutionReport.h"
#include "quickfix/fix44/ExecutionReport.h"
#include "quickfix/fix50/ExecutionReport.h"


#include "quickfix/fix40/NewOrderSingle.h"
#include "quickfix/fix41/NewOrderSingle.h"
#include "quickfix/fix42/NewOrderSingle.h"
#include "quickfix/fix43/NewOrderSingle.h"
#include "quickfix/fix44/NewOrderSingle.h"
#include "quickfix/fix50/NewOrderSingle.h"

namespace Experiments
{
    struct Application final : public FIX::Application, public FIX::MessageCracker
    {
        Application(): m_orderID(0),m_execID(0) {

        }

        void onCreate(const FIX::SessionID &) override {
            std::cout << __PRETTY_FUNCTION__ << std::endl;
        }

        void onLogon(const FIX::SessionID &sessionID) override {
            std::cout << __PRETTY_FUNCTION__ << std::endl;
        }

        void onLogout(const FIX::SessionID &sessionID) override {
            std::cout << __PRETTY_FUNCTION__ << std::endl;
        }

        void toAdmin(FIX::Message &message, const FIX::SessionID &sessionID) override  {
            std::cout << __PRETTY_FUNCTION__ << std::endl;
        }

        void toApp(FIX::Message &message, const FIX::SessionID &sessionID) override EXCEPT(FIX::DoNotSend) {
            std::cout << __PRETTY_FUNCTION__ << std::endl;
        }

        void fromAdmin(const FIX::Message &message, const FIX::SessionID &sessionID) override
            EXCEPT(FIX::FieldNotFound, FIX::IncorrectDataFormat, FIX::IncorrectTagValue, FIX::RejectLogon)
        {
            std::cout << __PRETTY_FUNCTION__ << std::endl;
        }
        void fromApp(const FIX::Message &message, const FIX::SessionID &sessionID) override
            EXCEPT(FIX::FieldNotFound, FIX::IncorrectDataFormat, FIX::IncorrectTagValue, FIX::UnsupportedMessageType)
        {
            crack(message, sessionID);
            std::cout << __PRETTY_FUNCTION__ << std::endl;
        }

        void onMessage(const FIX40::NewOrderSingle &message, const FIX::SessionID &sessionID) override
        {
            FIX::Symbol symbol;
            FIX::Side side;
            FIX::OrdType ordType;
            FIX::OrderQty orderQty;
            FIX::Price price;
            FIX::ClOrdID clOrdID;
            FIX::Account account;

            message.get(ordType);

            if (ordType != FIX::OrdType_LIMIT) {
                throw FIX::IncorrectTagValue(ordType.getTag());
            }

            message.get(symbol);
            message.get(side);
            message.get(orderQty);
            message.get(price);
            message.get(clOrdID);

            FIX40::ExecutionReport executionReport = FIX40::ExecutionReport(
                FIX::OrderID(genOrderID()),
                FIX::ExecID(genExecID()),
                FIX::ExecTransType(FIX::ExecTransType_NEW),
                FIX::OrdStatus(FIX::OrdStatus_FILLED),
                symbol,
                side,
                orderQty,
                FIX::LastShares(orderQty),
                FIX::LastPx(price),
                FIX::CumQty(orderQty),
                FIX::AvgPx(price));

            executionReport.set(clOrdID);

            if (message.isSet(account)) {
                executionReport.setField(message.get(account));
            }

            try {
                FIX::Session::sendToTarget(executionReport, sessionID);
            } catch (FIX::SessionNotFound &) {}
        }

        void onMessage(const FIX41::NewOrderSingle &message, const FIX::SessionID &sessionID) override {
            FIX::Symbol symbol;
            FIX::Side side;
            FIX::OrdType ordType;
            FIX::OrderQty orderQty;
            FIX::Price price;
            FIX::ClOrdID clOrdID;
            FIX::Account account;

            message.get(ordType);

            if (ordType != FIX::OrdType_LIMIT) {
                throw FIX::IncorrectTagValue(ordType.getTag());
            }

            message.get(symbol);
            message.get(side);
            message.get(orderQty);
            message.get(price);
            message.get(clOrdID);

            FIX41::ExecutionReport executionReport = FIX41::ExecutionReport(
                FIX::OrderID(genOrderID()),
                FIX::ExecID(genExecID()),
                FIX::ExecTransType(FIX::ExecTransType_NEW),
                FIX::ExecType(FIX::ExecType_FILL),
                FIX::OrdStatus(FIX::OrdStatus_FILLED),
                symbol,
                side,
                orderQty,
                FIX::LastShares(orderQty),
                FIX::LastPx(price),
                FIX::LeavesQty(0),
                FIX::CumQty(orderQty),
                FIX::AvgPx(price));

            executionReport.set(clOrdID);

            if (message.isSet(account)) {
                executionReport.setField(message.get(account));
            }

            try {
                FIX::Session::sendToTarget(executionReport, sessionID);
            } catch (FIX::SessionNotFound &) {}
        }

        void onMessage(const FIX42::NewOrderSingle &message, const FIX::SessionID &sessionID) override {
            FIX::Symbol symbol;
            FIX::Side side;
            FIX::OrdType ordType;
            FIX::OrderQty orderQty;
            FIX::Price price;
            FIX::ClOrdID clOrdID;
            FIX::Account account;

            message.get(ordType);

            if (ordType != FIX::OrdType_LIMIT) {
                throw FIX::IncorrectTagValue(ordType.getTag());
            }

            message.get(symbol);
            message.get(side);
            message.get(orderQty);
            message.get(price);
            message.get(clOrdID);

            FIX42::ExecutionReport executionReport = FIX42::ExecutionReport(
                FIX::OrderID(genOrderID()),
                FIX::ExecID(genExecID()),
                FIX::ExecTransType(FIX::ExecTransType_NEW),
                FIX::ExecType(FIX::ExecType_FILL),
                FIX::OrdStatus(FIX::OrdStatus_FILLED),
                symbol,
                side,
                FIX::LeavesQty(0),
                FIX::CumQty(orderQty),
                FIX::AvgPx(price));

            executionReport.set(clOrdID);
            executionReport.set(orderQty);
            executionReport.set(FIX::LastShares(orderQty));
            executionReport.set(FIX::LastPx(price));

            if (message.isSet(account)) {
                executionReport.setField(message.get(account));
            }

            try {
                FIX::Session::sendToTarget(executionReport, sessionID);
            } catch (FIX::SessionNotFound &) {}
        }

        void onMessage(const FIX43::NewOrderSingle &message, const FIX::SessionID &sessionID) override {
            FIX::Symbol symbol;
            FIX::Side side;
            FIX::OrdType ordType;
            FIX::OrderQty orderQty;
            FIX::Price price;
            FIX::ClOrdID clOrdID;
            FIX::Account account;

            message.get(ordType);

            if (ordType != FIX::OrdType_LIMIT) {
                throw FIX::IncorrectTagValue(ordType.getTag());
            }

            message.get(symbol);
            message.get(side);
            message.get(orderQty);
            message.get(price);
            message.get(clOrdID);

            FIX43::ExecutionReport executionReport = FIX43::ExecutionReport(
                FIX::OrderID(genOrderID()),
                FIX::ExecID(genExecID()),
                FIX::ExecType(FIX::ExecType_FILL),
                FIX::OrdStatus(FIX::OrdStatus_FILLED),
                side,
                FIX::LeavesQty(0),
                FIX::CumQty(orderQty),
                FIX::AvgPx(price));

            executionReport.set(clOrdID);
            executionReport.set(symbol);
            executionReport.set(orderQty);
            executionReport.set(FIX::LastQty(orderQty));
            executionReport.set(FIX::LastPx(price));

            if (message.isSet(account)) {
                executionReport.setField(message.get(account));
            }

            try {
                FIX::Session::sendToTarget(executionReport, sessionID);
            } catch (FIX::SessionNotFound &) {}
        }

        void onMessage(const FIX44::NewOrderSingle &message, const FIX::SessionID &sessionID) override {
            FIX::Symbol symbol;
            FIX::Side side;
            FIX::OrdType ordType;
            FIX::OrderQty orderQty;
            FIX::Price price;
            FIX::ClOrdID clOrdID;
            FIX::Account account;

            message.get(ordType);

            if (ordType != FIX::OrdType_LIMIT) {
                throw FIX::IncorrectTagValue(ordType.getTag());
            }

            message.get(symbol);
            message.get(side);
            message.get(orderQty);
            message.get(price);
            message.get(clOrdID);

            FIX44::ExecutionReport executionReport = FIX44::ExecutionReport(
                FIX::OrderID(genOrderID()),
                FIX::ExecID(genExecID()),
                FIX::ExecType(FIX::ExecType_TRADE),
                FIX::OrdStatus(FIX::OrdStatus_FILLED),
                side,
                FIX::LeavesQty(0),
                FIX::CumQty(orderQty),
                FIX::AvgPx(price));

            executionReport.set(clOrdID);
            executionReport.set(symbol);
            executionReport.set(orderQty);
            executionReport.set(FIX::LastQty(orderQty));
            executionReport.set(FIX::LastPx(price));

            if (message.isSet(account)) {
                executionReport.setField(message.get(account));
            }

            try {
                FIX::Session::sendToTarget(executionReport, sessionID);
            } catch (FIX::SessionNotFound &) {}
        }

        void onMessage(const FIX50::NewOrderSingle &message, const FIX::SessionID &sessionID) override
        {
            FIX::Symbol symbol;
            FIX::Side side;
            FIX::OrdType ordType;
            FIX::OrderQty orderQty;
            FIX::Price price;
            FIX::ClOrdID clOrdID;
            FIX::Account account;

            message.get(ordType);

            if (ordType != FIX::OrdType_LIMIT) {
                throw FIX::IncorrectTagValue(ordType.getTag());
            }

            message.get(symbol);
            message.get(side);
            message.get(orderQty);
            message.get(price);
            message.get(clOrdID);

            FIX50::ExecutionReport executionReport = FIX50::ExecutionReport(
                FIX::OrderID(genOrderID()),
                FIX::ExecID(genExecID()),
                FIX::ExecType(FIX::ExecType_TRADE),
                FIX::OrdStatus(FIX::OrdStatus_FILLED),
                side,
                FIX::LeavesQty(0),
                FIX::CumQty(orderQty));

            executionReport.set(clOrdID);
            executionReport.set(symbol);
            executionReport.set(orderQty);
            executionReport.set(FIX::LastQty(orderQty));
            executionReport.set(FIX::LastPx(price));
            executionReport.set(FIX::AvgPx(price));

            if (message.isSet(account)) {
                executionReport.setField(message.get(account));
            }

            try {
                FIX::Session::sendToTarget(executionReport, sessionID);
            } catch (FIX::SessionNotFound &) {}
        }

        std::string genOrderID()
        {
            return std::to_string(++m_orderID);
        }

        std::string genExecID()
        {
            return std::to_string(++m_execID);
        }

        int m_orderID, m_execID;
    };
}

void wait() {
    std::cout << "Type Ctrl-C to quit" << std::endl;
    while (true) {
        FIX::process_sleep(1);
    }
}

int main([[maybe_unused]] int argc,
         [[maybe_unused]] char** argv)
{
    const std::vector<std::string_view> args(argv + 1, argv + argc);
    const std::string confFile {
        R"(/home/andtokm/Projects/FinanceTechnologyProjects/FixExperiments/acceptor_settings.conf)"
    };

    try
    {
        FIX::SessionSettings settings(confFile);
        Experiments::Application application;
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

    return EXIT_SUCCESS;
}
