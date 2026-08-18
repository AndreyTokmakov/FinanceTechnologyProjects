Дальше я бы двигался так, чтобы сначала построить **полный execution → position → PnL → risk контур**, а уже потом делать `app` и recording.

Сейчас у нас уже есть:

```text
Market Data
    Exchange
       ↓
IMarketDataSource
       ↓
IMarketDataMessageHandler
       ↓
MarketDataParser
       ↓
IBookUpdateHandler
       ↓
BookBuilder
       ↓
OrderBook
       ↓
MarketEvent
       ↓
Strategy
```

и:

```text
Order
   ↓
OrderManager
   ↓
ExecutionGateway
   ↓
ExecutionReport
```

Я бы следующим сделал **Position**, но с небольшой корректировкой модели `ExecutionReport`.

### Следующий шаг

#### 1. Доработать `ExecutionReport`

Добавить туда данные, необходимые для позиции:

```cpp
InstrumentId instrument;
Side side;
```

В итоге:

```text
ExecutionReport
├── clientOrderId
├── exchangeOrderId
├── instrument
├── side
├── execType
├── status
├── price
├── quantity
└── filledQuantity
```

Это нужно сделать первым, потому что без `instrument` и `side` `PositionManager` не сможет работать.

---

#### 2. Реализовать `Position`

```text
src/position/
├── position.hpp
├── position_manager.hpp
└── position_manager.cpp
```

Пока ответственность:

```text
Trade
  ↓
PositionManager
  ↓
Position
```

`Position` хранит:

```text
InstrumentId
signed quantity
average entry price
```

Например:

```text
BUY  100 @ 100
BUY  100 @ 110

position = +200
average  = 105
```

и:

```text
SELL 100 @ 100
SELL 100 @ 110

position = -200
average  = 105
```

---

#### 3. Тесты PositionManager

Обязательно проверить:

```text
empty position
first Buy
first Sell
increase long
increase short
partial close long
full close long
partial close short
full close short
reverse long → short
reverse short → long
multiple instruments
ignore New
ignore Cancel
ignore Reject
```

Особенно важны переходы через `0`:

```text
+100 → SELL 150 → -50
-100 → BUY 150 → +50
```

---

### После Position

#### 4. `PnL`

Я бы сделал отдельный модуль:

```text
src/pnl/
├── pnl_calculator.hpp
├── pnl_calculator.cpp
└── ...
```

С разделением:

```text
Position
   │
   ├──────────────┐
   │              │
   ↓              ↓
Realized PnL   Unrealized PnL
                  ↑
                  │
              MarketEvent
```

То есть:

```text
ExecutionReport
       ↓
PositionManager
       ↓
Position ──────────────┐
                       │
MarketEvent ───────────┤
                       ↓
                  PnLCalculator
                       ↓
                      PnL
```

Здесь отдельно аккуратно решим fixed-point арифметику `Price × Quantity`.

---

### 5. `Risk`

После PnL:

```text
src/risk/
├── risk_manager.hpp
├── risk_manager.cpp
└── ...
```

Первый MVP risk может проверять:

```text
max position
max order quantity
max notional
max orders
max loss
```

Поток:

```text
Strategy
   │
   │ OrderRequest
   ↓
RiskManager
   │
   ├── rejected
   │
   └── accepted
          ↓
     OrderManager
```

Это важно: **Risk должен стоять перед отправкой ордера на ExecutionGateway.**

---

### 6. Recording

После этого:

```text
src/recording/
├── market_data_recorder.hpp
├── market_data_recorder.cpp
├── execution_recorder.hpp
└── execution_recorder.cpp
```

Причём я бы сначала записывал **нормализованные события**, а не Binance-specific JSON:

```text
MarketEvent
BookUpdate
ExecutionReport
Order
```

Это позволит потом воспроизводить торговую сессию независимо от биржи.

---

### 7. App

И только потом:

```text
src/app/
├── application.hpp
└── application.cpp
```

`Application` будет связывать всё вместе:

```text
                    Application
                         │
        ┌────────────────┼────────────────┐
        │                │                │
        ↓                ↓                ↓
 MarketData          Strategy         Execution
        │                │                │
        ↓                ↓                ↓
  OrderBook       OrderRequest     ExecutionGateway
        │                │                │
        └───────→ PnL ←──┴──── Position ←─┘
                       │
                       ↓
                     Risk
```

---

## Я бы зафиксировал такой roadmap

```text
[✓] Core types
[✓] Price / Quantity / Timestamp
[✓] OrderBook
[✓] BookBuilder
[✓] MarketData pipeline
[✓] Binance MarketDataSource / Parser
[✓] Strategy
[✓] Order / OrderManager
[✓] ExecutionGateway
[✓] ExecutionReport

[ ] 1. Доработать ExecutionReport
[ ] 2. Position
[ ] 3. PositionManager
[ ] 4. PositionManager tests
[ ] 5. PnL
[ ] 6. PnL tests
[ ] 7. Risk
[ ] 8. Risk tests
[ ] 9. Recording / Replay
[ ] 10. App / composition root
[ ] 11. Integration tests
[ ] 12. End-to-end simulation
```

**Я бы сейчас продолжил с пункта 1 — `ExecutionReport`**, потому что это маленькое изменение, но оно определит правильный контракт между `execution` и `position`. Затем сразу реализуем `Position` + `PositionManager` и их тесты.
