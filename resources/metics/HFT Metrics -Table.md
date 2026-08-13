### HFT Metrics Overview

| Категория                         | Метрика                    | Что измеряет                                   | Приоритет |
| --------------------------------- | -------------------------- | ---------------------------------------------- | --------: |
| **Latency**                       | Market Data → Strategy     | Скорость реакции стратегии на market event     |     ⭐⭐⭐⭐⭐ |
|                                   | Market Data → Order Send   | Полное время от market event до отправки order |     ⭐⭐⭐⭐⭐ |
|                                   | Market Data → Exchange     | End-to-End reaction latency                    |     ⭐⭐⭐⭐⭐ |
|                                   | Order Send → Exchange      | Network / transport latency                    |     ⭐⭐⭐⭐⭐ |
|                                   | Order Send → ACK           | Время подтверждения order биржей               |      ⭐⭐⭐⭐ |
|                                   | Order → Fill               | Время до исполнения                            |       ⭐⭐⭐ |
|                                   | p50 / p99 / p99.9 / p99.99 | Распределение latency                          |     ⭐⭐⭐⭐⭐ |
|                                   | Max / Spike Rate           | Редкие latency anomalies                       |      ⭐⭐⭐⭐ |
| **Jitter**                        | Latency variance           | Стабильность latency                           |     ⭐⭐⭐⭐⭐ |
|                                   | Tail latency               | Поведение latency в хвосте распределения       |     ⭐⭐⭐⭐⭐ |
|                                   | Latency spikes/sec         | Частота резких скачков                         |     ⭐⭐⭐⭐⭐ |
| **Market Data**                   | Messages/sec               | Скорость входящего market data                 |     ⭐⭐⭐⭐⭐ |
|                                   | Packets/sec                | Network packet rate                            |      ⭐⭐⭐⭐ |
|                                   | Peak message rate          | Максимальная нагрузка                          |     ⭐⭐⭐⭐⭐ |
|                                   | Burst rate                 | Способность переживать короткие bursts         |     ⭐⭐⭐⭐⭐ |
|                                   | Sequence gaps              | Потерянные market-data messages                |     ⭐⭐⭐⭐⭐ |
|                                   | Duplicate messages         | Дубликаты сообщений                            |       ⭐⭐⭐ |
|                                   | Out-of-order messages      | Нарушение порядка сообщений                    |      ⭐⭐⭐⭐ |
|                                   | Feed latency               | Exchange timestamp → local receive             |     ⭐⭐⭐⭐⭐ |
| **Order / Execution**             | Orders/sec                 | Производительность order subsystem             |      ⭐⭐⭐⭐ |
|                                   | Cancels/sec                | Скорость cancel operations                     |      ⭐⭐⭐⭐ |
|                                   | Replaces/sec               | Скорость modification operations               |       ⭐⭐⭐ |
|                                   | Order reject rate          | Доля rejected orders                           |     ⭐⭐⭐⭐⭐ |
|                                   | Cancel reject rate         | Доля rejected cancels                          |      ⭐⭐⭐⭐ |
|                                   | Fill rate                  | Доля исполненных orders                        |     ⭐⭐⭐⭐⭐ |
|                                   | Partial fill rate          | Частота частичных исполнений                   |       ⭐⭐⭐ |
|                                   | Queue position             | Позиция в очереди биржи                        |     ⭐⭐⭐⭐⭐ |
| **CPU**                           | CPU utilization            | Загрузка CPU                                   |       ⭐⭐⭐ |
|                                   | Cycles/message             | CPU cost обработки одного message              |     ⭐⭐⭐⭐⭐ |
|                                   | Cycles/order               | CPU cost одного order                          |      ⭐⭐⭐⭐ |
|                                   | IPC                        | Instructions per cycle                         |      ⭐⭐⭐⭐ |
|                                   | CPU frequency              | Текущая частота CPU                            |      ⭐⭐⭐⭐ |
|                                   | Core utilization           | Загрузка конкретных cores                      |      ⭐⭐⭐⭐ |
| **Cache / CPU Microarchitecture** | L1 miss rate               | L1 cache efficiency                            |       ⭐⭐⭐ |
|                                   | L2 miss rate               | L2 cache efficiency                            |      ⭐⭐⭐⭐ |
|                                   | LLC miss rate              | Last-level cache efficiency                    |     ⭐⭐⭐⭐⭐ |
|                                   | Branch miss rate           | Качество branch prediction                     |      ⭐⭐⭐⭐ |
|                                   | TLB miss rate              | Эффективность address translation              |       ⭐⭐⭐ |
|                                   | Memory stalls              | CPU stalls из-за memory                        |      ⭐⭐⭐⭐ |
| **Memory**                        | Allocations/sec            | Частота dynamic allocations                    |      ⭐⭐⭐⭐ |
|                                   | Allocation latency         | Стоимость allocation                           |      ⭐⭐⭐⭐ |
|                                   | Bytes allocated/sec        | Memory allocation throughput                   |       ⭐⭐⭐ |
|                                   | Reallocations              | Нежелательные reallocations                    |      ⭐⭐⭐⭐ |
|                                   | RSS / working set          | Используемая память                            |        ⭐⭐ |
| **NUMA**                          | Remote memory accesses     | Доступ к remote NUMA node                      |     ⭐⭐⭐⭐⭐ |
|                                   | Local / remote ratio       | Локальность памяти                             |      ⭐⭐⭐⭐ |
|                                   | NUMA migrations            | Перемещения между nodes                        |       ⭐⭐⭐ |
| **OS / Scheduler**                | Context switches           | Переключения threads                           |     ⭐⭐⭐⭐⭐ |
|                                   | CPU migrations             | Перемещение thread между cores                 |     ⭐⭐⭐⭐⭐ |
|                                   | Page faults                | Memory page faults                             |      ⭐⭐⭐⭐ |
|                                   | Major page faults          | Disk-backed faults                             |     ⭐⭐⭐⭐⭐ |
|                                   | Interrupts                 | Hardware interrupts                            |      ⭐⭐⭐⭐ |
|                                   | Softirqs                   | Kernel soft interrupts                         |      ⭐⭐⭐⭐ |
| **Synchronization**               | Lock contention            | Конкуренция за locks                           |     ⭐⭐⭐⭐⭐ |
|                                   | Lock wait time             | Время ожидания lock                            |     ⭐⭐⭐⭐⭐ |
|                                   | Lock hold time             | Время удержания lock                           |      ⭐⭐⭐⭐ |
|                                   | Atomic operations          | Частота atomic synchronization                 |       ⭐⭐⭐ |
| **Network / NIC**                 | RX packets/sec             | Входящий network traffic                       |      ⭐⭐⭐⭐ |
|                                   | TX packets/sec             | Исходящий traffic                              |      ⭐⭐⭐⭐ |
|                                   | RX drops                   | Потерянные RX packets                          |     ⭐⭐⭐⭐⭐ |
|                                   | TX drops                   | Потерянные TX packets                          |      ⭐⭐⭐⭐ |
|                                   | NIC → Application          | RX path latency                                |     ⭐⭐⭐⭐⭐ |
|                                   | Application → NIC          | TX path latency                                |     ⭐⭐⭐⭐⭐ |
|                                   | NIC queue utilization      | Загрузка NIC queues                            |      ⭐⭐⭐⭐ |
|                                   | Ring buffer utilization    | Заполнение NIC rings                           |      ⭐⭐⭐⭐ |
| **Trading / Business**            | P&L                        | Финансовый результат                           |     ⭐⭐⭐⭐⭐ |
|                                   | P&L / trade                | Эффективность сделки                           |     ⭐⭐⭐⭐⭐ |
|                                   | P&L / message              | Экономическая ценность обработки market data   |      ⭐⭐⭐⭐ |
|                                   | Slippage                   | Отклонение от ожидаемой цены                   |     ⭐⭐⭐⭐⭐ |
|                                   | Adverse selection          | Насколько fills оказываются невыгодными        |     ⭐⭐⭐⭐⭐ |
|                                   | Fill ratio                 | Эффективность выставления orders               |     ⭐⭐⭐⭐⭐ |
|                                   | Maker / Taker ratio        | Структура execution                            |       ⭐⭐⭐ |
|                                   | P&L vs latency             | Экономическая стоимость latency                |     ⭐⭐⭐⭐⭐ |
| **Capacity / Resilience**         | Sustained throughput       | Долговременная пропускная способность          |     ⭐⭐⭐⭐⭐ |
|                                   | Peak throughput            | Максимальная пропускная способность            |     ⭐⭐⭐⭐⭐ |
|                                   | Capacity headroom          | Запас производительности                       |     ⭐⭐⭐⭐⭐ |
|                                   | Queue depth                | Очереди внутри системы                         |      ⭐⭐⭐⭐ |
|                                   | Dropped events             | Потерянные события при нагрузке                |     ⭐⭐⭐⭐⭐ |
|                                   | Recovery time              | Время восстановления после overload/failure    |      ⭐⭐⭐⭐ |

### Если сильно сократить

Для **первого production dashboard** я бы выделил всего несколько групп:

| Группа              | Самые важные метрики                               |
| ------------------- | -------------------------------------------------- |
| **Latency**         | p50, p99, p99.9, p99.99, Market Data → Exchange    |
| **Jitter**          | tail latency, spike rate                           |
| **Market Data**     | msg/sec, peak msg/sec, sequence gaps, feed latency |
| **Execution**       | orders/sec, reject rate, fill rate, queue position |
| **CPU**             | cycles/message, IPC, core utilization              |
| **Cache**           | LLC misses, branch misses                          |
| **Memory**          | allocations, allocation latency                    |
| **NUMA**            | remote memory accesses                             |
| **OS**              | context switches, CPU migrations, page faults      |
| **Synchronization** | lock contention, lock wait time                    |
| **Network**         | NIC→App, App→NIC, RX/TX drops                      |
| **Trading**         | P&L, slippage, adverse selection, P&L vs latency   |
| **Capacity**        | peak throughput, sustained throughput, headroom    |

И я бы разделял их ещё на **три уровня**:

```text
                              HFT Performance
                                   │
          ┌────────────────────────┼──────────────────────────┐
          │                        │                          │
   Trading outcome           System behavior           Hardware efficiency
          │                        │                          │
       P&L                      Latency                      CPU
       Slippage                 Jitter                       Cache
       Fill rate                Throughput                   Memory
       Adverse                  Drops                        NUMA
       selection                Queues                       NIC
```

То есть **Latency / Throughput** показывают, насколько система быстрая, **CPU/Cache/Memory/NIC** объясняют *почему она такая быстрая или медленная*, а **P&L/Slippage/Fill Rate** показывают, *имеет ли эта производительность реальную торговую ценность*.
