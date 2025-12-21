# Market Making Strategy (C++)

Low-latency Market Making strategy written in modern C++ (C++20/23).

Данный модуль предназначен для использования в HFT / low-latency торговых системах и реализует котирование Bid / Ask с учётом inventory, волатильности и риск-ограничений.

---

## Цели модуля

- Реализация Market Making логики без virtual функций
- Минимальный latency (inline, no heap)
- Предсказуемое поведение под высокой нагрузкой
- Простая интеграция в HFT pipeline

---

## Идея стратегии

- Котирование Bid / Ask вокруг mid-price
- Управление spread
- Inventory-based skew
- Реакция на волатильность
- Inline risk checks

Интуитивная модель:

```
mid_price ± (spread / 2
             + inventory_skew
             + volatility_adjustment)
```

---

## Входные данные

### Market State

```cpp
struct MarketState {
    int64_t best_bid_px;
    int64_t best_ask_px;
    int32_t best_bid_qty;
    int32_t best_ask_qty;
};
```

### Inventory

```cpp
struct Inventory {
    int32_t position;   // signed
    int32_t max_pos;
};
```

---

## Выход стратегии (Quotes)

```cpp
struct Quote {
    int64_t bid_px;
    int64_t ask_px;
    int32_t bid_qty;
    int32_t ask_qty;
};
```

---

## Параметры стратегии

```cpp
struct MMParams {
    int64_t tick_size;        // price step
    int64_t base_spread;      // ticks
    int32_t base_qty;
    int32_t max_position;

    int64_t inventory_skew;   // ticks per position unit
    int64_t vol_widening;     // ticks per volatility unit
};
```

---

## Оценка волатильности (EMA)

Быстрая EMA без хранения истории и динамических аллокаций.

```cpp
struct VolatilityEMA {
    int64_t ema = 0;
    static constexpr int64_t alpha = 8; // power of two

    inline void update(int64_t mid_diff) noexcept {
        ema += (mid_diff - ema) >> alpha;
    }

    inline int64_t value() const noexcept {
        return ema;
    }
};
```

---

## Реализация стратегии

```cpp
class MarketMakingStrategy {
public:
    explicit MarketMakingStrategy(const MMParams& p) noexcept
        : params_(p) {}

    inline Quote on_market(
        const MarketState& mkt,
        Inventory inv
    ) noexcept
    {
        const int64_t mid =
            (mkt.best_bid_px + mkt.best_ask_px) >> 1;

        const int64_t mid_diff = mid - last_mid_;
        vol_.update(mid_diff);
        last_mid_ = mid;

        int64_t spread =
            params_.base_spread +
            (vol_.value() * params_.vol_widening >> 10);

        const int64_t skew =
            static_cast<int64_t>(inv.position) *
            params_.inventory_skew;

        const int64_t half = spread >> 1;

        Quote q;
        q.bid_px = mid - half - skew;
        q.ask_px = mid + half - skew;

        q.bid_px = align_down(q.bid_px);
        q.ask_px = align_up(q.ask_px);

        q.bid_qty = qty_for_side(inv.position, true);
        q.ask_qty = qty_for_side(inv.position, false);

        return q;
    }

private:
    MMParams params_;
    VolatilityEMA vol_{};
    int64_t last_mid_{0};

    inline int64_t align_down(int64_t px) const noexcept {
        return px - (px % params_.tick_size);
    }

    inline int64_t align_up(int64_t px) const noexcept {
        const int64_t r = px % params_.tick_size;
        return r ? px + (params_.tick_size - r) : px;
    }

    inline int32_t qty_for_side(
        int32_t pos,
        bool is_bid
    ) const noexcept
    {
        if (is_bid && pos >= params_.max_position)
            return 0;
        if (!is_bid && pos <= -params_.max_position)
            return 0;

        return params_.base_qty;
    }
};
```

---

## Пример использования

```cpp
MMParams params{
    .tick_size = 1,
    .base_spread = 4,
    .base_qty = 10,
    .max_position = 100,
    .inventory_skew = 1,
    .vol_widening = 2
};

MarketMakingStrategy mm(params);

MarketState mkt{1000, 1002, 5, 6};
Inventory inv{12, 100};

Quote q = mm.on_market(mkt, inv);
```

---

## HFT-свойства реализации

- Нет virtual dispatch
- Нет heap-аллокаций
- Inline hot-path
- Минимальное ветвление
- Tick-aligned цены
- Inventory-aware котирование

---

## Roadmap

1. Queue position model
2. Microprice вместо mid-price
3. Asymmetric spreads
4. Order lifetime (cancel / replace)
5. Alpha signals (order book imbalance)
6. Kill-switch latency
7. Интеграция с Exchange Simulator

---

## Disclaimer

Модуль предназначен исключительно для образовательных и исследовательских целей.
Не используйте данный код для реальной торговли без надлежащего тестирования, аудита и лицензирования.

