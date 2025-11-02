/**============================================================================
Name        : DemoTwo.cpp
Created on  :
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description :
============================================================================**/

#include <string>
#include <cstdint>
#include <array>
#include <unordered_map>
#include <utility>
#include <vector>
#include <atomic>
#include <mutex>
#include <functional>


enum class EventType : uint8_t {
    TRADE,
    TICKER,
    BOOK_UPDATE
};

enum class Exchange : uint8_t {
    BINANCE,
    BYBIT,
    OKX,
    COINBASE
};

struct Symbol
{
    std::string base;
    std::string quote;

    [[nodiscard]]
    std::string str() const { return base + "/" + quote; }
};

struct Trade
{
    Exchange exchange;
    Symbol symbol;
    double price;
    double qty;
    int64_t ts;    // epoch ms
    bool isBuyerMaker;
};

struct Ticker
{
    Exchange exchange;
    Symbol symbol;
    double bestBid;
    double bestAsk;
    double last;
    double volume24h;
    int64_t ts;
};

// ---------- Normalized OrderBook (optional, partial) ----------
struct OrderBookLevel {
    double price;
    double qty;
};

struct OrderBook
{
    Exchange exchange;
    Symbol symbol;
    std::vector<OrderBookLevel> bids;
    std::vector<OrderBookLevel> asks;
    int64_t ts;
};

// ---------- Normalized MarketEvent ----------
struct MarketEvent
{
    EventType type;
    union {
        Trade trade;
        Ticker ticker;
        OrderBook orderbook;
    };

    explicit MarketEvent(Trade  t) : type(EventType::TRADE), trade(std::move(t)) {}
    explicit MarketEvent(Ticker  t) : type(EventType::TICKER), ticker(std::move(t)) {}
    explicit MarketEvent(OrderBook  ob) : type(EventType::BOOK_UPDATE), orderbook(std::move(ob)) {}
};

struct IMarketFeed
{
    virtual ~IMarketFeed() = default;
    virtual void start() = 0;
};

struct MarketDispatcher
{
    using Callback = std::function<void(const MarketEvent&)>;

    void subscribe(Callback cb)
    {
        std::lock_guard<std::mutex> lock(mtx_);
        callbacks_.push_back(std::move(cb));
    }

    void publish(const MarketEvent& e)
    {
        std::lock_guard<std::mutex> lock(mtx_);
        for (auto& cb : callbacks_)
            cb(e);
    }

private:
    std::mutex mtx_;
    std::vector<Callback> callbacks_;
};
