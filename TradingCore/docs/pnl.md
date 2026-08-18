Да. Следующий шаг по плану — **PnL**.

Но перед ним я бы зафиксировал небольшое архитектурное решение: `Position` сейчас хранит только состояние позиции и `averageEntryPrice`, а PnL должен быть отдельным модулем.

### Следующий этап

Добавляем:

```text
src/pnl/
├── pnl.hpp
├── pnl_calculator.hpp
└── pnl_calculator.cpp

tests/pnl/
└── pnl_calculator_test.cpp
```

Поток будет таким:

```text
                    ExecutionReport
                           │
                           ▼
                    PositionManager
                           │
                           ▼
                       Position
                           │
                           │ position state
                           ▼
MarketEvent ─────────► PnLCalculator
                           │
                           ▼
                          PnL
```

### Ответственность `PnLCalculator`

Он должен уметь разделять:

**Realized PnL**

Возникает при закрытии существующей позиции:

```text
BUY  100 @ 100
SELL 100 @ 110

Realized PnL = +1000
```

**Unrealized PnL**

Зависит от текущей рыночной цены:

```text
Position:
    +100 @ 100

MarketEvent:
    bestBid = 110

Unrealized PnL = +1000
```

Для short:

```text
Position:
    -100 @ 100

MarketEvent:
    bestAsk = 90

Unrealized PnL = +1000
```

### Важный момент

Я бы **не заставлял `PnLCalculator` самостоятельно хранить позиции**.

Он получает:

```cpp
const Position&
```

и рыночную цену:

```cpp
const MarketEvent&
```

То есть `PositionManager` остаётся единственным владельцем состояния позиции.

---

И ещё я бы сейчас немного расширил модель `PnL`, чтобы она была пригодна для Risk:

```text
PnL
├── realized
├── unrealized
└── total
```

Причём `Price × Quantity` будем считать через `__int128` во внутренней арифметике, поскольку наши `Price` и `Quantity` имеют fixed-point scale `1e8`, и обычный `int64_t` для промежуточного произведения потенциально недостаточен.

**Следующий конкретный шаг:** вывести все файлы `src/pnl/` и тесты, сразу с тем же форматом подробных шапок и `Data Flow`, который мы теперь используем.
