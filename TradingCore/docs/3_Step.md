Да. На текущем этапе `market_data` + `execution` + зачаток `strategy` уже дают хороший фундамент. Я бы **не прыгал сразу в `risk`**, а двигался от торгового цикла к состоянию портфеля.

Я бы предложил такой порядок.

## 1. Доделать `Position`

Это следующий наиболее логичный модуль.

```text
src/position/
├── position.hpp
└── position_manager.hpp
```

`Position` должен отвечать примерно за:

```text
Instrument
    │
    ▼
PositionManager
    │
    ├── quantity
    ├── average entry price
    ├── realized PnL
    └── ...
```

Например:

```cpp
class PositionManager
{
public:
    void applyExecution(const ExecutionReport& report);

    [[nodiscard]]
    Quantity position(InstrumentId instrument) const noexcept;

    [[nodiscard]]
    Price averagePrice(InstrumentId instrument) const noexcept;
};
```

Но здесь надо аккуратно определить семантику Buy/Sell и partial fills.

**Это я бы сделал первым.**

---

# 2. `PnL`

После `Position` можно нормально реализовать PnL.

```text
src/pnl/
├── pnl_calculator.hpp
└── pnl_calculator.cpp
```

Причём я бы разделил:

### Realized PnL

Возникает при закрытии позиции:

```text
BUY  100 @ 100
SELL 100 @ 105

Realized PnL = +500
```

### Unrealized PnL

Зависит от текущей market price:

```text
Position = +100
Average price = 100
Market price = 105

Unrealized PnL = +500
```

И тут как раз `MarketEvent` становится источником market price.

---

# 3. `Risk`

После появления позиции и PnL risk становится гораздо содержательнее.

```text
src/risk/
├── risk_engine.hpp
└── risk_engine.cpp
```

Например:

```text
OrderRequest
     │
     ▼
 RiskEngine
     │
     ├── max order quantity
     ├── max position
     ├── max notional
     ├── price limits
     └── ...
     │
     ▼
OrderManager
```

То есть **risk должен находиться перед execution**, а не после него.

Получится:

```text
Strategy
   │
   ▼
RiskEngine
   │
   ▼
OrderManager
   │
   ▼
ExecutionGateway
```

---

# 4. Связать `Execution → Position → PnL`

Это будет первый настоящий end-to-end кусок.

```text
                    ┌──────────────┐
                    │   Strategy   │
                    └──────┬───────┘
                           │ OrderRequest
                           ▼
                    ┌──────────────┐
                    │ Risk Engine  │
                    └──────┬───────┘
                           │
                           ▼
                    ┌──────────────┐
                    │ OrderManager │
                    └──────┬───────┘
                           │
                           ▼
                    ┌──────────────┐
                    │   Exchange   │
                    └──────┬───────┘
                           │ ExecutionReport
                           ▼
                    ┌──────────────┐
                    │   Position   │
                    └──────┬───────┘
                           │
                           ▼
                    ┌──────────────┐
                    │     PnL      │
                    └──────────────┘
```

Это я бы считал **следующим большим milestone**.

---

# 5. `Recording`

После этого уже имеет смысл заниматься записью событий.

```text
src/recording/
├── event_log.hpp
├── event_log.cpp
├── replay.hpp
└── replay.cpp
```

И здесь можно записывать:

```text
MarketEvent
BookUpdate
OrderRequest
ExecutionReport
```

Причём это очень полезно именно для нашего проекта, потому что потом можно сделать:

```text
Recorded market data
        │
        ▼
      Replay
        │
        ▼
     Strategy
        │
        ▼
      Orders
```

и тестировать стратегию **без подключения к Binance**.

---

# 6. Расширить `Strategy`

Сейчас:

```text
strategy/
├── strategy.hpp
├── imbalance_strategy.hpp
└── imbalance_strategy.cpp
```

Я бы после инфраструктуры сделал нормальный lifecycle:

```text
MarketEvent
     │
     ▼
Strategy::onMarketEvent()
     │
     ▼
OrderRequest
```

Причём Strategy не должна знать о:

* Binance
* OrderManager
* ExecutionGateway
* RiskEngine

Она должна только принимать market data и генерировать trading intent/order requests.

---

# 7. `App`

И только после этого я бы собирал всё в:

```text
src/app/
└── main.cpp
```

Получится примерно:

```text
                    Binance
                       │
                       ▼
              MarketDataSource
                       │
                       ▼
                    Parser
                       │
                       ▼
                 BookBuilder
                       │
                       ▼
                  MarketEvent
                       │
                       ▼
                   Strategy
                       │
                 OrderRequest
                       ▼
                  RiskEngine
                       │
                       ▼
                 OrderManager
                       │
                       ▼
              ExecutionGateway
                       │
                ExecutionReport
                       │
            ┌──────────┴──────────┐
            ▼                     ▼
       PositionManager          PnL
```

---

# 8. Тесты

И параллельно, а не в самом конце.

Сейчас:

```text
tests/
├── execution/
│   └── order_manager_test.cpp
│
└── market_data/
    ├── book_builder_test.cpp
    └── order_book_test.cpp
```

Я бы постепенно получил:

```text
tests/
├── core/
│   ├── price_test.cpp
│   ├── quantity_test.cpp
│   └── timestamp_test.cpp
│
├── market_data/
│   ├── order_book_test.cpp
│   ├── book_builder_test.cpp
│   ├── market_data_message_handler_test.cpp
│   └── ...
│
├── execution/
│   └── order_manager_test.cpp
│
├── position/
│   └── position_manager_test.cpp
│
├── pnl/
│   └── pnl_calculator_test.cpp
│
├── risk/
│   └── risk_engine_test.cpp
│
└── strategy/
    └── imbalance_strategy_test.cpp
```

А Binance-specific parser тоже обязательно отдельно:

```text
tests/exchanges/binance/
└── binance_market_data_parser_test.cpp
```

---

# Мой предлагаемый roadmap

Я бы сейчас зафиксировал такой порядок:

```text
1. Position
       ↓
2. PnL
       ↓
3. Risk
       ↓
4. Execution → Position → PnL integration
       ↓
5. Recording / Replay
       ↓
6. Strategy
       ↓
7. App / wiring
       ↓
8. Integration tests
       ↓
9. Benchmarks
```

При этом **unit-тесты пишем вместе с каждым модулем**.

### И ещё важный момент

Я бы пока **не добавлял новые сложные абстракции** вроде:

```text
IPortfolio
IPnl
IRiskManager
IPositionStore
IEventBus
```

если для них ещё нет реальной необходимости.

Сейчас полезнее сначала получить **простую, работающую модель торгового цикла**, а уже потом посмотреть, где действительно нужны interfaces.

**Следующий конкретный шаг я бы сделал `PositionManager` + его unit-тесты.** Это даст нам основу для `PnL` и `Risk`.
