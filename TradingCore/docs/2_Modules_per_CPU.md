Да. И здесь я бы разделил два понятия:

1. **логические модули** — `MarketData`, `OrderBook`, `Strategy`, `Risk`, `Execution` и т.д.;
2. **execution threads / CPU cores** — где физически выполняется каждый кусок.

Для нашего MVP я бы **не делал один thread на каждый класс**. Намного лучше собрать связанные компоненты в несколько latency-critical stages.

## 1. Предлагаемая схема

Для первого варианта:

```text
                         Binance
                            │
                    WebSocket / REST
                            │
                            ▼
                  ┌───────────────────┐
                  │   Network RX      │
                  │      CPU 0        │
                  └─────────┬─────────┘
                            │
                       SPSC queue
                            │
                            ▼
                  ┌───────────────────┐
                  │ Market Data       │
                  │ + Parser          │
                  │ + OrderBook       │
                  │      CPU 1        │
                  └─────────┬─────────┘
                            │
                       SPSC queue
                            │
                            ▼
                  ┌───────────────────┐
                  │ Strategy           │
                  │ + Risk             │
                  │      CPU 2        │
                  └─────────┬─────────┘
                            │
                       SPSC queue
                            │
                            ▼
                  ┌───────────────────┐
                  │ Execution          │
                  │      CPU 3        │
                  └─────────┬─────────┘
                            │
                            ▼
                         Binance
```

А отдельно:

```text
              ┌──────────────────────────┐
              │ Recorder / Logger        │
              │ Replay / Metrics         │
              │                          │
              │ non-critical             │
              └──────────────────────────┘
```

То есть условно:

|       CPU | Thread      | Логические модули                        |
| --------: | ----------- | ---------------------------------------- |
|     CPU 0 | Network RX  | Binance Market Data Gateway              |
|     CPU 1 | Market Data | Parser + OrderBook                       |
|     CPU 2 | Trading     | Strategy + Risk + Position + PnL         |
|     CPU 3 | Execution   | Execution Gateway + Binance              |
| остальные | Background  | Recorder, Replay, Metrics, Control Plane |

Это очень близко к тому thread model, который был предложен в исходном документе: `network RX → market data + order book → strategy + risk → execution`.

---

# 2. Почему именно такое разделение

Главный принцип:

> **Thread boundary должен соответствовать потоку данных, а не архитектурному классу.**

Например, я **не стал бы** делать:

```text
CPU 0 → Parser
CPU 1 → OrderBook
CPU 2 → Strategy
CPU 3 → Risk
CPU 4 → Position
CPU 5 → PnL
CPU 6 → Execution
```

Это создаст слишком много communication boundaries.

Вместо этого:

```text
MarketData thread:

    receive
       ↓
    parse
       ↓
    normalize
       ↓
    OrderBook
       ↓
    MarketEvent
```

Все эти операции работают **в одном thread**.

Нет смысла передавать данные:

```text
Parser
   ↓ queue
OrderBook
   ↓ queue
Strategy
```

если они последовательно обрабатываются одним потоком.

---

# 3. CPU 0 — Network RX

Задача CPU 0:

```text
socket
   ↓
recv/read
   ↓
raw network message
   ↓
SPSC queue
```

Например:

```text
CPU 0
┌────────────────────────────┐
│ BinanceConnection          │
│                            │
│ socket                     │
│     ↓                      │
│ recv()                     │
│     ↓                      │
│ raw message                │
└─────────────┬──────────────┘
              │
              ▼
          RX Queue
```

Этот thread **не должен заниматься**:

* OrderBook
* Strategy
* Risk
* JSON → сложные domain objects
* logging

Его задача максимально простая:

> **получить байты от NIC/kernel и передать дальше.**

Это позволяет измерять:

```text
NIC/socket
     ↓
RX timestamp
```

и отдельно:

```text
RX
 ↓
decode
```

---

# 4. CPU 1 — Market Data + OrderBook

Вот здесь начинается настоящий trading core.

```text
CPU 1
┌───────────────────────────────┐
│ Market Data Thread            │
│                               │
│ Raw Message                   │
│      ↓                        │
│ Parser                        │
│      ↓                        │
│ Normalize                     │
│      ↓                        │
│ OrderBook                     │
│      ↓                        │
│ MarketEvent                   │
└───────────────┬───────────────┘
                │
                ▼
             SPSC
```

Например:

```cpp
struct MarketEvent
{
    InstrumentId instrument;
    uint64_t sequence;
    Timestamp exchangeTimestamp;
    Timestamp receiveTimestamp;

    Side side;
    Price price;
    Quantity quantity;
};
```

После этого `OrderBook` полностью обновлён.

И только после успешного применения update мы передаём событие дальше.

---

# 5. CPU 2 — Trading Core

Здесь я бы **объединил несколько логических модулей**:

```text
CPU 2
┌──────────────────────────────┐
│ Trading Thread               │
│                              │
│ MarketEvent                  │
│      ↓                       │
│ Strategy                     │
│      ↓                       │
│ OrderRequest                 │
│      ↓                       │
│ Risk                         │
│      ↓                       │
│ Position / PnL               │
│                              │
└───────────────┬──────────────┘
                │
                ▼
             SPSC
```

Но здесь есть один важный нюанс.

### Position/PnL не обязательно находятся на critical path

Например:

```text
Strategy
   ↓
Risk
   ↓
OrderRequest
   ↓
Execution
```

а `Position` обновляется на `ExecutionReport`.

То есть логически:

```text
             ┌──→ Position
             │
Execution ───┼──→ PnL
             │
             └──→ Strategy state
```

При этом для MVP я бы всё равно оставил их **на том же CPU**, чтобы не создавать дополнительные queues.

---

# 6. CPU 3 — Execution

Execution thread:

```text
CPU 3
┌──────────────────────────────┐
│ Execution Thread             │
│                              │
│ OrderRequest                 │
│      ↓                       │
│ Exchange Adapter             │
│      ↓                       │
│ serialize                    │
│      ↓                       │
│ send                         │
│      ↓                       │
│ Binance                      │
└──────────────────────────────┘
```

И обратно:

```text
Binance
   │
   │ ExecutionReport
   ▼
Execution Thread
   │
   ▼
Trading Thread
```

То есть получается **двунаправленная SPSC communication**:

```text
Trading CPU 2
      │
      │ OrderRequest
      ▼
Execution CPU 3
      │
      │ ExecutionReport
      ▼
Trading CPU 2
```

---

# 7. А Recorder куда?

Вот здесь важно **не испортить hot path**.

Я бы сделал:

```text
                         ┌───────────────┐
                         │   Recorder    │
                         │    CPU 4      │
                         └───────▲───────┘
                                 │
                            async queue
                                 │
                                 │
MarketData ──────────────────────┤
Strategy   ──────────────────────┤
Execution  ──────────────────────┤
Risk       ──────────────────────┘
```

То есть:

```text
Trading thread
      │
      ├──────────────→ Trading continues
      │
      └──────────────→ Recorder queue
```

**Никогда:**

```cpp
strategy.on_market_data();

write_to_disk(event);  // плохо
```

на hot path.

Лучше:

```cpp
strategy.on_market_data();

recorder.try_push(event);
```

И ещё лучше — чтобы запись вообще не блокировала producer.

Recorder уже отдельно:

```text
queue
  ↓
batch
  ↓
binary file
```

Это особенно важно потому, что recorder/replay в исходной архитектуре рассматривался как фундаментальная часть MVP, а не как что-то, что надо обязательно помещать в trading path.

---

# 8. А Control Plane?

Его вообще можно вынести за пределы этой схемы:

```text
                 Control Plane
        ┌──────────────────────────┐
        │ REST API                  │
        │ configuration             │
        │ metrics                   │
        │ Web UI                    │
        │ database                  │
        └────────────┬─────────────┘
                     │
                     │ control commands
                     ▼
              Trading Engine
```

Он может работать на:

```text
CPU 5+
```

или вообще на тех же CPU, если нагрузка небольшая.

Latency здесь не критична.

Именно поэтому в исходной архитектуре `Trading Core` и `Control Plane` предлагалось разделять.

---

# 9. Получается вот такая архитектура

Я бы зафиксировал её примерно так:

```text
                       ┌─────────────────────┐
                       │      Binance        │
                       └───────┬─────▲───────┘
                               │     │
                         MarketData Execution
                               │     │
                               ▼     │
                     ┌────────────────────┐
CPU 0                │   Network Gateway  │
                     └─────────┬──────────┘
                               │
                            SPSC
                               │
                               ▼
CPU 1                ┌────────────────────┐
                     │   Market Data      │
                     │                    │
                     │ Parser             │
                     │ Normalize          │
                     │ OrderBook          │
                     └─────────┬──────────┘
                               │
                            SPSC
                               │
                               ▼
CPU 2                ┌────────────────────┐
                     │   Trading Core     │
                     │                    │
                     │ Strategy           │
                     │ Risk               │
                     │ Position           │
                     │ PnL                │
                     └─────────┬──────────┘
                               │
                            SPSC
                               │
                               ▼
CPU 3                ┌────────────────────┐
                     │   Execution        │
                     │                    │
                     │ Order Manager      │
                     │ Exchange Adapter   │
                     └─────────┬──────────┘
                               │
                               ▼
                            Binance


          ┌───────────────────────────────────┐
          │          Background               │
          │                                   │
CPU 4     │ Recorder / Replay / Metrics       │
CPU 5     │ Control Plane / REST / DB         │
          └───────────────────────────────────┘
```

## 10. И самое главное — где находятся boundaries

Я бы сделал **ровно три основные очереди**:

```text
          Queue #1              Queue #2              Queue #3

CPU 0 ───────────→ CPU 1 ───────────→ CPU 2 ───────────→ CPU 3
Network            Market Data         Trading            Execution
                    + Book             + Risk
```

То есть:

```text
RawMessage
    ↓
[ Q1 ]
    ↓
MarketEvent
    ↓
[ Q2 ]
    ↓
OrderRequest
    ↓
[ Q3 ]
    ↓
Exchange
```

И обратно:

```text
Exchange
    ↓
ExecutionReport
    ↓
[ Q4 ]
    ↓
Trading
```

Причём **Q4 — отдельная обратная SPSC queue**, а не какой-нибудь общий EventBus.

---

## 11. Почему SPSC здесь особенно хорошо подходит

У нас очень естественная модель:

```text
one producer
      │
      ▼
    queue
      │
      ▼
one consumer
```

Например:

```text
Network thread
      │
      ▼
    Q1
      │
      ▼
Market Data thread
```

Это практически идеальный случай для SPSC ring buffer.

И дальше:

```text
Market Data
      │
      ▼
    Q2
      │
      ▼
Trading
```

```text
Trading
      │
      ▼
    Q3
      │
      ▼
Execution
```

Именно поэтому я бы **не начинал с какого-нибудь глобального lock-free EventBus**. Здесь communication topology заранее известна и очень простая.

---

## 12. Но есть важная оговорка

Это **не означает**, что CPU 0–3 обязательно должны быть физически отдельными CPU.

Это наша **логическая модель**, которую потом надо проверить benchmark'ами.

Например, вполне может оказаться, что:

```text
CPU 0
Network RX
   ↓
Market Data + OrderBook
```

быстрее, чем два отдельных CPU, потому что мы убираем:

```text
queue
cache miss
cross-core communication
```

Поэтому я бы начал с архитектуры:

```text
Network
    ↓
Market Data + OrderBook
    ↓
Strategy + Risk
    ↓
Execution
```

а **CPU affinity и количество threads уже оптимизировал бы измерениями**.

Это соответствует исходному принципу: сначала измерить `RX → decode → book → strategy → risk → send`, а затем решать, нужны ли SPSC, CPU pinning, cache optimization и прочие low-level оптимизации.

И вот следующим шагом я бы уже **очень подробно спроектировал эти 4 threads и контракты между ними**: какие именно сообщения идут через каждую очередь, кто владеет памятью, где timestamps, где создаются объекты и как сделать это без allocation на hot path.
