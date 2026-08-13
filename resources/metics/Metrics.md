Для HFT я бы разделил метрики на **6 уровней**. И важный момент: обычные CPU%, RAM%, network throughput — полезны, но **они не являются главными метриками эффективности HFT**. Главный вопрос HFT: *сколько времени занимает обработка события, насколько стабильно это время и сколько полезных торговых действий система совершает за единицу времени.*

![Image](https://images.openai.com/static-rsc-4/ihxVYLndLtD3LPQSjaTSjF7l6CEai6AkHqqCecpTXJ9me1YIkGEAYxPdltmDTFFDj2A1QrRw-WxkzMrd10oGngGyhU9KhReqdLvbFa2pxIiIRjh1WAit5uOudJyu_RYaniRn8PsSAMWSNJKkuhttDBHC2-mnW6-55EI_7zpAP73BgGt_KANUJQ0-yns6C44J?purpose=fullsize)

![Image](https://images.openai.com/static-rsc-4/rv70DSC0XVphn56qHuXFBKYmXSj1h9Cj_0dsYer9Cwij2fE9PMKSUCaYTk9d5Ape4apP39JkbKxP56HB6udRu-raXQBiCMlf9x0ZbJo1qc4CYzf-h9t3uBESYLjDNUYr0ds-m8WMW5vFqzEV4VwTsokab3-7cs4bgRm6GaI2SHIGpyGS36Has_vRJpxC5wqK?purpose=fullsize)

![Image](https://images.openai.com/static-rsc-4/qDpnJlyi30HWHhz8kYd6r7mLOL_x4ryCPx1e00dOT7p2tCf04CCzYvu0x-k7K8U78Z987d8wuDXbKT5Sd3FlXblci-bcWIaBLCaNRHEgi54E1_hwMzN9Nq0vQ7xOBRp2CkDIFW3YA-0Tswob1bAO_1Qvkp-dbJ9Tg2Nu_lXuk9Txlm8lFePlQsArXJzPMf1q?purpose=fullsize)

![Image](https://images.openai.com/static-rsc-4/AIknwjMGEVODLiaxu_-d7TQQx2OnEKUSv_1UruuBJzD6NgdreATT9gOAZjgSISg1JGfdH45qMtIVj6fTYGatJTE9J65Vy2u3JIO6CxCOIwQzdaF7OJEhtIdexZ5J-Tqld89CQBfR8kBfg9WBDCH3DQm5c2OdY7aQuosX2OE1YOZIv5UR-iRgnEhtSi9-g9dk?purpose=fullsize)

![Image](https://images.openai.com/static-rsc-4/mcEwkhZG4gjTUWrVDx9F_3yRSQb_UCb88Cgz4N_Jh6vjuNkqc0BjQIQiZOBrFIXVPgcGMqtx9qjoAGgbafDZdFc2ymN6mOT_A5YktqJbclR-R8cj5h11i2UkLY_mcWgMqRZ25Qb4JQBF99dE84LZ3DRlARUxHhvFH_waIjGXVGlxN6fAB6xVZUbdwNZ15kJ-?purpose=fullsize)

![Image](https://images.openai.com/static-rsc-4/UFg8i89F6W7uHDRS21zSG-2W_3TpMQpWghqc4w-Ue_IKCD4Akufcb3tAq9550ERVk1CJI55OzATyHCaTw5iqKm8Bk9ZKPYGLh2pz2QdzhZ2-Z7OCNHUdFT8vlgmBn7Rl2ZMwk2K7WaxXyXElTAp16cy68mr6hMqFcgNzTI5I2XgYKw-3FRyTB3Tq_NzNvbZx?purpose=fullsize)

## 1. Latency — метрика №1

В HFT почти всегда нужно смотреть не просто на average latency, а на **распределение latency**.

Например:

```text
Market Data
    ↓
Decode
    ↓
Strategy
    ↓
Risk
    ↓
Order Creation
    ↓
Network
    ↓
Exchange
```

Для каждого участка можно измерять:

```text
T_market_data_received
T_decode
T_strategy
T_risk
T_order_encode
T_send
T_exchange_ack
```

И получать:

```text
market-data → decision
decision     → order-send
order-send   → exchange-ack
market-data  → exchange-ack
```

### Какие значения смотреть

Не только:

```text
average = 3.2 µs
```

а:

```text
min
p50
p90
p95
p99
p99.9
p99.99
max
```

Например:

```text
Strategy latency

p50      0.7 µs
p90      0.9 µs
p99      1.4 µs
p99.9    4.8 µs
max     73.2 µs
```

Последние две цифры могут быть гораздо важнее среднего.

### Почему tail latency настолько важен?

Допустим:

```text
99.99% = 1 µs
0.01%  = 100 µs
```

Если редкие 100 µs случаются именно во время резкого движения рынка, именно они могут определять P&L.

Поэтому для HFT я бы считал **p99.9/p99.99 одной из ключевых operational metrics**.

---

# 2. Jitter

Очень близкая, но отдельная характеристика.

Если система делает:

```text
1.1 µs
1.2 µs
1.1 µs
1.3 µs
1.2 µs
```

это великолепно.

Если:

```text
0.8 µs
1.0 µs
1.2 µs
8.4 µs
1.1 µs
42 µs
```

среднее может выглядеть вполне прилично, но система для HFT уже проблемная.

Поэтому нужно мониторить:

```text
latency variance
latency standard deviation
tail latency
latency spikes
```

Особенно важно понимать **причину spikes**:

```text
scheduler
interrupt
page fault
cache miss
NUMA
lock contention
allocation
GC   // если применимо
network queue
NIC interrupt
CPU frequency change
```

---

# 3. Throughput

Следующий огромный блок.

Для market-data engine:

```text
messages/sec
packets/sec
updates/sec
symbols/sec
```

Например:

```text
Market data:
    2.5 M msgs/sec

Peak:
    8.7 M msgs/sec
```

Для order subsystem:

```text
orders/sec
cancels/sec
replaces/sec
acks/sec
rejects/sec
fills/sec
```

Причём важно смотреть не только average throughput, а:

```text
current
average
peak
sustained
burst
```

### Очень важная метрика

**Capacity headroom**

Например:

```text
Current:       3.2 M msg/s
Peak:          6.1 M msg/s
Capacity:      8.0 M msg/s
```

То есть:

```text
Headroom = 1.9 M msg/s
```

Если capacity всего на 5% выше текущего peak — система уже находится в опасной зоне.

---

# 4. Market Data Metrics

Для HFT это отдельная категория.

### Message rate

```text
msgs/sec
packets/sec
updates/sec
```

### Feed latency

Например:

```text
Exchange timestamp
        ↓
NIC receive
        ↓
Application
```

Можно измерять:

```text
feed latency = T_local_receive - T_exchange_timestamp
```

И смотреть:

```text
p50
p99
p99.9
max
```

---

## Packet loss

Очень важная метрика:

```text
packet loss %
sequence gaps
missing messages
duplicate messages
out-of-order messages
```

Например:

```text
Expected sequence:

10001
10002
10003
10004
10005

Received:

10001
10002
10004
10005
```

→ gap.

Для multicast market data это особенно критично.

---

# 5. Order / Execution Metrics

Здесь уже начинается связь технических характеристик с реальной торговлей.

### Order-to-Ack latency

```text
send order
    ↓
exchange
    ↓
ack
```

Измеряем:

```text
p50
p99
p99.9
```

---

### Order-to-Fill latency

Ещё интереснее:

```text
order sent
     ↓
order accepted
     ↓
order filled
```

---

### Reject rate

```text
rejects / total orders
```

Например:

```text
Orders:   10,000,000
Rejects:       3,200

Reject rate = 0.032%
```

---

### Cancel success / reject

Для некоторых стратегий это чрезвычайно важно.

```text
cancel success
cancel reject
cancel latency
```

---

# 6. Queue Position

Для HFT это уже **очень domain-specific метрика**, но иногда одна из самых важных.

Например, стратегия поставила:

```text
BUY 100 @ 100.00
```

а перед нами в очереди:

```text
12,500 shares
```

Тогда важно оценивать:

```text
queue position
queue ahead
queue depletion rate
estimated fill probability
```

То есть latency сама по себе ещё ничего не гарантирует.

Можно иметь:

```text
latency = 800 ns
```

но постоянно оказываться в хвосте очереди.

---

# 7. CPU Metrics

Теперь переходим к hardware.

Для HFT обычный:

```text
CPU utilization = 80%
```

почти бесполезен без контекста.

Нам гораздо интереснее:

### CPU cycles

```text
cycles/message
cycles/order
cycles/market-update
```

Например:

```text
Strategy A:
    420 cycles/update

Strategy B:
    180 cycles/update
```

Это часто гораздо информативнее CPU%.

---

## Instructions per cycle

Можно смотреть:

```text
IPC
```

например:

```text
IPC = 2.4
```

Плохой IPC может указывать на:

```text
cache misses
branch misprediction
dependency chains
memory stalls
```

---

# 8. Cache Metrics

Для low-latency C++ это очень важно.

Минимальный набор:

```text
L1 cache miss
L2 cache miss
LLC miss
DTLB miss
ITLB miss
```

Особенно:

```text
LLC misses / instruction
LLC misses / message
```

Например, структура данных может выглядеть совершенно нормально с точки зрения алгоритмической сложности:

```cpp
std::map
```

но приводить к большому количеству:

```text
pointer chasing
cache misses
branch mispredictions
```

и проигрывать contiguous data structure.

---

# 9. Branch Prediction

Ещё одна интересная hardware metric:

```text
branch instructions
branch misses
branch-miss rate
```

Для HFT:

```cpp
if (...)
    ...
else
    ...
```

может выполняться миллионы раз в секунду.

Если branch prediction плохой, latency начинает плавать.

---

# 10. Memory / Allocation

Очень важный блок для C++.

Нужно смотреть:

```text
allocations/sec
deallocations/sec
bytes allocated/sec
allocation latency
```

Но для HFT ещё важнее:

```text
allocation on hot path = 0
```

Например:

```cpp
std::vector
std::string
std::shared_ptr
std::function
```

сами по себе не являются "плохими", но нужно понимать, возникают ли:

```text
heap allocation
reallocation
reference counting
indirection
```

на critical path.

---

# 11. NUMA

Если машина multi-socket:

```text
CPU 0-31 → NUMA node 0
CPU 32-63 → NUMA node 1
```

нужно мониторить:

```text
local memory access
remote memory access
NUMA misses
remote DRAM traffic
```

Очень неприятная ситуация:

```text
NIC
 ↓
NUMA 0

application
 ↓
CPU on NUMA 1
```

В результате появляется дополнительная latency и jitter.

---

# 12. Scheduling / OS

Для low latency особенно интересны:

```text
context switches
CPU migrations
interrupts
softirqs
page faults
```

Особенно:

```text
context switches/sec
CPU migrations/sec
minor page faults
major page faults
```

На critical path хотелось бы видеть:

```text
major page faults = 0
```

и желательно минимальное количество:

```text
minor page faults
```

---

# 13. Network Metrics

Для HFT network stack — огромная часть системы.

Минимальный набор:

```text
RX packets/sec
TX packets/sec
RX drops
TX drops
packet errors
NIC queue utilization
NIC ring utilization
```

И latency:

```text
NIC → application
application → NIC
```

---

# 14. Kernel / NIC latency

Если нужна очень серьёзная оптимизация:

```text
NIC timestamp
kernel timestamp
application timestamp
```

можно построить:

```text
NIC RX
  ↓
kernel
  ↓
socket
  ↓
application
```

и отдельно измерить каждый участок.

Например:

```text
NIC → kernel       0.4 µs
kernel → userspace 1.1 µs
userspace → strategy 0.7 µs
strategy → NIC     0.5 µs
```

Теперь становится понятно, **где именно находится latency budget**.

---

# 15. Lock Contention

Для HFT это одна из наиболее неприятных вещей.

Мониторить:

```text
lock acquisitions
lock contention
lock wait time
lock hold time
```

Например:

```text
mutex wait:
p50   = 0 ns
p99   = 0 ns
p99.9 = 8 µs
```

Это может быть катастрофой.

Поэтому в hot path часто стремятся к:

```text
lock-free
wait-free
single-producer/single-consumer
per-core data
```

или вообще:

```text
single-thread ownership
```

---

# 16. GC / Runtime

Если система не полностью C++, тогда добавляются:

```text
GC pause
allocation
heap growth
runtime safepoints
```

Для C++ это обычно заменяется мониторингом:

```text
heap allocation
allocator contention
page faults
```

---

# 17. Trading / Business Metrics

И вот здесь очень важный момент.

**Технически быстрая система не обязательно является эффективной торговой системой.**

Поэтому обязательно нужно связывать performance metrics с trading metrics.

Например:

```text
P&L
PnL / trade
PnL / message
PnL / unit latency
Sharpe
fill rate
reject rate
cancel rate
slippage
adverse selection
maker/taker ratio
```

---

## Latency → P&L

Очень интересная метрика:

```text
PnL as a function of latency
```

Например:

```text
Latency < 2 µs       → +$120k
2-5 µs               → +$35k
5-10 µs              → -$20k
>10 µs               → -$80k
```

Это намного полезнее простого:

```text
average latency = 2.8 µs
```

Потому что показывает **экономическую стоимость latency**.

---

# 18. Adverse Selection

Для market making очень важна ситуация:

```text
мы поставили bid
        ↓
нас заполнили
        ↓
рынок сразу пошёл вниз
```

То есть fill оказался плохим.

Можно измерять:

```text
mid-price after 1 µs
mid-price after 10 µs
mid-price after 100 µs
mid-price after 1 ms
```

относительно цены нашего fill.

Это позволяет оценивать:

```text
adverse selection
```

и понимать, насколько стратегия действительно эффективна.

---

# 19. Самые важные метрики — мой TOP

Если построить **HFT Performance Dashboard**, я бы начал примерно с этого:

| Категория       | Метрика            | Важность |
| --------------- | ------------------ | -------: |
| Latency         | p50                |      ⭐⭐⭐ |
| Latency         | p99                |    ⭐⭐⭐⭐⭐ |
| Latency         | p99.9 / p99.99     |    ⭐⭐⭐⭐⭐ |
| Latency         | max / spikes       |    ⭐⭐⭐⭐⭐ |
| Jitter          | latency variance   |    ⭐⭐⭐⭐⭐ |
| Market Data     | messages/sec       |     ⭐⭐⭐⭐ |
| Market Data     | packet loss / gaps |    ⭐⭐⭐⭐⭐ |
| Execution       | order → ack        |    ⭐⭐⭐⭐⭐ |
| Execution       | order → fill       |     ⭐⭐⭐⭐ |
| Execution       | reject rate        |     ⭐⭐⭐⭐ |
| CPU             | cycles/message     |    ⭐⭐⭐⭐⭐ |
| CPU             | IPC                |     ⭐⭐⭐⭐ |
| Cache           | LLC miss           |     ⭐⭐⭐⭐ |
| CPU             | branch miss        |      ⭐⭐⭐ |
| Memory          | allocations/sec    |     ⭐⭐⭐⭐ |
| NUMA            | remote memory      |     ⭐⭐⭐⭐ |
| OS              | context switches   |     ⭐⭐⭐⭐ |
| OS              | page faults        |     ⭐⭐⭐⭐ |
| Network         | NIC → application  |    ⭐⭐⭐⭐⭐ |
| Network         | application → NIC  |    ⭐⭐⭐⭐⭐ |
| Synchronization | lock wait          |    ⭐⭐⭐⭐⭐ |
| Trading         | fill rate          |     ⭐⭐⭐⭐ |
| Trading         | slippage           |    ⭐⭐⭐⭐⭐ |
| Trading         | adverse selection  |    ⭐⭐⭐⭐⭐ |
| Trading         | P&L vs latency     |    ⭐⭐⭐⭐⭐ |

---

# 20. Самое главное: смотреть не отдельные метрики, а цепочку

Для HFT я бы вообще мыслил не так:

```text
CPU = 70%
RAM = 40%
Network = 20%
```

а так:

```text
Market Event
     │
     ▼
NIC RX
     │
     │  400 ns
     ▼
Market Data Parser
     │
     │  300 ns
     ▼
Order Book
     │
     │  150 ns
     ▼
Strategy
     │
     │  500 ns
     ▼
Risk
     │
     │  100 ns
     ▼
Order Encoder
     │
     │  300 ns
     ▼
NIC TX
     │
     ▼
Exchange
```

И для **каждого участка** иметь:

```text
p50
p99
p99.9
p99.99
max
cycles
```

А затем связывать это с:

```text
Fill
    ↓
Slippage
    ↓
Adverse Selection
    ↓
P&L
```

Получается причинная цепочка:

**Market Data → Processing Latency → Order Latency → Queue Position → Fill → Slippage → P&L**

И именно эта цепочка, на мой взгляд, является правильной моделью мониторинга HFT.

### Если смотреть именно глазами C++ Low-Latency разработчика

Я бы выделил **10 метрик первого приоритета**:

```text
1. p99.9 end-to-end latency
2. p99.9 latency каждого этапа pipeline
3. latency jitter
4. messages/sec + peak burst
5. packet loss / sequence gaps
6. cycles/message
7. LLC cache misses
8. context switches / CPU migrations
9. lock contention
10. P&L / slippage / adverse selection vs latency
```