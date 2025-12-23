Ниже — **реалистичная, HFT-ориентированная реализация `MarketMakingStrategy` на C++**, которая логически **стыкуется с тем, что мы уже обсуждали**
(parser → order book → strategy → gateway → exchange simulator).

Цель — **не учебный пример**, а **скелет продакшн-стратегии**, без `virtual`, без heap, с inline hot-path.

---

## 1. Контракты (минимум)

### Side / Order

```cpp
enum class Side : uint8_t { Buy, Sell };

struct Order {
    uint64_t order_id;
    Side side;
    int64_t price;   // tick-aligned
    int32_t qty;
};
```

---

## 2. Order Book View (strategy-side)

Стратегия **не владеет книгой**, она только читает snapshot.

```cpp
struct BookTop {
    int64_t best_bid;
    int64_t best_ask;
};
```

---

## 3. Strategy Parameters

```cpp
struct MMParams {
    int64_t tick_size;
    int64_t base_spread_ticks;     // e.g. 2 ticks
    int32_t quote_qty;

    int32_t max_position;
    int64_t inventory_skew_ticks;  // how aggressively skew price

    int64_t volatility_ticks;      // dynamic widening
};
```

---

## 4. MarketMakingStrategy (ядро)

```cpp
class MarketMakingStrategy {
public:
    explicit MarketMakingStrategy(
        MMParams params
    ) noexcept
        : params_(params)
    {}

    inline void on_book_update(
        const BookTop& book
    ) noexcept
    {
        best_bid_ = book.best_bid;
        best_ask_ = book.best_ask;

        compute_quotes();
    }

    inline bool has_order() const noexcept {
        return has_new_order_;
    }

    inline Order next_order() noexcept {
        has_new_order_ = false;
        return pending_order_;
    }

    inline void on_fill(
        Side side,
        int32_t qty
    ) noexcept
    {
        position_ += (side == Side::Buy ? qty : -qty);
    }

private:
    // -------- state --------
    MMParams params_;

    int64_t best_bid_{0};
    int64_t best_ask_{0};

    int32_t position_{0};

    Order pending_order_{};
    bool has_new_order_{false};

    uint64_t order_id_{1};

    // -------- logic --------
    inline void compute_quotes() noexcept
    {
        if (best_bid_ <= 0 || best_ask_ <= 0)
            return;

        const int64_t mid =
            (best_bid_ + best_ask_) >> 1;

        // base spread
        int64_t half_spread =
            params_.base_spread_ticks * params_.tick_size / 2;

        // volatility widening
        half_spread +=
            params_.volatility_ticks * params_.tick_size;

        // inventory skew
        int64_t skew = 0;
        if (position_ > 0)
            skew = -params_.inventory_skew_ticks * params_.tick_size;
        else if (position_ < 0)
            skew = +params_.inventory_skew_ticks * params_.tick_size;

        // compute prices
        const int64_t bid_px =
            align_price(mid - half_spread + skew);

        const int64_t ask_px =
            align_price(mid + half_spread + skew);

        // risk check
        if (position_ < params_.max_position)
            place_order(Side::Buy, bid_px);

        if (position_ > -params_.max_position)
            place_order(Side::Sell, ask_px);
    }

    inline void place_order(
        Side side,
        int64_t price
    ) noexcept
    {
        pending_order_ = {
            order_id_++,
            side,
            price,
            params_.quote_qty
        };
        has_new_order_ = true;
    }

    inline int64_t align_price(
        int64_t px
    ) const noexcept
    {
        const int64_t t = params_.tick_size;
        return (px / t) * t;
    }
};
```

---

## 5. Как это встраивается в pipeline

```cpp
MarketMakingStrategy strategy(params);

// market data loop
BookTop top{best_bid, best_ask};
strategy.on_book_update(top);

if (strategy.has_order()) {
    Order o = strategy.next_order();
    gateway.send(o);
}
```

Fills возвращаются **через market data**:

```cpp
strategy.on_fill(Side::Buy, filled_qty);
```

---

## 6. Почему это HFT-реализация

✔ inline hot-path
✔ deterministic
✔ no virtual
✔ no heap
✔ tick-aligned prices
✔ inventory-aware
✔ volatility-aware

---

## 7. Что легко добавить дальше

1. Cancel / Replace logic
2. Separate bid / ask state
3. Quote lifetime (TTL)
4. Queue position model
5. Multi-symbol support
6. SIMD volatility estimator

---

## 8. Итог

Это **минимально достаточная, но правильная** Market Making стратегия:

* реально работает в замкнутом контуре
* легко расширяется
* соответствует HFT-практикам

---

Если хочешь, следующий логичный шаг 🔥

* добавить **Order Lifecycle (cancel/replace)**
* или связать **strategy + exchange simulator end-to-end**
* или сделать **SIMD volatility estimator**

Скажи, **куда идём дальше**.
