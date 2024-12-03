
// TODO: Unit tests && Test Methods
//  + Проверка BID
//  + Проверка BID - пустой список BID
//  + Проверка BID - пустой список BID | SELL's only
//  + Проверка BID - после Cancel
//  - Проверка BID - после Amend
//  - Проверка BID - после добавления Order-ов с одной ценой
//  - Проверка BID - после Trade
//  -
//  + Проверка ASK
//  + Проверка ASK - пустой список ASK
//  + Проверка ASK - пустой список ASK | BUY's only
//  + Проверка ASK - после Cancel
//  - Проверка ASK - после Amend
//  - Проверка ASK - после добавления Order-ов с одной ценой
//  - Проверка ASK - после Trade
//  -
//  - Проверка количества PriceLevels BID
//  - Проверка количества PriceLevels ASK
//  - Проверка количества Order-ов в PriceLevel - BID
//  - Проверка количества Order-ов в PriceLevel - ASK
//  - Проверка Qunantiry  PriceLevel - BID
//  - Проверка Qunantiry  PriceLevel - ASK
//  -
//  + Проверка Cancel: Cancel 1 BUY
//  + Проверка Cancel: Cancel 1 SELL
//  + Проверка Cancel: Cancel 100 BUY
//  + Проверка Cancel: Cancel 100 SELL
//  + Проверка Cancel: Cancel Not-existing SELL
//  + Проверка Cancel: Cancel Not-existing BUY
//  - Проверка Cancel: Check total order count
//  - Проверка Cancel: Check BID's orders count ( SELL cancel order )
//  - Проверка Cancel: Check ASK's orders count ( BUY cancel order )
//  - Проверка Cancel: Check price level deleted
//  -
//  - Проверка Amend:
//  + Amend - Single BUY - new Quantity, same Price
//  + Amend - Single SELL - new Quantity, same Price
//  + Amend - Single BUY - new Quantity, new Price
//  + Amend - Single SELL - new Quantity, new Price
//  - Amend - Single SELL + BUY

========================================================================================================
                            BUY
========================================================================================================

--  Empty
        Проверить bestBuy: Empty
        Проверить bestSell: Empty

--  BUY Order:              [Test_getBestBuyOrder]
        Послать несколько Order-ов
        Проверить bestBuy: Price, Quantity

--  BUY Orders:
        Послать несколько SELL Order-ов, 0 BUY order-ов
        Проверить bestSell: Empty

--  BUY Orders:
        5 раз
            Отправить BUY Order
            Проверить bestBuy: Price == Price нового Order-а, Quantity

--  BUY Orders:
       5 BUY Order
       Проверить PriceList: quantity, количество Order-ов в PriceList::quantity

========================================================================================================
                            SELL
========================================================================================================

--  SELL Order:
        Послать несколько Order-ов
        Проверить bestSell: Price, Quantity

--  SELL Orders:
        Послать несколько SELL Order-ов, 0 SELL order-ов
        Проверить bestSell: Empty

--  SELL Orders:
        5 раз
            Отправить SELL Order
            Проверить bestSell: Price == Price нового Order-а, Quantity

--  SELL Orders:
       5 SELL Order
       Проверить PriceList: quantity, количество Order-ов в PriceList::quantity

========================================================================================================
                            AMEND
========================================================================================================

--  Basic: 1 x BUY : Amend change Quantity
--  Basic: 1 x SELL : Amend change Quantity
--  Basic: 1 x BUY : Amend change Price
--  Basic: 1 x SELL : Amend change Price
--  Basic: 1 x BUY : Amend change Price && Quantity
--  Basic: 1 x SELL : Amend change Price && Quantity
--  Wrong Side
--  Wrong Action
--  Missing ID

========================================================================================================
                            CANCEL
========================================================================================================

--  Basic: 1 x BUY : & Cancel
--  Basic: 1 x SELL : & Cancel
--  Cancel Wrong ID
--  Cancel Wrong SIDE

--  BUY Orders: & Cancel
        Отправить BUY Order 5 раз
        Отправляь 5 CANCEL и проверять обновление BID - price, PriceList::quantity

--  SELL Orders:  & Cancel
        Отправить SELL Order 5 раз
        Отправляь 5 CANCEL и проверять обновление BID - price, PriceList::quantity

--  SELL Orders && BUY && Cancel
        Отправить 5 x SELL + 5 x BUY
        Отправляь 5 CANCEL и проверять обновление BID - price, PriceList::quantity

========================================================================================================
                            PriceLevel
========================================================================================================

--  Удаление PriceLevel:
    - при отправке Cancel
    - при AMEND + Price Change
    - при TRADE

- Send 5x BUY:  Same price - Check Quanity Change per each new Order
- Send 5x SELL: Same price - Check Quanity Change per each new Order

========================================================================================================
                            getOrdersCount | Buy orders count | SELL orders count
========================================================================================================

--  1 BUY Orders:  Check getOrdersCount() && Check BUY Order count && Check SELL Orders Count
--  1 SELL Orders:  Check getOrdersCount() && Check BUY Order count && Check SELL Orders Count
--  1 SELL Orders && 1 BUY: Check orders count
--  1 SELL Orders && CANCEL
--  1 BUY Orders && CANCEL
--  1 SELL Orders && 1 BUY && Cancel BUY
--  1 SELL Orders && 1 BUY && Cancel SELL