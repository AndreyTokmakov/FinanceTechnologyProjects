
#include "DaVinchiTest_LinkedList.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <string_view>
#include <list>
#include <unordered_map>
#include <charconv>
#include <chrono>

namespace TestAssignmentLocal
{
    [[nodiscard]]
    static std::array<std::string, 7> splitOrder(std::string_view str) {
        std::array<std::string, 7> parts {};
        size_t pos = 0, prev = 0, idx = 0;
        while ((pos = str.find(';', prev)) != std::string::npos) {
            parts[idx++].assign(str, prev, pos - prev);
            prev = pos + 1;
        }
        parts[idx++].assign(str, prev, str.length() - prev);
        return parts;
    }

    enum class OrderSide : char {
        Buy,
        Sell
    };

    enum class Operation : char {
        Insert = 'I',
        Cancel = 'C',
        Amend = 'A'
    };

    struct Order final {
        std::string timestamp {};
        std::string symbol {};
        std::string id {};
        uint32_t volume {0};
        float price {0};
        Operation operation {Operation::Insert};
        OrderSide side {OrderSide::Buy};

        [[nodiscard]]
        static Operation getOperation(std::string_view op) noexcept {
            return static_cast<Operation>(op.front());
        }

        [[nodiscard]]
        static OrderSide getSide(std::string_view method) noexcept {
            return "BUY" == method ? OrderSide::Buy : OrderSide::Sell;
        }

        template<typename T>
        [[nodiscard]]
        static T extractType(std::string_view str) {
            T result {};
            std::from_chars(str.data(), str.data() + str.size(), result);
            return result;
        }

        static bool compareByVolume(const Order& x, const Order& y) {
            return x.volume > y.volume ;
        };
    };

    struct SymbolOrders final
    {
        using OrdersList = std::list<Order>;
        using OrderIter = typename OrdersList::iterator;

        OrdersList buyOrders {};
        OrdersList sellOrders {};

        [[nodiscard]]
        inline OrdersList& getOrders(OrderSide side) noexcept {
            return (OrderSide::Buy == side) ? buyOrders : sellOrders;
        }

        [[nodiscard]]
        inline size_t size() const noexcept {
            return buyOrders.size() + sellOrders.size();
        }
    };

    Order parseOrder(const std::string& rawOrder) {
        std::array<std::string, 7> params = splitOrder(rawOrder);
        return {
                std::move(params[0]),
                std::move(params[1]),
                std::move(params[2]),
                Order::extractType<uint32_t>(params[5]),
                std::strtof(params[6].data(), nullptr),
                Order::getOperation(params[3]),
                Order::getSide(params[4])
        };
    }

    struct SimpleOrderBook
    {
        std::unordered_map<std::string, SymbolOrders> orderBook;
        std::unordered_map<std::string, SymbolOrders::OrderIter> ordersById;

        void processOrder(const std::string& rawOrder)
        {
            Order order = parseOrder(rawOrder);
            SymbolOrders &symbolOrders = orderBook[order.symbol];
            SymbolOrders::OrdersList& orders = symbolOrders.getOrders(order.side);

            if (Operation::Insert == order.operation)
            {
                const auto [iter, _] = ordersById.try_emplace(order.id);
                iter->second = orders.insert(orders.end(), std::move(order));
            }
            else
            {
                if (const auto orderIter = ordersById.find(order.id); ordersById.end() != orderIter) {
                    if (Operation::Cancel == order.operation) {
                        orders.erase(orderIter->second);
                        ordersById.erase(orderIter);
                    } else if (Operation::Amend == order.operation) {
                        orderIter->second->price = order.price;
                        orderIter->second->volume = order.volume;
                    }
                } else {
                    // Cancel/Amend order received with ID that has already been removed or has not yet been added
                    std::cerr  << "Invalid order ID received. ID: " << order.id;
                }
            }
        }

        void readAndProcessOrders(std::string_view path)
        {
            if (std::fstream file = std::fstream(path.data()); file.is_open() && file.good()) {
                std::string line;
                while (std::getline(file, line)) {
                    processOrder(line);
                }
            }
        }

        void OrderCounts() const {
            for (const auto& [symbol, orders]: orderBook) {
                std::cout << symbol << " : " << orders.size() << std::endl;
            }
        }

        void BiggestBuyOrders(const std::string& symbol) const
        {
            const auto iter = orderBook.find(symbol);
            if (orderBook.end() == iter)
                return;

            constexpr size_t topSize {3};
            const SymbolOrders::OrdersList& buyOrders { iter->second.buyOrders };
            const size_t actualSize = buyOrders.size();
            auto endIt = buyOrders.begin();
            std::advance(endIt, std::min(actualSize, topSize));

            std::vector<Order> topOrders;
            for (auto it = buyOrders.begin(); it != endIt; ++it)
                topOrders.push_back(*it);

            for (auto it = endIt; it != buyOrders.end(); ++it) {
                std::make_heap(topOrders.begin(), topOrders.end(),  Order::compareByVolume);
                if (Order::compareByVolume(*it, topOrders.front()))
                    topOrders.front() = *it;
            }

            for (const Order& order: topOrders) {
                std::cout << order.timestamp << ";" << order.symbol << ";" << order.id << ";"
                          << order.volume << ";" << order.price << std::endl;
            }
        }

        void BestSellAtTime(const std::string& symbol,
                            const std::string& timeStr) const
        {
            const auto iter = orderBook.find(symbol);
            if (orderBook.end() == iter)
                return;

            std::pair<float, uint32_t> bestSellPrice {0.0f, 0};
            const SymbolOrders::OrdersList& sellOrders { iter->second.sellOrders };
            std::for_each(sellOrders.cbegin(), sellOrders.cend(), [&](const Order& order) {
                if (order.timestamp.starts_with(timeStr) && order.price > bestSellPrice.first) {
                    bestSellPrice = {order.price, order.volume};
                }
            });

            if (bestSellPrice.second) {
                std::cout << bestSellPrice.first << ", " << bestSellPrice.second << std::endl;
            }
        }
    };
}

void DaVinchiTest_LinkedList::TestAll()
{
    TestAssignmentLocal::SimpleOrderBook book;
    book.readAndProcessOrders("orders coding test developer.dat");

    book.OrderCounts();
    book.BiggestBuyOrders("TEST8");
    book.BestSellAtTime("TEST8", "15:38");
}
