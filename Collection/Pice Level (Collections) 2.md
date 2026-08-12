Если контекст именно **C++ + FinTech + Low Latency**, то выбор структуры для `Price Level` сильно зависит от того, какие операции являются hot path.

Для order book обычно есть структура примерно такого вида:

```cpp
struct PriceLevel
{
    Price price;
    Quantity quantity;
    uint32_t orderCount;
};
```

и нам нужно быстро:

* найти уровень по `price`;
* добавить/изменить/удалить уровень;
* получить `best bid` / `best ask`;
* пройти уровни от лучшей цены к худшей;
* иногда получить N лучших уровней.

### Короткий ответ

Для low-latency order book я бы рассматривал в первую очередь:

| Структура               |   Lookup | Best price | Ordered iteration |    Allocations | Когда использовать           |
| ----------------------- | -------: | ---------: | ----------------: | -------------: | ---------------------------- |
| `std::map`              | O(log N) |       O(1) |           отлично |          много | простой вариант              |
| `std::unordered_map`    |    O(1)* |      плохо |               нет |         средне | lookup важнее порядка        |
| `std::vector`           |     O(N) |    отлично |           отлично |           мало | маленькое число levels       |
| sorted `std::vector`    | O(log N) |    отлично |           отлично |           мало | небольшая/средняя книга      |
| flat hash map           |    ~O(1) |   отдельно |               нет |           мало | очень быстрый lookup         |
| array/direct addressing |     O(1) |    отлично |           отлично |            нет | **фиксированный tick range** |
| intrusive tree          | O(log N) |    отлично |           отлично | контролируемые | production low-latency       |

* в среднем.

---

# 1. Самый интересный вариант — array/direct addressing

Если цены имеют дискретный tick:

```text
100.00
100.01
100.02
100.03
...
```

то вообще не обязательно использовать associative container.

Можно сделать:

```cpp
std::vector<PriceLevel> levels;
```

где индекс напрямую соответствует цене:

```cpp
index = price / tick_size;
```

Например:

```cpp
constexpr int MIN_PRICE = 10000;
constexpr int MAX_PRICE = 20000;

std::array<PriceLevel, MAX_PRICE - MIN_PRICE + 1> levels;
```

Тогда:

```cpp
levels[price - MIN_PRICE]
```

это **O(1)** без hash, без tree traversal и практически идеально для cache locality.

Это один из самых привлекательных вариантов для low-latency.

---

# 2. Но есть проблема с огромным price range

Например:

```text
BTC:      50,000
AAPL:       200
Futures:  5,000
```

Если диапазон огромный, прямой массив становится дорогим.

Тогда можно использовать **centered array**:

```text
                 current price
                       ↓
        [ ... | ... | 100 | ... | ... ]
```

Храним только небольшой диапазон вокруг текущей цены.

Например:

```cpp
constexpr size_t LEVELS = 4096;

std::array<PriceLevel, LEVELS> levels;
```

и поддерживаем:

```cpp
basePrice
```

При выходе цены за диапазон можно сдвинуть окно.

Это уже довольно специализированная реализация order book, но для конкретного инструмента может быть очень эффективной.

---

# 3. `std::map`

Классический вариант:

```cpp
std::map<Price, PriceLevel> levels;
```

Например:

```cpp
std::map<Price, PriceLevel, std::greater<>> bids;
std::map<Price, PriceLevel, std::less<>> asks;
```

Получаем:

```cpp
bids.begin(); // best bid
asks.begin(); // best ask
```

И уровни уже отсортированы.

Плюсы:

* O(log N) lookup;
* O(log N) insert/erase;
* всегда sorted;
* очень простой код.

Но для **low latency** есть существенные минусы:

### Node allocation

Каждый элемент `std::map` обычно является отдельным node.

То есть:

```text
PriceLevel
   ↓
tree node
   ↓
heap allocation
```

Это плохо для:

* cache locality;
* allocator overhead;
* tail latency;
* predictable performance.

Поэтому `std::map` — хороший **reference implementation**, но не обязательно хороший production choice для HFT/low-latency.

---

# 4. `std::unordered_map`

Можно сделать:

```cpp
std::unordered_map<Price, PriceLevel> levels;
```

Lookup:

```cpp
auto it = levels.find(price);
```

В среднем:

```text
O(1)
```

Это хорошо.

Но:

```cpp
bestBid
bestAsk
```

получить напрямую нельзя.

Придётся отдельно хранить:

```cpp
Price bestBid;
Price bestAsk;
```

и поддерживать их.

Кроме того:

* hash;
* buckets;
* rehash;
* memory indirection;
* плохая locality.

Поэтому `unordered_map` не всегда выигрывает у более cache-friendly структур.

---

# 5. Очень интересный вариант — sorted vector

Например:

```cpp
std::vector<PriceLevel> levels;
```

с уровнями:

```text
100.00
100.01
100.03
100.05
```

и поддержанием sorted order.

Поиск:

```cpp
std::lower_bound(...)
```

даёт:

```text
O(log N)
```

Но самое интересное — **cache locality**.

В отличие от `std::map`:

```text
map:

node -> node -> node -> node
  ↓      ↓      ↓
heap   heap   heap
```

vector:

```text
[ level ][ level ][ level ][ level ][ level ]
```

CPU cache очень любит второй вариант.

Для небольшого количества price levels `vector` может оказаться **быстрее `std::map`**, несмотря на теоретическую сложность.

---

# 6. `std::vector` особенно хорош для небольшого depth

Допустим, тебя интересуют только:

```text
Top 10
Top 20
Top 50
```

levels.

Тогда можно вообще хранить:

```cpp
std::array<PriceLevel, 32>
```

или:

```cpp
std::vector<PriceLevel>
```

и поддерживать порядок вручную.

При 10–50 уровнях:

```text
O(N)
```

может оказаться быстрее, чем:

```text
O(log N)
```

из-за cache locality и отсутствия pointer chasing.

Это важный момент в low-latency C++:

> **Big-O не рассказывает всю историю.**

---

# 7. Flat containers

Для low-latency я бы обязательно посмотрел на концепцию **flat map**.

Идея:

```text
keys:   [100][101][102][103][104]
values: [ L ][ L ][ L ][ L ][ L ]
```

вместо tree nodes.

Например, существуют реализации:

* `boost::container::flat_map`
* `absl::flat_hash_map`
* `folly::F14`
* `ankerl::unordered_dense`

Для price levels `flat_map` особенно интересен, когда:

* уровней относительно немного;
* нужны ordered levels;
* важна cache locality.

---

# 8. Intrusive tree

Если нужен настоящий ordered tree, но хочется контролировать allocation, можно использовать intrusive structures.

Например концептуально:

```cpp
struct PriceLevel
{
    Price price;
    Quantity quantity;

    TreeNode treeNode;
};
```

То есть node является частью самого `PriceLevel`.

Получается:

```text
PriceLevel
+----------------+
| price          |
| quantity       |
| treeNode       |
+----------------+
```

а не:

```text
Tree node
+----------------+
| pointers       |
| PriceLevel*    |
+----------------+
        |
        v
 PriceLevel
```

Это позволяет:

* убрать отдельные allocations;
* контролировать memory lifetime;
* улучшить locality;
* сделать latency более predictable.

Для серьёзного low-latency matching engine это уже очень интересный подход.

---

# 9. Отдельно хранить `bestBid` / `bestAsk`

Я бы практически всегда рассматривал это независимо от основной структуры.

Например:

```cpp
struct OrderBookSide
{
    Price bestPrice;
    ...
};
```

Потому что операция:

```cpp
getBestBid()
```

должна быть максимально дешёвой.

Идеально:

```cpp
[[nodiscard]]
Price bestBid() const noexcept
{
    return m_bestBid;
}
```

а не каждый раз искать его в container.

---

# 10. Ещё важнее: Price лучше не хранить как `double`

Для FinTech это принципиально.

Не:

```cpp
double price;
```

а, например:

```cpp
using Price = int64_t;
```

где цена хранится в минимальных units:

```text
100.01 USD

=> 10001 ticks
```

или:

```cpp
using Price = int64_t;
constexpr Price TICK_SIZE = 1;
```

Тогда:

```cpp
Price price = 10001;
```

и сравнение:

```cpp
price < otherPrice
```

абсолютно детерминированное.

И это очень хорошо сочетается с array/direct addressing.

---

# Что бы я выбрал

Если проектировать **low-latency order book**, я бы рассматривал архитектуру примерно так:

```text
                     Order Book
                         │
             ┌───────────┴───────────┐
             │                       │
           Bids                     Asks
             │                       │
       descending                ascending
             │                       │
       Price Levels              Price Levels
             │                       │
       ┌─────┴─────┐           ┌─────┴─────┐
       │           │           │           │
    lookup       order       lookup       order
       │           │           │           │
     array       vector      array       vector
```

Если диапазон цен ограничен:

### 🥇 Direct-addressed array

```cpp
std::array<PriceLevel, N>
```

Если диапазон большой:

### 🥈 Flat/sorted vector

```cpp
std::vector<PriceLevel>
```

Если нужны произвольные цены и большой book:

### 🥉 Intrusive ordered tree

```text
intrusive RB-tree / AVL / similar
```

А `std::map` я бы оставил прежде всего для:

* первой реализации;
* reference implementation;
* тестов;
* не-hot-path компонентов.

---

## И ещё один очень важный момент

Для **Price Level** структура данных — только половина задачи.

Реальный order book обычно имеет два уровня:

```text
Price
  │
  ▼
PriceLevel
  │
  ├── Order
  ├── Order
  ├── Order
  └── Order
```

Например:

```cpp
struct PriceLevel
{
    Price price;
    Quantity totalQuantity;

    std::list<Order> orders;
};
```

Но `std::list<Order>` для low latency я бы тоже **не выбирал автоматически**.

Там возникает ещё более интересная задача:

> **какую структуру использовать для очереди Orders внутри Price Level, чтобы поддерживать FIFO, cancel/replace и match без heap allocation?**

И вот здесь уже появляются **intrusive linked lists, object pools, slab allocators, preallocated arrays и индексированные free-lists** — и это, на мой взгляд, гораздо интереснее для обсуждения именно в контексте Low Latency.
