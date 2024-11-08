/**============================================================================
Name        : main.cpp
Created on  : 24.28.2024
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Tests C++ project
============================================================================**/

#include <vector>
#include <iostream>
#include <string_view>

#include <memory>
#include <thread>

#include <pulsar/Client.h>
#include <nlohmann/json.hpp>

#include "Order.h"

#include <numeric>
#include <list>
#include <unordered_map>

#include <boost/container/flat_map.hpp>


namespace
{
    // const std::string pulsarHost { "pulsar://192.168.101.2:6650" };
    const std::string pulsarHost { "pulsar://0.0.0.0:6650" };

    const std::string markerCode { "BTC-USDT" };
    const std::string pretradePrefix { "persistent://OPNX-V1/PRETRADE-ME" };
    const std::string orderTopic { pretradePrefix + "/ORDER-IN-" + markerCode};

    template<typename T>
    struct Closer
    {
        void operator()(T *ptrObject) const {
            // std::cout << "Close && Delete" << std::endl;
            ptrObject->close();
            delete ptrObject;
        }
    };

    struct ClientDeleter
    {
        void operator()(pulsar::Client *ptrClient) const {
            ptrClient->shutdown();
            ptrClient->close();
            // delete ptrClient;
        }
    };

    struct ConsumerDeleter
    {
        void operator()(pulsar::Consumer *ptrConsumer) const
        {
            if (ptrConsumer->isConnected())
            {
                const std::string name {ptrConsumer->getConsumerName()};
                std::cout << "Closing Pulsar Consumer [Name: " << name << ", Topic: " << ptrConsumer->getTopic() << "]\n";
                ptrConsumer->unsubscribe();
                ptrConsumer->close();
            }
            delete ptrConsumer;
        }
    };

    struct ProducerDeleter
    {
        void operator()(pulsar::Producer *ptrProducer) const
        {
            if (ptrProducer->isConnected())
            {
                std::cout << "Closing Pulsar producer [Name: "
                          << ptrProducer->getProducerName() << ", Topic: " << ptrProducer->getTopic() << "]\n";
                ptrProducer->close();
            }
            delete ptrProducer;
        }
    };


    using PulsarClientrPtr  = std::shared_ptr<pulsar::Client>;
    using PulsarProducerPtr = std::unique_ptr<pulsar::Producer, ProducerDeleter>;
    using PulsarConsumerPtr = std::unique_ptr<pulsar::Consumer, ConsumerDeleter>;


    template<typename ... Args>
    [[nodiscard]]
    std::unique_ptr<pulsar::Client, ClientDeleter> makePulsarClient(Args&&... params)
    {
        return std::unique_ptr<pulsar::Client, ClientDeleter>(
                new pulsar::Client(std::forward<Args>(params)...), ClientDeleter{});
    }

    template<typename ... Args>
    [[nodiscard]]
    std::unique_ptr<pulsar::Producer, ProducerDeleter> makePulsarProducer(Args&&... params)
    {
        return std::unique_ptr<pulsar::Producer, ProducerDeleter>(
                new pulsar::Producer(std::forward<Args>(params)...), ProducerDeleter{});
    }

    template<typename ... Args>
    [[nodiscard]]
    PulsarConsumerPtr makePulsarConsumer(Args&&... params) {
        return PulsarConsumerPtr(new pulsar::Consumer(std::forward<Args>(params)...), ConsumerDeleter{});
    }


    [[nodiscard]]
    std::unique_ptr<pulsar::Client, ClientDeleter> getPulsarClient()
    {
        return makePulsarClient(pulsarHost);
    }

    [[nodiscard]]
    std::unique_ptr<pulsar::Client, ClientDeleter> getPulsarClient(int32_t threadsCount)
    {
        pulsar::ClientConfiguration conf;
        conf.setIOThreads(threadsCount);
        return makePulsarClient(pulsarHost, conf);
    }

    [[nodiscard]]
    std::unique_ptr<pulsar::Client, ClientDeleter> getPulsarClient(const pulsar::ClientConfiguration& configuration)
    {
        return makePulsarClient(pulsarHost, configuration);
    }


    struct PulsarClientGuard
    {
        pulsar::Client client;

        ~PulsarClientGuard() {
            client.close();
        }

        template<typename ... Args>
        auto createProducer(Args&&... params) -> decltype(auto)
        {
            return client.createProducer(std::forward<Args>(params)...);
        }

        template<typename ... Args>
        auto createProducerAsync(Args&&... params) -> decltype(auto)
        {
            return client.createProducerAsync(std::forward<Args>(params)...);
        }
    };

    struct PulsarProducerGuard
    {
        pulsar::Producer producer;

        ~PulsarProducerGuard() {
            producer.close();
        }

        template<typename ... Args>
        auto send(Args&&... params) -> decltype(auto)
        {
            return producer.send(std::forward<Args>(params)...);
        }

        template<typename ... Args>
        auto sendAsync(Args&&... params) -> decltype(auto)
        {
            return producer.sendAsync(std::forward<Args>(params)...);
        }
    };
}

namespace MatchingEngine
{
    using namespace Common;

    struct Trade
    {
        struct OrderInfo
        {
            Order::OrderID id = 0;
            Order::OrderID price = 0;
        };

        OrderInfo buyOrderInfo;
        OrderInfo sellOrderInfo;
        uint32_t  quantity = 0;

        static void addOrder(const Order& order, OrderInfo& orderInfo)
        {
            orderInfo.id = order.orderId;
            orderInfo.price = order.price;
        }

        Trade& setBuyOrder(const Order& order) {
            addOrder(order, buyOrderInfo);
            return *this;
        }

        Trade& setSellOrder(const Order& order) {
            addOrder(order, sellOrderInfo);
            return *this;
        }

        Trade& setQuantity(const uint32_t qnty) {
            quantity = qnty;
            return *this;
        }
    };


    struct Trades
    {
        std::vector<Trade> trades {};

        Trades()
        {
            trades.reserve(10'000'000);
        }

        Trade& addTrade() {
            return trades.emplace_back();
        }
    };


    /*
    struct Trades
    {
        // std::vector<Trade> trades {};
        Trade dummyTread;

        Trade& addTrade() {
            // return trades.emplace_back();
            return dummyTread;
        }
    };
     */


    struct OrderMatchingEngine
    {
        using OrderIter = typename std::list<Order>::iterator;
        using PriceOrderList = std::list<OrderIter>;
        using PriceOrderListPtr = PriceOrderList*;
        using PriceOrderListIter = typename PriceOrderList::iterator;

        struct ReferencesBlock
        {
            OrderIter orderIter;
            PriceOrderListIter priceOrderIter;
            PriceOrderListPtr priceLevelOrderList;
        };

        std::list<Order> orders {};
        std::unordered_map<Order::IDType, ReferencesBlock> orderByIDMap;

        // TODO: Test replace std::map --> boost::flat_map [std::list --> shall be pointer?]
        //       Since look performance of this lookup is more critical one

    #if 0
        std::map<Order::Price, PriceOrderListPtr, std::less<>> buyOrders;
            std::map<Order::Price, PriceOrderListPtr, std::greater<>> sellOrders;
    #else
        boost::container::flat_map<Order::Price, PriceOrderListPtr, std::less<>> buyOrders;
        boost::container::flat_map<Order::Price, PriceOrderListPtr, std::greater<>> sellOrders;
    #endif

        Trades trades;

        void processOrder(Order& order)
        {
            // TODO: Remove branching ???
            switch (order.action)
            {
                case OrderActionType::NEW:
                    return handleOrderNew(order);
                case OrderActionType::CANCEL:
                    return handleOrderCancel(order);
                case OrderActionType::AMEND:
                    return handleOrderAmend(order);
                default:
                    return;
            }
        }

        unsigned long long matchOrder(Order& order)
        {
            // TODO: Remove branching ???
            if (OrderSide::SELL == order.side) {
                matchOrder(order, buyOrders);
            } else {
                matchOrder(order, sellOrders);
            }

            // Return remaining quantity
            return order.quantity;
        }

        template<typename OrderSideMap>
        void matchOrder(Order& order, OrderSideMap& oppositeSideOrdersPriceMap)
        {
            auto matchedPriceLevelIter = oppositeSideOrdersPriceMap.lower_bound(order.price);
            while (oppositeSideOrdersPriceMap.end() != matchedPriceLevelIter && order.quantity > 0)
            {
                matchOrderList(order, matchedPriceLevelIter->second);
                ++matchedPriceLevelIter;
            }
        }

        void matchOrderList(Order& order,
                            PriceOrderList* matchedOrderList)
        {
            for (auto orderIter = matchedOrderList->begin(); matchedOrderList->end() != orderIter;)
            {
                Order& matchedOrder = *(*orderIter);

                // FIXME : Required performance improvements: cause of ~35% CPU usage
    #if 1
                Trade& trade = trades.addTrade();
                trade.setQuantity(std::min(matchedOrder.quantity,order.quantity));
                // TODO: Remove branching ???
                if (OrderSide::SELL == order.side) {
                    trade.setBuyOrder(matchedOrder).setSellOrder(order);
                } else {
                    trade.setBuyOrder(order).setSellOrder(matchedOrder);
                }

                std::cout << "Trade(Buy: {id: " << trade.buyOrderInfo.id  << ", price: " << trade.buyOrderInfo.price << "}, "
                          << "Sell: {id: " << trade.sellOrderInfo.id << ", price: " << trade.sellOrderInfo.price << "}, "
                          << "quantity: " << trade.quantity << ")\n";

    #endif

                if (order.quantity >= matchedOrder.quantity)
                {
                    order.quantity -= matchedOrder.quantity;
                    matchedOrder.quantity = 0;

                    /** Deleting order **/
                    orderByIDMap.erase(matchedOrder.orderId);
                    matchedOrderList->erase(orderIter++);
                } else {
                    matchedOrder.quantity -= order.quantity;
                    order.quantity = 0;
                    ++orderIter;
                    return;;
                }
            }
        }

        template<class Map>
        PriceOrderListPtr getOrderPriceList(Map& map, const Order::Price& price)
        {
            const auto [iter, inserted] = map.emplace(price, nullptr);
            if (inserted) {
                iter->second = new PriceOrderList ;
            }
            return iter->second;
        };

        void handleOrderNew(Order& order)
        {
            if (0 == matchOrder(order)) {
                return;
            }

            const auto [iterOrderMap, inserted] = orderByIDMap.emplace(order.orderId, ReferencesBlock{});
            if (inserted)
            {
                auto& [orderIter, priceOrderIter, priceLevelOrderList] = iterOrderMap->second;
                orderIter = orders.insert(orders.end(), order);
                // TODO: Remove branching ???
                priceLevelOrderList = (OrderSide::BUY == order.side) ? getOrderPriceList(buyOrders, order.price) :
                                      getOrderPriceList(sellOrders, order.price);
                priceOrderIter = priceLevelOrderList->insert(priceLevelOrderList->end(), orderIter);
            }
        }

        void handleOrderCancel(const Order& order)
        {
            if (const auto orderByIDIter = orderByIDMap.find(order.orderId);
                    orderByIDMap.end() != orderByIDIter)
            {
                auto& [orderIter, priceOrderIter, priceLevelOrderList] = orderByIDIter->second;
                priceLevelOrderList->erase(priceOrderIter);
                orders.erase(orderIter);
                orderByIDMap.erase(orderByIDIter);
            }
        }

        void handleOrderAmend(Order& order)
        {
            if (const auto orderByIDIter = orderByIDMap.find(order.orderId);
                    orderByIDMap.end() != orderByIDIter)
            {
                // TODO: Remove branching ???
                Order& orderOriginal = *(orderByIDIter->second.orderIter);
                if (orderOriginal.side != order.side) {
                    return;
                } else if (orderOriginal.price != order.price) {
                    handleOrderCancel(order);
                    handleOrderNew(order);
                } else if (orderOriginal.price == order.price) {
                    // TODO: update order parameters
                    orderOriginal.quantity = order.quantity;
                }
            }
        }

        void info(bool printTrades = true)
        {
            for (const auto& [orderId, orderIter]: orderByIDMap) {
                Order& orderOne = *orderIter.orderIter;
                Order& orderTwo = *(*orderIter.priceOrderIter);
                if (orderId != orderOne.orderId || orderId != orderTwo.orderId) {
                    std::cerr << "ERROR: ID: " << orderId << "!= " << orderOne.orderId << std::endl;
                }
            }

            auto printOrders = [](const auto& orderMap) {
                for (const auto& [price, ordersList]: orderMap) {
                    std::cout << "\tPrice: [" << price << "]" << std::endl;
                    for (const auto & orderIter: *ordersList) {
                        Common::printOrder(*orderIter);
                    }
                }
            };

            std::cout << "BUY:  " << std::endl; printOrders(buyOrders);
            std::cout << "SELL: " << std::endl; printOrders(sellOrders);

            std::cout << std::string(160, '=') << std::endl;
            /*
            if (!printTrades)
                return;
            for (const auto& trade: trades.trades)
            {
                std::cout << "Trade(Buy: {id: " << trade.buyOrderInfo.id  << ", price: " << trade.buyOrderInfo.price << "}, "
                          << "Sell: {id: " << trade.sellOrderInfo.id << ", price: " << trade.sellOrderInfo.price << "}, "
                          << "quantity: " << trade.quantity << ")\n";
            }*/
        }
    };
}


namespace Pulsar
{
    using namespace pulsar;

    const std::string topicNotifications { "persistent://public/default/notifications" };
    // const std::string topic2 { "non-persistent://OPNX-V1/PRETRADE-ME/ORDER-IN-BTC-USDT" };

    void printPulsarMessage(const pulsar::Message& msg)
    {
        std::cout << std::string(120, '=') << '\n'
                  << "getEventTimestamp: " << msg.getEventTimestamp() << "\n"
                  << "getMessageId: " << msg.getMessageId() << "\n"
                  //<< "getIndex: " << msg.() << "\n"
                  << "getPublishTimestamp: " << msg.getPublishTimestamp()
                  << std::endl;

        std::cout << std::string(120, '=') << '\n' << msg.getDataAsString() << '\n' << std::string(120, '=') << '\n';
    }

    void ConsumeMessages()
    {
        const std::unique_ptr<pulsar::Client, ClientDeleter> pulsarClient { getPulsarClient() };
        const std::unique_ptr<pulsar::Consumer, ConsumerDeleter> consumer { makePulsarConsumer() };

        pulsar::ConsumerConfiguration config;
        // config.setSubscriptionInitialPosition(pulsar::InitialPositionEarliest);

        Result result = pulsarClient->subscribe(topicNotifications, "consumer-1", config, *consumer);
        if (result != ResultOk) {
            std::cerr << "Failed to subscribe: " << result << std::endl;
            return;
        }

        Message msg;
        while (true)
        {
            const pulsar::Result receiveResult = consumer->receive(msg, 1'000);
            if (pulsar::Result::ResultOk == receiveResult)
            {
                printPulsarMessage(msg);
                consumer->acknowledge(msg);
            }
            else if (pulsar::Result::ResultTimeout == receiveResult)
            {
                std::cout << "Timeout" << std::endl;
            }
        }

        std::cout << "Finished consuming synchronously!" << std::endl;
    }

    void Consumer_Async()
    {
        std::unique_ptr<pulsar::Client, ClientDeleter> pulsarClient { getPulsarClient() };
        std::unique_ptr<pulsar::Consumer, ConsumerDeleter> consumer { makePulsarConsumer() };

        ConsumerConfiguration config;
        config.setSubscriptionInitialPosition(InitialPositionEarliest);

        config.setMessageListener([](pulsar::Consumer& consumer, const pulsar::Message& msg) {
            printPulsarMessage(msg);
        });

        const Result result  = pulsarClient->subscribe(topicNotifications, "consumer-1", config, *consumer);
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds (1U));
            std::cout << "Sleeping ..... \n";
        }
    }
}



namespace MatchingEngine_Pulsar
{
    using namespace pulsar;
    using namespace Common;
    using namespace std::string_literals;

    Common::Order orderFromJson(const std::string& jsonStr)
    {
        const nlohmann::json orderJson = nlohmann::json::parse(jsonStr);

        Common::Order order;
        order.orderId = orderJson["id"].get<decltype(order.orderId)>();
        order.marketId = orderJson["mid"].get<decltype(order.marketId)>();
        order.price = orderJson["p"].get<decltype(order.price)>();
        order.amount = orderJson["a"].get<decltype(order.amount)>();
        order.remainAmount = orderJson["ra"].get<decltype(order.remainAmount)>();
        order.accountId = orderJson["aid"].get<decltype(order.accountId)>();
        order.quantity = orderJson["q"].get<decltype(order.quantity)>();
        order.displayQuantity = orderJson["dq"].get<decltype(order.displayQuantity)>();
        order.remainQuantity = orderJson["rq"].get<decltype(order.remainQuantity)>();

        // order.timestamp = orderJson["t"].get<decltype(order.timestamp)>();
        // order.upperBound = orderJson["ub"].get<decltype(order.upperBound)>();
        // order.lowerBound = orderJson["lb"].get<decltype(order.lowerBound)>();
        // order.parentAccountId = orderJson["paid"].get<decltype(order.parentAccountId)>();
        // order.timestamp = orderJson["t"].get<decltype(order.timestamp)>();

        std::string value = orderJson["ac"].get<std::string>();
        {
            if ("NEW"s == value)
                order.action = OrderActionType::NEW;
            else if ("AMEND"s == value)
                order.action = OrderActionType::AMEND;
            else if ("CANCEL"s == value)
                order.action = OrderActionType::CANCEL;
            else if ("RECOVERY"s == value)
                order.action = OrderActionType::RECOVERY;
            else if ("RECOVERY_END"s == value)
                order.action = OrderActionType::RECOVERY_END;
        }

        value = orderJson["mt"].get<std::string>();
        {
            if ("MAKER"s == value)
                order.matchedType = OrderMatchedType::MAKER;
            else
                order.matchedType = OrderMatchedType::TAKER;
        }

        value = orderJson["s"].get<std::string>();
        {
            if ("BUY"s == value)
                order.side = OrderSide::BUY;
            else
                order.side = OrderSide::SELL;
        }

        value = orderJson["st"].get<std::string>();
        {
            if ("OPEN"s == value)
                order.status = OrderStatusType::OPEN;
            else if ("PARTIAL_FILL"s == value)
                order.status = OrderStatusType::PARTIAL_FILL;
            else if ("FILLED"s == value)
                order.status = OrderStatusType::FILLED;
            else if ("CANCELED_BY_USER"s == value)
                order.status = OrderStatusType::CANCELED_BY_USER;
            else if ("CANCELED_BY_MARKET_ORDER_NOT_FULL_MATCHED"s == value)
                order.status = OrderStatusType::CANCELED_BY_MARKET_ORDER_NOT_FULL_MATCHED;
            else if ("CANCELED_BY_MARKET_ORDER_NOTHING_MATCH"s == value)
                order.status = OrderStatusType::CANCELED_BY_MARKET_ORDER_NOTHING_MATCH;
            else if ("CANCELED_ALL_BY_IOC"s == value)
                order.status = OrderStatusType::CANCELED_ALL_BY_IOC;
            else if ("CANCELED_PARTIAL_BY_IOC"s == value)
                order.status = OrderStatusType::CANCELED_PARTIAL_BY_IOC;
            else if ("CANCELED_BY_FOK"s == value)
                order.status = OrderStatusType::CANCELED_BY_FOK;
            else if ("CANCELED_BY_MAKER_ONLY"s == value)
                order.status = OrderStatusType::CANCELED_BY_MAKER_ONLY;
            else if ("CANCELED_BY_AMEND"s == value)
                order.status = OrderStatusType::CANCELED_BY_AMEND;
            else if ("CANCELED_BY_SELF_TRADE_PROTECTION"s == value)
                order.status = OrderStatusType::CANCELED_BY_SELF_TRADE_PROTECTION;
            else if ("REJECT_CANCEL_ORDER_ID_NOT_FOUND"s == value)
                order.status = OrderStatusType::REJECT_CANCEL_ORDER_ID_NOT_FOUND;
            else if ("REJECT_AMEND_ORDER_ID_NOT_FOUND"s == value)
                order.status = OrderStatusType::REJECT_AMEND_ORDER_ID_NOT_FOUND;
            else if ("REJECT_DISPLAY_QUANTITY_ZERO"s == value)
                order.status = OrderStatusType::REJECT_DISPLAY_QUANTITY_ZERO;
            else if ("REJECT_DISPLAY_QUANTITY_LARGER_THAN_QUANTITY"s == value)
                order.status = OrderStatusType::REJECT_DISPLAY_QUANTITY_LARGER_THAN_QUANTITY;
            else if ("REJECT_QUANTITY_AND_AMOUNT_ZERO"s == value)
                order.status = OrderStatusType::REJECT_QUANTITY_AND_AMOUNT_ZERO;
            else if ("REJECT_LIMIT_ORDER_WITH_MARKET_PRICE"s == value)
                order.status = OrderStatusType::REJECT_LIMIT_ORDER_WITH_MARKET_PRICE;
            else if ("REJECT_MATCHING_ENGINE_RECOVERING"s == value)
                order.status = OrderStatusType::REJECT_MATCHING_ENGINE_RECOVERING;
            else if ("REJECT_STOP_CONDITION_IS_NONE"s == value)
                order.status = OrderStatusType::REJECT_STOP_CONDITION_IS_NONE;
            else if ("REJECT_STOP_TRIGGER_PRICE_IS_NONE"s == value)
                order.status = OrderStatusType::REJECT_STOP_TRIGGER_PRICE_IS_NONE;
            else if ("REJECT_QUANTITY_AND_AMOUNT_LARGER_ZERO"s == value)
                order.status = OrderStatusType::REJECT_QUANTITY_AND_AMOUNT_LARGER_ZERO;
            else if ("REJECT_PRICE_LESS_ZERO"s == value)
                order.status = OrderStatusType::REJECT_PRICE_LESS_ZERO;
            else if ("REJECT_ORDER_QUANTITY_GREATER_THAN_MAX"s == value)
                order.status = OrderStatusType::REJECT_ORDER_QUANTITY_GREATER_THAN_MAX;
            else if ("REJECT_ORDER_AMOUNT_GREATER_THAN_MAX"s == value)
                order.status = OrderStatusType::REJECT_ORDER_AMOUNT_GREATER_THAN_MAX;
            else if ("REJECT_ORDER_TIMEOUT"s == value)
                order.status = OrderStatusType::REJECT_ORDER_TIMEOUT;
            else if ("REJECT_MAXIMUM_OPEN_ORDERS_LIMIT_REACHED"s == value)
                order.status = OrderStatusType::REJECT_MAXIMUM_OPEN_ORDERS_LIMIT_REACHED;
        }

        /*value = orderJson["stc"].get<std::string>();
        {
            if ("BUY"s == value)
                order.side = OrderSide::BUY;
            else
                order.side = OrderSide::SELL;
        }*/

        value = orderJson["stp"].get<std::string>();
        {
            if ("NONE"s == value)
                order.selfTradeProtectionType = SelfTradeProtectionType::STP_NONE;
            else if ("EXPIRE_TAKER"s == value)
                order.selfTradeProtectionType = SelfTradeProtectionType::STP_TAKER;
            else if ("EXPIRE_MAKER"s == value)
                order.selfTradeProtectionType = SelfTradeProtectionType::STP_MAKER;
            else if ("EXPIRE_BOTH"s == value)
                order.selfTradeProtectionType = SelfTradeProtectionType::STP_BOTH;
        }

        value = orderJson["tc"].get<std::string>();
        {
            if ("GTC"s == value)
                order.timeCondition = OrderTimeCondition::GTC;
            else if ("IOC"s == value)
                order.timeCondition = OrderTimeCondition::IOC;
            else if ("FOK"s == value)
                order.timeCondition = OrderTimeCondition::FOK;
            else if ("MAKER_ONLY"s == value)
                order.timeCondition = OrderTimeCondition::MAKER_ONLY;
            else if ("MAKER_ONLY_REPRICE"s == value)
                order.timeCondition = OrderTimeCondition::MAKER_ONLY_REPRICE;
        }

        value = orderJson["ty"].get<std::string>();
        {
            if ("LIMIT"s == value)
                order.type = OrderType::LIMIT;
            else if ("MARKET"s == value)
                order.type = OrderType::MARKET;
        }

        return order;
    }

    void consumeOrders()
    {
        const std::unique_ptr<pulsar::Client, ClientDeleter> pulsarClient { getPulsarClient() };
        const std::unique_ptr<pulsar::Consumer, ConsumerDeleter> consumer { makePulsarConsumer() };

        const ConsumerConfiguration config;
        // config.setSubscriptionInitialPosition(pulsar::InitialPositionEarliest);

        Result result = pulsarClient->subscribe(orderTopic, "consumer-1", config, *consumer);
        if (result != ResultOk) {
            std::cerr << "Failed to subscribe: " << result << std::endl;
            return;
        }

        MatchingEngine::OrderMatchingEngine engine;
        Message msg;
        while (true)
        {
            const pulsar::Result receiveResult = consumer->receive(msg, 1'000);
            if (pulsar::Result::ResultOk == receiveResult)
            {
                Order order = MatchingEngine_Pulsar::orderFromJson(msg.getDataAsString());
                engine.processOrder(order);
                consumer->acknowledge(msg);
            }
            else if (pulsar::Result::ResultTimeout == receiveResult) {
                continue;
            }
        }
    }
}


int main([[maybe_unused]] const int argc,
         [[maybe_unused]] char** argv)
{
    const std::vector<std::string_view> args(argv + 1, argv + argc);

    // MatchingEngine::TestAll();
    // Pulsar::ConsumeMessages();

    MatchingEngine_Pulsar::consumeOrders();

    return EXIT_SUCCESS;
}
