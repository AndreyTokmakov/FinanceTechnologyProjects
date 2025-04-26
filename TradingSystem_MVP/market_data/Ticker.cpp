/**============================================================================
Name        : Ticker.cpp
Created on  : 26.04.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Ticker.cpp
============================================================================**/

#include "Ticker.h"
#include "format"


namespace market_data
{
    std::ostream& operator<<(std::ostream& stream, const Ticker& ticker)
    {
        stream << std::format("Ticker(\n\teventTime: {},\n\tsymbol: {},"
            "\n\tpriceChange: {},\n\tpriceChangePercent: {},\n\tlastPrice: {},\n\tlastQuantity: {},\n\topenPrice: {},"
            "\n\thighPrice: {},\n\tlowPrice: {},\n\ttotalTradedVolume: {},\n\ttotalTradedBaseAssetVolume: {},"
            "\n\tfirstTradeId: {},\n\tlastTradeId: {},\n\ttotalTradesNumber: {}\n)",
            ticker.eventTime, ticker.symbol, ticker.priceChange, ticker.priceChangePercent, ticker.lastPrice,
            ticker.lastQuantity, ticker.openPrice, ticker.highPrice, ticker.lowPrice, ticker.totalTradedVolume,
            ticker.totalTradedBaseAssetVolume, ticker.firstTradeId, ticker.lastTradeId, ticker.totalTradesNumber
        );
        return stream;
    }
}

