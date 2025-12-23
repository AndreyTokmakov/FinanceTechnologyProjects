```
SIMD Market Data Parser
        ↓
     Order Book (L2)
        ↓
 Market Making Strategy
        ↓
     Quotes / Orders
```

Ниже — **реалистичная, минимальная, но production-style связка** на C++
(без virtual, без heap, без лишнего).

---

# 1. Общая архитектура потока

```
[ NIC / Replay ]
      ↓
[ SIMD Parser ]        // batch из 8–16 сообщений
      ↓
[ Order Book L2 ]      // обновление best bid / ask
      ↓
[ Strategy ]           // расчет котировок
      ↓
[ Order Gateway ]      // place / cancel / replace
```

Ключевая идея:
**parser и order book работают batch’ами**, strategy — на **изменении top-of-book**.

---

# 2. Контракты между модулями (самое важное)

## 2.1 Parser → Order Book

Parser **не знает** про стратегию.
Он отдаёт **нормализованные события**:

```cpp
enum class MsgType : uint8_t {
    Add,
    Modify,
    Delete
};

struct MDUpdate {
    MsgType type;
    int64_t price;
    int32_t qty;
    bool is_bid;
};
```

Batch (SIMD-friendly):

```cpp
struct MDBatch {
    MDUpdate updates[8];
    uint8_t size;
};
```

---

## 2.2 Order Book → Strategy

Order Book **не дергает стратегию на каждое событие**,
а только когда изменился **best bid / ask**.

```cpp
struct MarketState {
    int64_t best_bid_px;
    int64_t best_ask_px;
    int32_t best_bid_qty;
    int32_t best_ask_qty;
};
```

---

# 3. Order Book (L2, минимальный)

```cpp
class OrderBookL2 {
public:
    inline bool on_update(const MDUpdate& u) noexcept {
        bool top_changed = false;

        if (u.is_bid) {
            if (u.type == MsgType::Add && u.price >= best_bid_px_) {
                best_bid_px_ = u.price;
                best_bid_qty_ = u.qty;
                top_changed = true;
            }
        } else {
            if (u.type == MsgType::Add && u.price <= best_ask_px_) {
                best_ask_px_ = u.price;
                best_ask_qty_ = u.qty;
                top_changed = true;
            }
        }
        return top_changed;
    }

    inline MarketState snapshot() const noexcept {
        return {
            best_bid_px_,
            best_ask_px_,
            best_bid_qty_,
            best_ask_qty_
        };
    }

private:
    int64_t best_bid_px_{0};
    int64_t best_ask_px_{INT64_MAX};
    int32_t best_bid_qty_{0};
    int32_t best_ask_qty_{0};
};
```

✔ cache-friendly
✔ inline
✔ без контейнеров

---

# 4. Strategy (уже готовая, используем)

Предположим, у нас уже есть:

```cpp
MarketMakingStrategy strategy;
Inventory inventory;
```

---

# 5. Pipeline glue code (ключевая часть)

## 5.1 Parser → Order Book → Strategy

```cpp
class HFTPipeline {
public:
    HFTPipeline(
        OrderBookL2& ob,
        MarketMakingStrategy& strat
    ) noexcept
        : order_book_(ob)
        , strategy_(strat)
    {}

    inline void on_md_batch(
        const MDBatch& batch
    ) noexcept
    {
        bool top_changed = false;

        #pragma unroll
        for (uint8_t i = 0; i < batch.size; ++i) {
            top_changed |=
                order_book_.on_update(batch.updates[i]);
        }

        if (top_changed) {
            on_top_of_book();
        }
    }

private:
    OrderBookL2& order_book_;
    MarketMakingStrategy& strategy_;

    inline void on_top_of_book() noexcept {
        MarketState mkt = order_book_.snapshot();

        Quote q = strategy_.on_market(
            mkt,
            inventory_
        );

        send_quotes(q);
    }

    inline void send_quotes(const Quote& q) noexcept {
        // order gateway (stub)
        // place / cancel / replace
    }

    Inventory inventory_{0, 100};
};
```

🔥 **важно**:

* strategy вызывается **не на каждое сообщение**
* а только при **реальном изменении рынка**

---

# 6. SIMD Parser → MDBatch (пример)

```cpp
inline MDBatch parse_simd_batch(
    const uint8_t* data
) noexcept
{
    MDBatch b;
    b.size = 8;

    #pragma unroll
    for (int i = 0; i < 8; ++i) {
        b.updates[i] = MDUpdate{
            .type = MsgType::Add,
            .price = *reinterpret_cast<const int64_t*>(data + i * 16 + 1),
            .qty   = *reinterpret_cast<const int32_t*>(data + i * 16 + 9),
            .is_bid = (data[i * 16] & 0x1)
        };
    }
    return b;
}
```

(в реальности — SIMD load + shuffle, здесь логика)

---

# 7. Полный поток (как это живёт)

```cpp
OrderBookL2 order_book;
MarketMakingStrategy strategy(params);
HFTPipeline pipeline(order_book, strategy);

while (running) {
    uint8_t* packet = receive_md();
    MDBatch batch = parse_simd_batch(packet);
    pipeline.on_md_batch(batch);
}
```

---

# 8. Почему это HFT-правильно

✔ Parser — batch + SIMD
✔ Order Book — cheap updates
✔ Strategy — вызывается редко
✔ Нет virtual / heap
✔ Хороший cache locality

---

# 9. Что делают в проде дальше

1. Microprice вместо mid
2. Imbalance signal из Order Book
3. Queue position tracking
4. Order throttling
5. Cancel/replace latency control
6. Separate hot / cold path

---

## Следующий логичный шаг

Мы можем:

* 🔥 усложнить **Order Book (реальный L2)**
* 🔥 добавить **Order Lifecycle**
* 🔥 сделать **Exchange Simulator**
* 🔥 или собрать **полный README “HFT Pipeline”**

Скажи, **куда идём дальше**.
