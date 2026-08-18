Да. После `Position` и `PnL` логично перейти к **Risk**.

Я бы сейчас сделал не огромный risk-engine, а минимальный, но архитектурно правильный слой:

```text
ExecutionReport
      │
      ▼
PositionManager
      │
      ▼
   Position
      │
      ├──────────────┐
      │              │
      ▼              ▼
PnLCalculator    RiskManager
                     │
                     ▼
                RiskDecision
```

### Что реализуем

```text
src/risk/
├── risk.hpp
├── risk_limits.hpp
├── risk_manager.hpp
└── risk_manager.cpp

tests/risk/
└── risk_manager_test.cpp
```

#### `risk.hpp`

Модель результата проверки:

```cpp
enum class RiskResult
{
    Accepted,
    Rejected
};
```

и, вероятно, причина отказа:

```cpp
enum class RiskReason
{
    None,
    MaxOrderQuantity,
    MaxPositionQuantity,
    MaxNotional
};
```

#### `risk_limits.hpp`

Конфигурация лимитов:

```cpp
struct RiskLimits
{
    Quantity maxOrderQuantity {};
    Quantity maxPositionQuantity {};
    Price maxNotional {};
};
```

#### `risk_manager.hpp/.cpp`

Основная ответственность:

```cpp
class RiskManager
{
public:
    explicit RiskManager(RiskLimits limits) noexcept;

    [[nodiscard]]
    RiskResult checkOrder(const OrderRequest& request,
                          const Position& position) const noexcept;

    [[nodiscard]]
    RiskReason lastReason() const noexcept;

private:
    RiskLimits limits;
};
```

Но здесь я бы сделал ещё одно архитектурное решение **до написания кода**.

Для HFT/трейдинга Risk должен проверять **order до отправки в `IExecutionGateway`**:

```text
Strategy
   │
   │ OrderRequest
   ▼
OrderManager
   │
   ▼
RiskManager
   │
   ├── Reject ──────► Strategy / caller
   │
   └── Accept
        │
        ▼
   create Order
        │
        ▼
IExecutionGateway
```

То есть текущий:

```cpp
OrderManager::createOrder()
```

станет примерно:

```cpp
OrderId OrderManager::createOrder(const OrderRequest& request)
{
    if (!riskManager.checkOrder(request, position))
        return 0;

    ...
    gateway.send(order);

    return orderId;
}
```

Но **`OrderManager` не должен сам знать, как считается risk**. Он только вызывает `IRiskManager`.

Поэтому я предлагаю сделать интерфейс:

```text
IRiskManager
     ▲
     │
RiskManager
```

и передавать его в `OrderManager`.

В итоге получится хороший основной execution flow:

```text
MarketData
    │
    ▼
OrderBook
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
    ▼
OrderRequest
    │
    ▼
OrderManager
    │
    ▼
IRiskManager
    │
    ├── reject
    │
    └── accept
          │
          ▼
   IExecutionGateway
          │
          ▼
       Exchange
          │
          ▼
  ExecutionReport
          │
          ├──────────────► OrderManager
          │
          └──────────────► PositionManager
                              │
                              ▼
                           Position
                              │
                              ▼
                        PnLCalculator
```

Это уже начинает выглядеть как нормальный небольшой trading engine.

**Следующим шагом я предлагаю сделать именно `Risk` целиком:** все новые файлы + изменение `OrderManager` + тесты `RiskManager` и тесты того, что `OrderManager` действительно не отправляет rejected order в gateway.
