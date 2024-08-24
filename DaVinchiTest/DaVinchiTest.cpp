/**============================================================================
Name        : DaVinchiTest.h
Created on  : 17.04.2022
Author      : Tokmakov Andrei
Version     : 1.0
Copyright   : Your copyright notice
Description : DaVinchiTest
============================================================================**/

#include "DaVinchiTest.h"


#include <iostream>
#include <fstream>
#include <vector>
#include <string_view>
#include <map>
#include <unordered_map>
#include <charconv>
#include <cstdint>

/*
    I = Insert / new order - his is a new order added to the book; it will have a new/unique order-id.
    C = Cancel / delete order: The order with the given order-id is to be removed from the book.
    A = Amend / modify order :The order with the given order-id is to be changed to the new volume and/or price.

    timestamp;symbol;order-id;operation;side;volume;price

    For example:
    14:17:21.877391;DVAM1;00000001;I;BUY;100;12.5
    14:17:22.123523;DVAM1;00000002;I;SELL;37;13.5
    14:17:22.343883;DVAM1;00000001;A;BUY;100;12.7
    14:17:24.737292;DVAM1;00000003;I;SELL;37;13.3
    14:17:24.893811;DVAM1;00000004;I;BUY;55;12.7
    14:17:25.883711;DVAM1;00000002;C;SELL;37;13.5
 */


namespace DaVinchiTest {
    inline constexpr std::string_view dataFilePath {
            R"(/home/andtokm/DiskS/ProjectsUbuntu/CppProjects/FinTechMarketProjects/data/da_vinchi_orders.dat)"};

    inline constexpr std::string_view dataFilePathTest {
            R"(/home/andtokm/DiskS/ProjectsUbuntu/CppProjects/FinTechMarketProjects/data/orders_test1.dat)"};


    // TODO: Move str::string --> to func???
    // Run tests
    [[nodiscard]]
    static std::vector<std::string> split(const std::string &str) {
        std::vector<std::string> params{};
        params.reserve(7);
        size_t pos = 0, prev = 0;
        while ((pos = str.find(';', prev)) != std::string::npos) {
            params.emplace_back(str, prev, pos - prev);
            prev = pos + 1;
        }
        params.emplace_back(str, prev, str.length() - prev);
        return params;
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
        std::string timestamp {}; // TODO: to int
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

        // TODO: Concepts: default_ctor, floating_point, std::itegral
        template<typename T>
        [[nodiscard]]
        static T extractType(std::string_view str) {
            T result {};
            std::from_chars(str.data(), str.data() + str.size(), result);
            return result;
        }
    };

    struct SymbolOrders final
    {
        using OrdersList = std::map<double, Order>;
        using OrderIter = typename OrdersList::iterator;

        OrdersList buyOrders {};  // TODO: Sort increasing
        OrdersList sellOrders {}; // TODO: Sort decreasing

        // SymbolOrders() { std::cout << "SymbolOrders" << std::endl; }

        [[nodiscard]]
        inline OrdersList& getOrders(OrderSide side) noexcept {
            return (OrderSide::Buy == side) ? buyOrders : sellOrders;
        }

        [[nodiscard]]
        inline const OrdersList& getOrders(OrderSide side) const noexcept {
            return (OrderSide::Buy == side) ? buyOrders : sellOrders;
        }

        [[nodiscard]]
        inline bool empty() const noexcept {
            return buyOrders.empty() && sellOrders.empty();
        }

        [[nodiscard]]
        inline size_t size() const noexcept {
            return buyOrders.size() + sellOrders.size();
        }
    };

    // TODO: Move str::string --> to func???
    Order parseOrder(const std::string& rawOrder) {
        std::vector<std::string> params = split(rawOrder);

        Order order { // TODO: move ???
            std::move(params[0]),
            std::move(params[1]),
            std::move(params[2]),
            Order::extractType<uint32_t>(params[5]),
            Order::extractType<float>(params[6]),
            Order::getOperation(params[3]),
            Order::getSide(params[4])
        };

        return order;
    }

    // TODO: Debug
    void printOrder(const Order &order) {
        std::cout << order.timestamp << "|" << order.symbol << "|" << order.id << "|";

        if (Operation::Insert == order.operation) {
            std::cout << 'I' << '|';
        } else if (Operation::Cancel == order.operation) {
            std::cout << 'C' << '|';
        } else if (Operation::Amend == order.operation) {
            std::cout << 'A' << '|';
        }

        if (OrderSide::Buy == order.side) {
            std::cout << "BUY" << '|';
        } else {
            std::cout << "SELL" << '|';
        }

        std::cout << order.volume << "|" << order.price << std::endl;
    }



    struct Book
    {
        std::unordered_map<std::string, SymbolOrders> orderBook;
        std::unordered_map<std::string, SymbolOrders::OrderIter> ordersById;

        // size_t debugCounter = 0;

        // TODO: Move string??
        void processRawOrder(const std::string& rawOrder)
        {
            // TODO: std::move it to book
            Order order = parseOrder(rawOrder);

            // TODO: of use [] operator
            const auto [iter, ok] = orderBook.try_emplace(order.symbol);

            // TODO: Handle 'Cancel' and 'Delete/Amend'

            SymbolOrders::OrdersList& orders = iter->second.getOrders(order.side);

            if (Operation::Insert == order.operation) {
                orders[order.price].volume += order.volume;
            }

            std::cout << rawOrder << std::endl;
            printOrder(order);
        }

        // TODO: Move string??
        void processRawOrder2(const std::string& rawOrder) {
            // TODO: std::move it to book
            Order order = parseOrder(rawOrder);

            SymbolOrders &symbolOrders = orderBook[order.symbol];
            SymbolOrders::OrdersList& orders = symbolOrders.getOrders(order.side);

            /*
            auto  iter = orders.find(order.price);
            if (Operation::Insert == order.operation)
            {
                if (orders.end() == iter) {
                    orders.emplace(order.price, std::move(order));
                } else {
                    std::cout << "Duplicate price "; printOrder(order);
                    for (const auto & [k, v]: orders) {
                        std::cout << "            " << k << "  "; printOrder(v);
                    }
                }
            }*/

            if (Operation::Cancel == order.operation)
            {
                auto iter = ordersById.find(order.id);
                if (ordersById.end() == iter) {
                    std::cout << "ERROR Cancel" << std::endl;
                    printOrder(order);
                  }

                orders.erase(iter->second);
                ordersById.erase(iter);
            }

            if (Operation::Amend == order.operation)
            {
                auto orderIter = ordersById.find(order.id);
                if (ordersById.end() == orderIter) {
                    std::cout << "ERROR Amend" << std::endl;
                    printOrder(order);
                }

                // Price has changed
                if (order.price != orderIter->second->second.price) {
                    orders.erase(orderIter->second);
                    auto [iter, ok] = orders.try_emplace(order.price, std::move(order));
                    orderIter->second = iter;
                }
                else {
                    orderIter->second->second.volume = order.volume;
                }
            }


            if (Operation::Insert == order.operation)
            {
                // auto [iter, ok] = orders.try_emplace(order.price, std::move(order));

                auto [iter, ok] = orders.try_emplace(order.price);
                if (ok) {
                    ordersById[order.id] = iter; // TODO: change to emplace
                    iter->second = std::move(order);
                    // debugCounter++;
                }
                else {
                    std::cout << "Duplicate price "; printOrder(order);
                    for (const auto & [k, v]: orders) {
                        std::cout << "            " << k << "  "; printOrder(v);
                    }
                }
            }
        }

        void readOrders(std::string_view path)
        {
            std::string line;
            if (std::fstream file = std::fstream(path.data()); file.is_open() && file.good()) {
                while (std::getline(file, line)) {
                    // processRawOrder(line); // TODO: Move
                    processRawOrder2(line); // TODO: Move
                }
            }
        }

        // TODO: ----------------- Delete -----------------------
        void printBook() {
            for (const auto& [symbol, orders]: orderBook) {
                std::cout << "------------------- " << symbol << "------------------------------\n";

                const SymbolOrders::OrdersList& sellOrders = orders.sellOrders;
                if (!sellOrders.empty()) {
                    std::cout << "Sell orders:\n";
                    for (const auto &[price, order]: sellOrders) {
                        std::cout << "   " << price << " <-> " << order.volume << std::endl;
                    }
                }

                const SymbolOrders::OrdersList& buyOrders = orders.buyOrders;
                if (!buyOrders.empty()) {
                    std::cout << "Buy orders:\n";
                    for (const auto &[price, order]: buyOrders) {
                        std::cout << "   " << price << " <-> " << order.volume << std::endl;
                    }
                }
            }
        }

        // TODO: ----------------- Delete -----------------------
        void info() {
            size_t ordersCount = 0;
            for (const auto& [symbol, orders]: orderBook) {
                ordersCount += orders.size();
            }
            std::cout << "ordersCount = " << ordersCount << std::endl;
            std::cout << "ordersCount = " << ordersById.size() << " by ID"<< std::endl;
            // std::cout << "ordersCount = " << debugCounter << std::endl;
        }
    };
}

void DaVinchiTest::TestAll()
{
    Book book;

    book.readOrders(dataFilePathTest);
    book.printBook();
    book.info();


    /*
    std::cout << sizeof(Order) << std::endl;                    // 112
    std::cout << sizeof(SymbolOrders::OrdersList) << std::endl; // 48
    std::cout << sizeof(SymbolOrders) << std::endl;             // 96
    */
}
