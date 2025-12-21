# HFT Market Making System (C++)

High-Frequency Trading (HFT) Market Making system written in modern C++ (C++20/23).

Проект предназначен для изучения и практической реализации **low-latency trading systems**, приближенных по архитектуре и требованиям к реальным HFT-платформам.

---

## Цели проекта

- Понять архитектуру HFT-систем
- Реализовать Market Making стратегию
- Отработать low-latency C++ паттерны
- Изучить обработку market data без аллокаций
- Построить полный pipeline: MD → Strategy → Orders

---

## Область применения

- High-Frequency Trading (HFT)
- Market Making
- Algo Trading
- Exchange simulators
- Research / Education

Проект **не является** торговым советом и используется исключительно в учебных и исследовательских целях.

---

## Trading vs Algo Trading vs HFT

### Trading
- Ручные решения
- Таймфреймы: секунды → дни
- Задержки некритичны

### Algo Trading
- Алгоритмические решения
- Таймфреймы: миллисекунды → минуты
- Важны latency и throughput

### HFT
- Микросекундные задержки
- Конкуренция за очередь заявок (queue position)
- Оптимизация до уровня:
  - CPU cache
  - NUMA
  - NIC
  - Kernel bypass

---

## Market Making

### Что делает Market Maker

- Непрерывно котирует:
  - Bid
  - Ask
- Зарабатывает на:
  - Bid/Ask spread
  - Exchange rebates
- Управляет рисками:
  - Inventory
  - Adverse selection
  - Latency risk

### Интуитивная модель котирования

```
mid_price ± (spread / 2
             + inventory_risk
             + volatility_adjustment)
```

---

## Архитектура системы

```
NIC
 ↓
Kernel bypass (DPDK / AF_XDP / Solarflare)
 ↓
Market Data Parser (SIMD)
 ↓
Order Book (lock-free, cache-friendly)
 ↓
Market Making Strategy
 ↓
Risk Checks
 ↓
Order Gateway
 ↓
Exchange / Simulator
```

---

## Структура проекта

```
project/
├── market_data/
│   ├── parser/
│   ├── protocols/
│   └── replay/
│
├── order_book/
│   ├── l2/
│   └── l3/
│
├── strategy/
│   └── market_making/
│
├── risk/
│   └── checks/
│
├── gateway/
│   └── order_sender/
│
├── exchange_sim/
│   └── matching_engine/
│
├── common/
│   ├── ring_buffer/
│   ├── time/
│   └── utils/
│
├── benchmarks/
│
└── main.cpp
```

---

## Основные компоненты

### Market Data

- Binary protocols (ITCH, OUCH, FAST)
- SIMD parsing (AVX2 / AVX-512)
- Zero allocations
- Single-producer pipelines

Используемые структуры данных:
- Flat arrays
- Ring buffers
- Intrusive containers

---

### Order Book

- L2 / L3 order book
- Fixed-size price levels
- Без `std::map` и heap-аллокаций

Подходы:
- Price → index mapping
- Dense arrays
- Pointerless design

---

### Strategy (Market Making)

```cpp
struct Quote {
    int64_t bid_px;
    int64_t ask_px;
    int32_t bid_qty;
    int32_t ask_qty;
};
```

Логика:
- Spread control
- Inventory skew
- Volatility widening
- Queue position tracking

---

### Risk Management

Inline-проверки:
- Max position
- Max order rate
- Fat-finger checks
- Kill-switch

Принципы:
- Без virtual функций
- Без исключений
- Минимальный latency

---

## Требования к C++

### Используется

- C++20 / C++23
- Компиляция: `-O3 -march=native`
- Concepts вместо virtual
- CRTP / Policy-based design
- `std::variant` вместо inheritance
- `constexpr` где возможно

### Запрещено в hot-path

- Exceptions
- RTTI
- `std::shared_ptr`
- Virtual dispatch

---

## Latency Optimization Stack

| Уровень | Оптимизация |
|------|-----------|
| CPU | Cache alignment, prefetch |
| Memory | NUMA pinning |
| OS | isolcpus, nohz_full |
| Network | Kernel bypass |
| Code | Branchless logic |
| Algo | Минимум сообщений |

---

## Roadmap

1. Реализация L2 Order Book
2. SIMD Market Data Parser
3. Market Making Strategy
4. Inline Risk Engine
5. Lock-free queues
6. Exchange Simulator
7. Replay / Backtesting
8. Latency Benchmarks

---

## Инструменты

- `perf`
- `vtune`
- `clang / gcc`
- `cmake`

---

## Disclaimer

Проект предназначен исключительно для обучения и исследований.
Не используйте данный код для реальной торговли без надлежащего аудита и лицензий.

