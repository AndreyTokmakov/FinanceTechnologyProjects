Ниже — **SIMD Market Data Parser** в контексте **HFT / low-latency C++**, оформлено как **README-раздел модуля** + **реалистичный пример кода**.
Это именно то, что используют в проде для ITCH / бинарных фидов (не JSON).

---

## SIMD Market Data Parser

**SIMD Market Data Parser** — модуль высокопроизводительного парсинга биржевых market data сообщений с использованием **AVX2 / AVX-512**, ориентированный на **микросекундную латентность** и **максимальный throughput**.

Модуль предназначен для обработки **binary market data feeds** (ITCH, OUCH, FAST, proprietary protocols) и интегрируется в HFT pipeline.

---

## Цели модуля

* Минимальная latency (nanoseconds per message)
* Zero allocations
* Высокий throughput (10–100+ M msg/s)
* Предсказуемое поведение
* SIMD-friendly layout данных

---

## Место в HFT pipeline

```
NIC
 ↓
Kernel bypass (DPDK / AF_XDP / Solarflare)
 ↓
SIMD Market Data Parser   <─── ЭТОТ МОДУЛЬ
 ↓
Order Book
 ↓
Strategy
```

---

## Почему SIMD

### Без SIMD

* 1 сообщение = 1 цикл парсинга
* Много branch’ей
* Низкий IPC

### С SIMD

* Парсинг **8–16 сообщений за раз**
* Branchless logic
* Высокая загрузка CPU

---

## Общие принципы дизайна

* **Structure of Arrays (SoA)** вместо AoS
* Выравнивание по cache line (64B)
* Фиксированные размеры сообщений
* Минимум условных переходов
* No `std::string`, no `std::vector`

---

## Пример бинарного сообщения (упрощённо)

```
| type (1B) | price (8B) | qty (4B) | flags (1B) |
```

---

## Выходные данные парсера

```cpp
struct ParsedUpdate {
    int64_t price;
    int32_t qty;
    uint8_t type;
};
```

---

## SIMD layout (SoA)

```cpp
struct alignas(64) SIMDUpdates {
    int64_t price[8];
    int32_t qty[8];
    uint8_t type[8];
};
```

---

## SIMD Parser (AVX2, пример)

### Заголовок

```cpp
#include <immintrin.h>
#include <cstdint>
```

---

### SIMD загрузка цен

```cpp
inline __m256i load_prices(const uint8_t* ptr) noexcept {
    return _mm256_loadu_si256(
        reinterpret_cast<const __m256i*>(ptr)
    );
}
```

---

### SIMD парсинг batch из 8 сообщений

```cpp
inline void parse_batch_8(
    const uint8_t* data,
    SIMDUpdates& out
) noexcept
{
    // price offsets (example)
    constexpr int PRICE_OFFSET = 1;

    __m256i prices = _mm256_loadu_si256(
        reinterpret_cast<const __m256i*>(data + PRICE_OFFSET)
    );

    _mm256_store_si256(
        reinterpret_cast<__m256i*>(out.price),
        prices
    );

    // qty (scalar for simplicity here)
    #pragma unroll
    for (int i = 0; i < 8; ++i) {
        out.qty[i] =
            *reinterpret_cast<const int32_t*>(
                data + i * 16 + 9
            );
        out.type[i] = data[i * 16];
    }
}
```

⚠️ В реальном коде **qty и type тоже SIMD**, но пример оставлен читаемым.

---

## Branchless фильтрация сообщений

Например, оставить только `ADD_ORDER`:

```cpp
inline uint32_t filter_add_orders(
    const SIMDUpdates& u
) noexcept
{
    constexpr uint8_t ADD = 0x01;

    uint32_t mask = 0;
    #pragma unroll
    for (int i = 0; i < 8; ++i)
        mask |= (u.type[i] == ADD) << i;

    return mask;
}
```

---

## Интеграция с Order Book

```cpp
void on_market_data(const SIMDUpdates& u) {
    #pragma unroll
    for (int i = 0; i < 8; ++i) {
        order_book.on_add(
            u.price[i],
            u.qty[i]
        );
    }
}
```

---

## HFT-свойства реализации

* SIMD batch processing
* No heap allocations
* Fixed-size buffers
* Cache-line aligned
* Подходит для replay и live feed

---

## Что оптимизируют дальше

1. AVX-512 (16 сообщений за batch)
2. Prefetch next packet
3. NIC RSS → core pinning
4. Endianness fixups SIMD’ом
5. Separate hot / cold paths

---

## Типичная производительность

| Подход  | Msg/s (1 core) |
| ------- | -------------- |
| Scalar  | ~5–10M         |
| AVX2    | ~40–70M        |
| AVX-512 | 100M+          |

(зависит от протокола и CPU)

---

## Roadmap модуля

1. Полный ITCH parser
2. AVX-512 backend
3. Replay engine
4. Drop detection
5. Latency timestamps

---

## Связанные модули

* Order Book (L2 / L3)
* Market Making Strategy
* Exchange Simulator
* Risk Engine

---

Если хочешь, следующим шагом можем:

* 🔥 разобрать **реальный ITCH message**
* 🔥 написать **AVX-512 версию**
* 🔥 связать parser → order book → strategy
* 🔥 сделать **market data replay engine**

Скажи, **что делаем дальше**.
