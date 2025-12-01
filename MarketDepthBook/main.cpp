/**============================================================================
Name        : main.cpp
Created on  : 30.11.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description :
============================================================================**/

#include <vector>
#include <iostream>
#include <string_view>
#include <optional>
#include <format>
#include <iomanip>


#include "FlatMap.hpp"
#include "Testing.hpp"


namespace
{
    template<typename K, typename V, flat_map::SortOrder ordering = flat_map::SortOrder::Ascending>
    std::ostream& operator<<(std::ostream& stream, const flat_map::FlatMap<K, V, ordering>& map)
    {
        for (uint32_t idx = 0; idx < map.size(); ++idx) {
            stream << map.data()[idx].key << " => " << map.data()[idx].value << std::endl;
        }
        return stream;
    }
}


struct MarketDepthBook
{
    constexpr static size_t MaxDepth { 1'000 };
    using Price    = double;
    using Quantity = double;

    flat_map::FlatMap<Price, Quantity, flat_map::SortOrder::Ascending>   asks { MaxDepth };
    flat_map::FlatMap<Price, Quantity, flat_map::SortOrder::Descending>  bids { MaxDepth };

    void buyUpdate(const Price price, const Quantity quantity)
    {
        if (quantity != 0) {
            if (const auto result = bids.push(price, quantity)) {
                result->value = quantity;
            }
        }
        else {
            bids.erase(price);
        }
    }

    void askUpdate(const Price price, const Quantity quantity)
    {
        if (quantity != 0) {
            if (const auto result = asks.push(price, quantity)) {
                result->value = quantity;
            }
        }
        else {
            asks.erase(price);
        }
    }

    [[nodiscard]]
    std::optional<std::pair<Price, Quantity>> getBestBid() const noexcept
    {
        if (bids.size() == 0) {
            return std::nullopt;
        }
        const auto [price, quantity] = *(bids.front());
        return std::make_optional(std::make_pair(price, quantity));
    }

    [[nodiscard]]
    std::optional<std::pair<Price, Quantity>> getBestAsk() const noexcept
    {
        if (asks.size() == 0) {
            return std::nullopt;
        }
        const auto [price, quantity] = *(asks.front());
        return std::make_optional(std::make_pair(price, quantity));
    }

    // TODO:
    //  - bid()
    //  - ask()
    //  - spread()
    //  - number of asks / bids ?
};

namespace printer
{
    using namespace std::string_view_literals;

    static constexpr std::string_view red   = "\033[31m"sv;
    static constexpr std::string_view green = "\033[32m"sv;
    static constexpr std::string_view reset = "\033[0m"sv;
    static constexpr uint32_t tableWidth = 45;

    struct Printer
    {
        static void print(const MarketDepthBook& depthBook)
        {
            const uint16_t bidSize = depthBook.bids.size(), askSize = depthBook.asks.size();

            std::cout << std::string(tableWidth, '-') << "\n|";
            std::cout << green << "    Bids: " << std::format("{: ^9}", bidSize);
            std::cout << reset << "  |  ";
            std::cout << red   << "    Asks: " << std::format("{: ^9}", askSize);
            std::cout << reset << "|\n";

            std::cout << std::string(tableWidth, '-') << std::endl;
            for (uint32_t bIdx = 0, sIdx = 0; bIdx < bidSize || sIdx < askSize; ++bIdx, ++sIdx)
            {
                std::cout << "| ";

                if (bidSize > bIdx)
                {
                    const auto [price, quantity] = depthBook.bids.data()[bIdx];
                    std::cout << std::format("{:{}.{}}", price, 8, 6);
                    std::cout << " : ";
                    std::cout << std::format("{:{}.{}}", quantity, 8, 6);
                }
                else {
                    std::cout << std::string(21, ' ');
                }

                std::cout << " | ";

                if (askSize > sIdx)
                {
                    const auto [price, quantity] = depthBook.asks.data()[sIdx];
                    std::cout << std::format("{:{}.{}}", price, 8, 6);
                    std::cout << " : ";
                    std::cout << std::format("{:{}.{}}", quantity, 8, 6);
                }
                else {
                    std::cout << std::string(19, ' ');
                }

                std::cout << " |\n";
            }
            std::cout << std::string(tableWidth, '-') << std::endl;
        }
    };
}

namespace testing
{
    using namespace ::testing;

    void print(const MarketDepthBook& depthBook)
    {
        printer::Printer::print(depthBook);
    }

    enum class Side {
        Buy,
        Sell
    };

    struct DepthUpdate
    {
        double price { 0.0 };
        double quantity { 0.0 };
        Side side { Side::Buy };
    };

    void handleUpdate(MarketDepthBook& book,
                      const DepthUpdate& depthUpdate)
    {
        if (Side::Buy == depthUpdate.side) {
            book.buyUpdate(depthUpdate.price, depthUpdate.quantity);
        }
        else {
            book.askUpdate(depthUpdate.price, depthUpdate.quantity);
        }
    }

    void handleEvents(MarketDepthBook& book,
                      const std::vector<DepthUpdate>& events)
    {
        for (const auto& event : events) {
            handleUpdate(book, event);
        }
    }

    void test_Bid_Empty()
    {
        MarketDepthBook book;
        handleEvents(book, {});

        const std::optional<std::pair<double, double>> bestBid = book.getBestBid();
        AssertFalse(bestBid.has_value());
    }

    void test_Bid_SingleBuy()
    {
        MarketDepthBook book;
        handleEvents(book, {
            { 101.01, 10.01, Side::Buy },
        });

        const std::optional<std::pair<double, double>> bestBid = book.getBestBid();
        AssertTrue(bestBid.has_value());

        const auto [price, quantity] = bestBid.value();
        AssertEqual(101.01, price);
        AssertEqual(10.01, quantity);
    }

    void test_Bid_MultipleBuy()
    {
        MarketDepthBook book;
        handleEvents(book, {
            { 100.0, 10.01, Side::Buy },
            { 101.0, 11.01, Side::Buy },
            { 102.02, 12.07, Side::Buy },
        });

        const std::optional<std::pair<double, double>> bestBid = book.getBestBid();
        AssertTrue(bestBid.has_value());

        const auto [price, quantity] = bestBid.value();
        AssertEqual(102.02, price);
        AssertEqual(12.07, quantity);
    }

    void test_Bid_UpdateBB()
    {
        MarketDepthBook book;
        handleEvents(book, {
            { 100.0, 10.01, Side::Buy },
            { 101.0, 11.01, Side::Buy },
            { 102.02, 12.07, Side::Buy },
            { 101.6, 11.01, Side::Buy },
            { 102.02, 12.01, Side::Buy },

        });

        const std::optional<std::pair<double, double>> bestBid = book.getBestBid();
        AssertTrue(bestBid.has_value());

        const auto [price, quantity] = bestBid.value();
        AssertEqual(102.02, price);
        AssertEqual(12.01, quantity);
    }

    void test_Bid_Delete_BB()
    {
        MarketDepthBook book;
        handleEvents(book, {
            { 100.0, 10.01, Side::Buy },
            { 101.0, 11.01, Side::Buy },
            { 102.02, 12.07, Side::Buy },
            { 101.6, 11.011, Side::Buy },
            { 102.02, 0, Side::Buy },

        });

        const std::optional<std::pair<double, double>> bestBid = book.getBestBid();
        AssertTrue(bestBid.has_value());

        const auto [price, quantity] = bestBid.value();
        AssertEqual(101.6, price);
        AssertEqual(11.011, quantity);
    }

    void test_Bid_Delete_Non_BB()
    {
        MarketDepthBook book;
        handleEvents(book, {
            { 100.00, 10.01, Side::Buy },
            { 101.00, 11.01, Side::Buy },
            { 102.02, 12.07, Side::Buy },
            { 101.60, 11.011, Side::Buy }
        });

        const uint32_t bidsSize = book.bids.size();

        handleEvents(book, {
             { 101.00, 0, Side::Buy },
        });

        AssertEqual(bidsSize - 1, book.bids.size());


        const std::optional<std::pair<double, double>> bestBid = book.getBestBid();
        AssertTrue(bestBid.has_value());

        const auto [price, quantity] = bestBid.value();
        AssertEqual(102.02, price);
        AssertEqual(12.07, quantity);
    }

    void test_Ask_Empty()
    {
        MarketDepthBook book;
        handleEvents(book, {});

        const std::optional<std::pair<double, double>> bestAsk = book.getBestAsk();
        AssertFalse(bestAsk.has_value());
    }

    void test_Ask_SingleBuy()
    {
        MarketDepthBook book;
        handleEvents(book, {
            { 201.01, 20.02, Side::Sell },
        });

        const std::optional<std::pair<double, double>> bestAsk = book.getBestAsk();
        AssertTrue(bestAsk.has_value());

        const auto [price, quantity] = bestAsk.value();
        AssertEqual(201.01, price);
        AssertEqual(20.02, quantity);
    }

    void test_Ask_MultipleBuy()
    {
        MarketDepthBook book;
        handleEvents(book, {
            { 202.02, 21.01, Side::Sell },
            { 200.01, 20.01, Side::Sell },
            { 201.55, 22.02, Side::Sell },
        });

        const std::optional<std::pair<double, double>> bestAsk = book.getBestAsk();
        AssertTrue(bestAsk.has_value());

        const auto [price, quantity] = bestAsk.value();
        AssertEqual(200.01, price);
        AssertEqual(20.01, quantity);
    }

    void test_Ask_UpdateBA()
    {
        MarketDepthBook book;
        handleEvents(book, {
            { 202.02, 21.01, Side::Sell },
            { 200.01, 20.01, Side::Sell },
            { 201.55, 22.02, Side::Sell },
            { 200.01, 10.0123, Side::Sell },

        });
        const std::optional<std::pair<double, double>> bestAsk = book.getBestAsk();
        AssertTrue(bestAsk.has_value());

        const auto [price, quantity] = bestAsk.value();
        AssertEqual(200.01, price);
        AssertEqual(10.0123, quantity);
    }

    void test_Ask_Delete_BA()
    {
        MarketDepthBook book;
        handleEvents(book, {
            { 202.02, 21.01, Side::Sell },
            { 200.01, 20.01, Side::Sell },
            { 201.55, 22.02, Side::Sell },
            { 200.01, 0, Side::Sell },

        });
        const std::optional<std::pair<double, double>> bestAsk = book.getBestAsk();
        AssertTrue(bestAsk.has_value());

        const auto [price, quantity] = bestAsk.value();
        AssertEqual(201.55, price);
        AssertEqual(22.02, quantity);
    }

    void test_load()
    {
        MarketDepthBook book;

        double buyPrice = 100.0, sellPrice = 200.0;
        double buyQuantity = 10.0, sellQuantity = 10.0;
        for (int i = 0; i < 100; ++i)
        {
            handleUpdate(book, DepthUpdate { .price=buyPrice, .quantity=buyQuantity, .side= Side::Buy });
            buyPrice += 0.001;
            buyQuantity += 0.0001;

            handleUpdate(book, DepthUpdate { .price=sellPrice, .quantity=sellQuantity, .side= Side::Sell });
            sellPrice -= 0.001;
            sellQuantity -= 0.0001;
        }

        print(book);
    }
}


// TODO: BID
//  + Empty - no Best Buy
//  + Single Buy - check Best Buy
//  + Multiple Buy - Check BB
//  + Update Exising BB
//  + Delete Exising (BB) Buy - Check BB
//  + Delete Exising (non BB) Buy - Check BB

// TODO: ASK
//  + Empty - no Best Ask
//  + Single Ask - check Best Ask
//  + Multiple Ask - Check BA
//  + Update Exising BA
//  - Delete Exising (BA) Ask - Check BA
//  - Delete Exising (non BA) Ask - Check BA


int main([[maybe_unused]] const int argc,
         [[maybe_unused]] char** argv,
         [[maybe_unused]] char** env)
{
    const std::vector<std::string_view> args(argv + 1, argv + argc);

    testing::test_Bid_Empty();
    testing::test_Bid_SingleBuy();
    testing::test_Bid_MultipleBuy();
    testing::test_Bid_UpdateBB();
    testing::test_Bid_Delete_BB();
    testing::test_Bid_Delete_Non_BB();

    testing::test_Ask_Empty();
    testing::test_Ask_SingleBuy();
    testing::test_Ask_MultipleBuy();
    testing::test_Ask_UpdateBA();
    testing::test_Ask_Delete_BA();




    // testing::test_Ask_1();
    // testing::test_load();


    return EXIT_SUCCESS;
}
