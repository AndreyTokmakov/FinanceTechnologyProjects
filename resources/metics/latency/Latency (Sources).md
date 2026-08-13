1. [Ключевое правило: Аппаратный vs Софтовый замер (Дьявол в кварце)](#1-ключевое-правило-аппаратный-vs-софтовый-замер-дьявол-в-кварце)
    - 1.1. Почему std::chrono врет
    - 1.2. Как работает PTP-синхронизация
    - 1.3. Код калибровки TSC по PTP
    - 1.4. Где в C++ это реализуется

2. [Tick-to-Trade Latency (Главный KPI войны)](#2-tick-to-trade-latency-главный-kpi-войны)
    - 2.1. Анатомия сквозной задержки
    - 2.2. Аппаратные timestamp-ы Solarflare
    - 2.3. Реальный код в колбэках TX completion
    - 2.4. Почему p99.9 важнее среднего

3. [Ingress-to-Application Latency (Стоимость просыпания)](#3-ingress-to-application-latency-стоимость-просыпания)
    - 3.1. Цена системных вызовов и контекстных переключений
    - 3.2. Busy Polling vs IRQ-driven
    - 3.3. Код с recvmsg и инструкцией PAUSE
    - 3.4. Как измерить время пробуждения без шума

4. [Parsing/Decoding Latency (Битва с Branch Miss)](#4-parsingdecoding-latency-битва-с-branch-miss)
    - 4.1. Почему условные переходы убивают производительность
    - 4.2. __builtin_prefetch и __builtin_expect
    - 4.3. Zero-copy парсинг через reinterpret_cast
    - 4.4. Реальный код парсера ITCH/SBE
    - 4.5. Гистограмма распределения времени парсинга

5. [Order Entry Processing Latency (Цена шаблонов и виртуальных функций)](#5-order-entry-processing-latency-цена-шаблонов-и-виртуальных-функций)
    - 5.1. Три под-метрики: сериализация, копирование, CRC
    - 5.2. Аппаратный CRC32 через _mm_crc32_u64
    - 5.3. Pre-allocated буферы vs std::string
    - 5.4. Код энкодера с LFENCE/SFENCE
    - 5.5. Как избавиться от виртуальных функций в горячем пути

6. [Inter-Core Latency (Проклятие MESI протокола)](#6-inter-core-latency-проклятие-mesi-протокола)
    - 6.1. Что такое когерентность кэшей и MESI
    - 6.2. Почему cache line bouncing добавляет 300+ нс
    - 6.3. Micro-benchmark для измерения межъядерной задержки
    - 6.4. Код SPSC очереди с memory_order_acquire
    - 6.5. Как привязать потоки к ядрам (pthread_setaffinity_np)

7. [Где конкретно в C++ коде ставятся замеры (Карта модулей)](#7-где-конкретно-в-c-коде-ставятся-замеры-карта-модулей)
    - 7.1. Таблица: модуль → точка замера → метрика
    - 7.2. Что должно быть в горячем пути, а что нет

8. [Как НЕ НУЖНО замерять Latency в C++ (Смертельные грехи)](#8-как-не-нужно-замерять-latency-в-c-смертельные-грехи)
    - 8.1. Чек-лист запрещенных практик с объяснением
    - 8.2. Почему volatile бесполезен
    - 8.3. Ошибки с memory ordering
    - 8.4. Debug vs Release билды

9. [Эталонная архитектура замера в C++ (Production-код)](#9-эталонная-архитектура-замера-в-c-production-код)
    - 9.1. Thread-Local Storage (TLS) для счетчиков
    - 9.2. Выравнивание на 64 байта (cache line alignment)
    - 9.3. Гистограмма через __builtin_clz (бинарный логарифм)
    - 9.4. Фоновый агрегатор с отправкой по UDP
    - 9.5. Полный пример класса MetricCollector

---

## 1. Ключевое правило: Аппаратный vs Софтовый замер (Дьявол в кварце)

### 1.1. Почему std::chrono врет
`std::chrono::high_resolution_clock` в 99% реализаций — это тонкая обертка над `clock_gettime(CLOCK_MONOTONIC)` или `gettimeofday()`.

**Проблема №1:** Это VDSO-вызов. Да, он не переключает контекст в ядро, но он читает разделяемую память, которую обновляет ядро каждое прерывание таймера (обычно каждые 1 мс). В момент обновления этой страницы памяти происходит **TLB-shootdown** — процессор сбрасывает кэш трансляции адресов, и чтение `clock_gettime` может застрять на 100-200 нс.

**Проблема №2 (главная):** Дрейф частот. У CPU кварц 2.8 ГГц, у NIC свой кварц, синхронизированный по PTP от биржи с точностью до 50 нс. За 1 секунду дрейф может составить до 200 нс. Через 5 минут торговли ваши замеры уедут на 60 мкс, и вы не поймете, стали вы торговать медленнее или просто "уплыло" время.

### 1.2. Как работает PTP-синхронизация
PTP (Precision Time Protocol, IEEE 1588) работает на уровне L2 (Ethernet) и синхронизирует часы NIC с мастер-часами биржи. Ваша задача — привязать показания TSC (Time Stamp Counter) CPU к этим часам.

Делается это через **сервис phc2sys** (Linux PTP проект), который раз в несколько секунд читает время NIC через `ioctl` и корректирует частоту CPU (через `adjtimex`).

```bash
# Запуск демона синхронизации (пример)
phc2sys -s CLOCK_REALTIME -c /dev/ptp0 -O 0 -m -q
```

### 1.3. Код калибровки TSC по PTP
В C++ вы не вызываете `clock_gettime`. Вы читаете `rdtsc` (инструкция CPU, ~5 нс) и умножаете на коэффициент, который обновляется фоновым потоком калибровки.

```cpp
// Глобальные атомарные переменные (пишем из калибровщика, читаем из горячего пути)
alignas(64) static std::atomic<uint64_t> g_tsc_offset{0};
alignas(64) static std::atomic<double> g_tsc_to_ptp_ratio{1.0};

// Фоновый поток калибровки (вызывается раз в 5-10 секунд)
void calibrate_tsc_to_ptp() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(5));
        
        // Читаем PTP-время из NIC через ioctl
        struct timespec ptp_time;
        clock_gettime(CLOCK_REALTIME, &ptp_time); // Это НЕ для горячего пути!
        uint64_t ptp_ns = ptp_time.tv_sec * 1e9 + ptp_time.tv_nsec;
        
        // Читаем TSC одновременно
        uint64_t tsc_val = __rdtsc();
        
        // Вычисляем смещение и коэффициент (приближенно)
        g_tsc_offset.store(tsc_val - (ptp_ns * CPU_FREQ_GHZ));
        g_tsc_to_ptp_ratio.store(1.0); // В реальности вычисляем дрейф
    }
}

// Горячий путь: получение времени в наносекундах
inline uint64_t get_hw_timestamp_ns() {
    uint64_t tsc = __rdtsc();
    uint64_t offset = g_tsc_offset.load(std::memory_order_relaxed);
    double ratio = g_tsc_to_ptp_ratio.load(std::memory_order_relaxed);
    return (uint64_t)((tsc - offset) * ratio);
}
```

### 1.4. Где в C++ это реализуется
Этот подход используется в **базовом классе-синглтоне** `TimeKeeper`, который инжектится во все модули: Network RX, Network TX, Market Data Parser, Order Manager.

**Важно:** В горячем пути используется только `relaxed` memory ordering (без барьеров), потому что нам не нужна строгая последовательность для времени — допустимо прочитать чуть устаревший коэффициент.

---

## 2. Tick-to-Trade Latency (Главный KPI войны)

### 2.1. Анатомия сквозной задержки
Tick-to-Trade =
- T1: Время, когда **последний байт** входящего пакета записан в DMA-буфер NIC (RX timestamp).
- T2: Время, когда **первый байт** исходящего пакета покинул порт NIC (TX timestamp).

Важно: мы не учитываем время прохождения света по оптоволокну — это физика, мы не можем на неё повлиять. Но мы должны вычесть его из метрики, чтобы видеть только нашу софтовую задержку. Обычно расстояние до биржи известно, и время транзита фиксировано (~1 мкс на 200 км).

### 2.2. Аппаратные timestamp-ы Solarflare
На картах Solarflare (теперь Xilinx) есть регистры, которые аппаратно прошивают время в момент прихода/отправки пакета.

**Как получить RX timestamp:**
```cpp
// В обработчике пакета из DPDK или OpenOnload
void on_rx_packet(efx_rx_event_t* ev) {
    // Поле timestamp в событии — это PTP-время в наносекундах
    uint64_t rx_ts = ev->er_timestamp; 
    // ... сохраняем в структуру пакета вместе с order_id
}
```

**Как получить TX timestamp:**
Отправка пакета — асинхронная. Сначала вы кладете пакет в TX-кольцо NIC. Затем через 1-2 мкс приходит **TX completion event**, в котором NIC сообщает, что пакет улетел, и кладет туда timestamp отправки.

```cpp
void on_tx_completion(efx_tx_event_t* ev) {
    uint64_t tx_ts = ev->et_timestamp;
    uint64_t order_id = ev->et_user_data; // Мы сохранили ID ордера при отправке
    
    // Достаем время приема запроса на этот ордер
    uint64_t rx_ts = g_order_latency_map[order_id];
    
    // Считаем Tick-to-Trade и пишем в метрики
    uint64_t tick_to_trade = tx_ts - rx_ts;
    g_tick_to_trade_metrics.push(tick_to_trade);
}
```

### 2.3. Реальный код в колбэках TX completion

```cpp
// Класс-обертка над Solarflare NIC
class SolarflareNIC {
    static constexpr size_t TX_RING_SIZE = 4096;
    
    struct TxCompletion {
        uint64_t timestamp; // Аппаратное время отправки
        uint64_t order_id;  // ID нашего ордера
    };
    
    SPSCRing<TxCompletion, 4096> tx_completions; // SPSC очередь для комплитов
    
public:
    // Отправка ордера (горячий путь)
    void send_order(const Order& ord, const char* tx_buffer, size_t len) {
        // Сохраняем order_id в буфере дескриптора NIC перед отправкой
        uint64_t order_id = ord.id;
        efx_tx_desc_t* desc = get_next_tx_desc();
        desc->ed_data = (uintptr_t)tx_buffer;
        desc->ed_len = len;
        desc->ed_user_data = order_id; // <-- Запомнили ID для комплита
        
        // Запускаем DMA-отправку (NIC сам проставит время)
        efx_tx_qpush(desc);
        
        // Не ждем отправки — вернемся в цикл
    }
    
    // Обработчик TX completion (вызывается из прерывания или poll-а)
    void process_tx_completions() {
        efx_tx_event_t ev;
        while (efx_tx_qpoll(&ev)) {
            uint64_t tx_ts = ev.et_timestamp; // Аппаратное время!
            uint64_t order_id = ev.et_user_data;
            
            // Кладем комплит в очередь для фонового агрегатора
            tx_completions.push({tx_ts, order_id});
        }
    }
};

// Фоновый поток-агрегатор (не критичный к задержке)
void latency_aggregator() {
    while (true) {
        TxCompletion comp;
        if (tx_completions.pop(comp)) {
            uint64_t rx_ts = g_order_rx_map[comp.order_id];
            uint64_t latency = comp.timestamp - rx_ts;
            
            // Отправляем в Grafana / Prometheus через UDP
            send_udp_metric("tick_to_trade", latency);
        }
    }
}
```

### 2.4. Почему p99.9 важнее среднего
Распределение Tick-to-Trade в HFT **мультимодальное**:
- Пик 1 (50% пакетов): 2-3 мкс — данные уже в L2 кэше.
- Пик 2 (40%): 5-7 мкс — данные в L3 кэше.
- Пик 3 (9%): 10-15 мкс — данные в RAM (TLB miss).
- Хвост (1%): 50+ мкс — прерывание от ядра или шедулер.

Если вы смотрите на среднее (mean) — оно будет ~6 мкс, и вы будете думать, что всё отлично. Но ваш p99.9 = 50 мкс. Это значит, что **каждая 1000-я заявка опаздывает на 50 мкс**, и стратегия теряет деньги на этих сделках.

**Вывод:** Собирайте HDR Histogram и смотрите на p99.9, p99.99, а также на **max latency за последние 10 секунд**.

---

## 3. Ingress-to-Application Latency (Стоимость просыпания)

### 3.1. Цена системных вызовов и контекстных переключений
Когда пакет приходит на NIC:
1. **IRQ (аппаратное прерывание)** — CPU останавливает текущий поток, сохраняет контекст (~100 нс).
2. **SoftIRQ (половинка прерывания)** — ядро вызывает NAPI poll, кладет пакет в сокет-буфер (~200-500 нс).
3. **Пробуждение пользовательского процесса** — если поток спал на `epoll` или `select`, шедулер ставит его в очередь на выполнение (может быть 1 мс +, если ядро загружено).
4. **Вызов `recvmsg`/`read`** — копирование данных из ядерного буфера в пользовательский (~50 нс на копирование + системный вызов).

**Итог:** Если использовать стандартный Linux-стек с прерываниями, Ingress-задержка может быть **от 2 до 50 мкс** в зависимости от загрузки ядра.

### 3.2. Busy Polling vs IRQ-driven
Чтобы убить задержку, в HFT используют **Busy Polling**:
- Отключаем прерывания для нужного сетевого интерфейса (`ethtool -C eth0 adaptive-rx off rx-usecs 0`).
- Поток крутится в бесконечном цикле, постоянно вызывая `recvmsg` с флагом `MSG_DONTWAIT` (неблокирующий).
- Если пакетов нет, вставляем инструкцию `_mm_pause()` (или `rep nop`) — она говорит CPU "подожди немного, не сжигай энергию, но не переключай контекст".

**Цена Busy Polling:** 100% загрузка одного ядра CPU, но задержка снижается до 200-500 нс.

### 3.3. Код с recvmsg и инструкцией PAUSE

```cpp
class BusyPollRXEngine {
    static constexpr size_t BATCH_SIZE = 64;
    char rx_buffers[BATCH_SIZE][2048]; // Pre-allocated буферы
    struct iovec iov[BATCH_SIZE];
    struct mmsghdr msgs[BATCH_SIZE];
    
public:
    void run() {
        // Привязываем поток к ядру 2 (изолированное)
        pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &core_2);
        
        // Переводим сокет в неблокирующий режим
        int flags = fcntl(socket_fd, F_GETFL, 0);
        fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK);
        
        while (running) {
            // Замер №1: до попытки чтения
            uint64_t t_before = __rdtsc();
            
            // Пытаемся прочитать пачку пакетов (recvmmsg — читает несколько за раз)
            int received = recvmmsg(socket_fd, msgs, BATCH_SIZE, MSG_DONTWAIT, nullptr);
            
            if (received > 0) {
                // Замер №2: после чтения
                uint64_t t_after = __rdtsc();
                uint64_t ingress_latency = t_after - t_before;
                
                // Сохраняем в метрики (это и есть Ingress-to-Application)
                g_ingress_metrics.push(ingress_latency);
                
                // Обрабатываем все пакеты
                for (int i = 0; i < received; ++i) {
                    process_packet(msgs[i].msg_hdr.msg_iov[0].iov_base, 
                                   msgs[i].msg_len);
                }
            } else {
                // Нет пакетов — даем CPU отдохнуть (инструкция PAUSE)
                _mm_pause(); // ~10-20 нс "пустой" операции
            }
        }
    }
};
```

### 3.4. Как измерить время пробуждения без шума
Проблема: замер `t_before` делается **до** `recvmmsg`, но если системный вызов блокируется (даже с `DONTWAIT` он может войти в ядро на 1 такт), то часть времени в `ingress_latency` — это время самого вызова.

**Решение:** Замерять не только время от начала до конца, но и **количество циклов между успешными чтениями**. Если вы прочитали пакет на 1000-й итерации цикла, значит, вы ждали ~1000 * (время PAUSE) = ~20 мкс. Это и есть реальное время ожидания.

```cpp
uint64_t idle_cycles = 0;
while (running) {
    uint64_t tsc = __rdtsc();
    int n = recvmmsg(...);
    if (n > 0) {
        // Мы ждали idle_cycles тактов
        g_wait_latency.push(idle_cycles * TSC_TO_NS);
        idle_cycles = 0;
    } else {
        idle_cycles++;
        _mm_pause();
    }
}
```

---

## 4. Parsing/Decoding Latency (Битва с Branch Miss)

### 4.1. Почему условные переходы убивают производительность
Современные процессоры имеют конвейер глубиной 14-19 стадий. Чтобы поддерживать его заполненным, CPU предсказывает, по какой ветке `if` пойдет выполнение (Branch Prediction).

Если предсказание неверно (Branch Misprediction), конвейер сбрасывается, и CPU теряет **~14-20 тактов** (примерно 5-7 нс на 3 ГГц). В парсерах биржевых протоколов один `if` может стоить 5-7 нс, а их сотни — это уже микросекунды.

**HFT-подход:** Все частые типы сообщений идут по предсказуемому пути (fast-path). Редкие сообщения (ошибки, ретрансляции) обрабатываются медленно, и их задержка не важна.

### 4.2. __builtin_prefetch и __builtin_expect
- **`__builtin_prefetch(addr, 0, 1)`** — подгружает данные в кэш L1/L2 до того, как они понадобятся. В парсере вы кладете префетч на **следующий** пакет, пока парсите текущий.
- **`__builtin_expect(condition, 1)`** — подсказка компилятору, какая ветка наиболее вероятна. Влияет на расстановку инструкций `jmp` и уменьшает количество промахов предсказателя.

### 4.3. Zero-copy парсинг через reinterpret_cast
Никогда не делайте `memcpy` для разбора бинарных протоколов. Используйте прямое приведение указателей. Протоколы ITCH/SBE имеют фиксированное выравнивание, поэтому это безопасно.

```cpp
// Разбор ITCH Add Order (Type 'A')
struct ItchAddOrder {
    uint8_t type;      // 'A'
    uint16_t tracking; // 2 байта
    uint64_t ref_num;  // 8 байт
    uint32_t price;    // 4 байта (в сетевом порядке)
    uint32_t size;     // 4 байта
} __attribute__((packed));

inline void parse_add_order(const char* raw) {
    const ItchAddOrder* pkt = reinterpret_cast<const ItchAddOrder*>(raw);
    
    // Читаем напрямую из памяти (без копирования)
    uint64_t ref = ntohll(pkt->ref_num); // ntohll может быть медленной
    uint32_t price = ntohl(pkt->price);
    uint32_t size = ntohl(pkt->size);
    
    // ...
}
```

**Секрет:** `ntohl` на x86 — это инструкция `bswap` (1 такт). Не бойтесь её, но не вызывайте в цикле по одному байту.

### 4.4. Реальный код парсера ITCH/SBE

```cpp
class ItchParser {
    // Статистика для fast-path
    static constexpr size_t HIST_BINS = 64;
    uint64_t parse_hist[HIST_BINS] alignas(64);
    
public:
    inline void decode(const char* __restrict buffer, size_t len) {
        uint64_t t0 = __rdtsc();
        
        // Префетч следующего пакета (если он идет в памяти сразу за этим)
        __builtin_prefetch(buffer + len, 0, 1);
        
        const uint8_t type = buffer[0];
        
        // Fast-path: Add Order (80-90% всех сообщений)
        if (__builtin_expect(type == 'A', 1)) {
            const ItchAddOrder* pkt = reinterpret_cast<const ItchAddOrder*>(buffer);
            uint64_t ref = ntohll(pkt->ref_num);
            uint32_t price = ntohl(pkt->price);
            uint32_t size = ntohl(pkt->size);
            
            // Обновляем стакан...
            update_order_book(ref, price, size);
        } 
        // Medium-path: Modify/Delete (5-10%)
        else if (__builtin_expect(type == 'M', 0)) {
            // ... обработка модификации
        } 
        // Slow-path: Остальные (Trade, Cancel, Error) — 1-5%
        else {
            // Здесь время не критично, можно делать медленно
            handle_rare_message(buffer, len);
        }
        
        uint64_t t1 = __rdtsc();
        uint64_t latency = t1 - t0;
        
        // Записываем в гистограмму (бинарный логарифм)
        int bin = 63 - __builtin_clzll(latency);
        __atomic_add_fetch(&parse_hist[bin], 1, __ATOMIC_RELAXED);
    }
};
```

### 4.5. Гистограмма распределения времени парсинга
Почему нельзя использовать среднее? Потому что распределение **бимодальное**:
- Быстрые пакеты (попали в L1) — 20-30 нс.
- Медленные пакеты (промах L3) — 80-100 нс.

Среднее будет 60 нс, и вы не увидите, что 20% пакетов обрабатываются в 3 раза медленнее. Гистограмма с бинарными бакетами (0-15 нс, 16-31 нс, 32-63 нс, 64-127 нс и т.д.) показывает реальную картину.

---

## 5. Order Entry Processing Latency (Цена шаблонов и виртуальных функций)

### 5.1. Три под-метрики: сериализация, копирование, CRC
Время формирования исходящего ордера разбивается на три части:
1. **Сериализация заголовка и тела** — запись полей в буфер (зависит от кэша).
2. **Копирование данных** — если вы делаете `memcpy` больших блоков.
3. **Вычисление контрольной суммы (CRC/Checksum)** — самая дорогая часть.

Каждую часть нужно замерять отдельно, чтобы понять, где узкое место.

### 5.2. Аппаратный CRC32 через _mm_crc32_u64
Софтовый CRC32 через табличный метод — это ~3-5 циклов на байт. Для 100-байтного пакета = 300-500 тактов = ~100-150 нс. Это слишком много для HFT.

**Решение:** Инструкция `crc32` в SSE 4.2. Она считает CRC32 за **1 такт на 8 байт**.

```cpp
#include <immintrin.h> // для _mm_crc32_u64

inline uint32_t fast_crc32(const void* data, size_t len) {
    uint32_t crc = 0xFFFFFFFF;
    const uint64_t* words = reinterpret_cast<const uint64_t*>(data);
    const uint64_t* end = words + (len / 8);
    
    while (words < end) {
        crc = _mm_crc32_u64(crc, *words++);
    }
    
    // Остаток (если len не кратен 8)
    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(words);
    for (size_t i = 0; i < len % 8; ++i) {
        crc = _mm_crc32_u8(crc, bytes[i]);
    }
    
    return crc ^ 0xFFFFFFFF;
}
```

### 5.3. Pre-allocated буферы vs std::string
`std::string` вызывает `malloc` при создании (даже для небольших строк, если превышен SSO). `malloc` — это тяжелый системный вызов с блокировкой хипа (потенциально 100-500 нс). В HFT это абсолютно недопустимо.

**Решение:** Использовать массив на стеке или пул буферов, закрепленных в памяти (DMA-память).

```cpp
class OrderEncoder {
    // Буфер на стеке (выравнен на 64 байта для кэш-линии)
    char tx_buffer[256] __attribute__((aligned(64)));
    
public:
    inline size_t encode(const Order& ord) {
        // Запись напрямую в tx_buffer (без аллокаций!)
        *(uint32_t*)(tx_buffer) = htonl(ord.client_id);
        *(uint64_t*)(tx_buffer + 4) = htonll(ord.price);
        *(uint32_t*)(tx_buffer + 12) = htonl(ord.volume);
        
        // CRC (аппаратный)
        uint32_t crc = fast_crc32(tx_buffer, 16);
        *(uint32_t*)(tx_buffer + 16) = crc;
        
        return 20; // Размер пакета
    }
};
```

### 5.4. Код энкодера с LFENCE/SFENCE
Инструкции **LFENCE** (Load Fence) и **SFENCE** (Store Fence) нужны, чтобы запретить CPU переупорядочивать чтения/записи вокруг замера. Без них `rdtsc` может выполниться до того, как данные реально будут записаны в память, и замер покажет заниженное время.

```cpp
inline size_t encode_order(const Order& ord, char* buffer) {
    // LFENCE — дожидаемся завершения всех предыдущих чтений
    _mm_lfence();
    uint64_t t0 = __rdtsc();
    
    // ... запись в буфер ...
    
    // SFENCE — дожидаемся завершения всех записей
    _mm_sfence();
    uint64_t t1 = __rdtsc();
    
    uint64_t latency = t1 - t0;
    g_order_entry_metrics.push(latency);
    
    return packet_size;
}
```

### 5.5. Как избавиться от виртуальных функций в горячем пути
Виртуальная функция (`virtual`) — это косвенный вызов через vtable, который:
1. Не может быть инлайнен.
2. Создает зависимость по данным (load из таблицы, затем прыжок).
3. Добавляет 5-15 нс задержки.

**Решение:** Использовать **статический полиморфизм** через шаблоны (CRTP) или **переключатель через `if`/`switch`**.

```cpp
// ПЛОХО (виртуальная функция)
class IEncoder {
public:
    virtual size_t encode(const Order& ord, char* buffer) = 0;
};

// ХОРОШО (шаблон)
template<typename EncoderImpl>
class OrderSender {
    EncoderImpl encoder;
public:
    inline void send(const Order& ord) {
        char buffer[128];
        size_t len = encoder.encode(ord, buffer); // compile-time resolution
        nic_send(buffer, len);
    }
};

class SbeEncoder { /* ... */ };
class FixEncoder { /* ... */ };

OrderSender<SbeEncoder> sender; // Без vtable!
```

---

## 6. Inter-Core Latency (Проклятие MESI протокола)

### 6.1. Что такое когерентность кэшей и MESI
Процессоры Intel используют протокол MESI для согласования кэшей между ядрами:
- **Modified (M):** Данные изменены, только в этом кэше.
- **Exclusive (E):** Данные только в этом кэше, чистые.
- **Shared (S):** Данные есть в нескольких кэшах (только чтение).
- **Invalid (I):** Данные недействительны.

Когда одно ядро пишет в переменную, которая находится в кэше другого ядра, происходит **Cache Line Bouncing**:
1. Ядро A отправляет запрос на шине (Ring Bus).
2. Ядро B инвалидирует свою кэш-линию (переводит в I).
3. Ядро A забирает кэш-линию в состояние M.
4. Всё это занимает **80-400 нс** в зависимости от расстояния между ядрами и загрузки шины.

### 6.2. Почему cache line bouncing добавляет 300+ нс
Если вы используете `std::atomic` без явного выравнивания, несколько переменных могут лежать на одной кэш-линии (64 байта). Когда два ядра пишут в разные переменные на одной линии, они постоянно борются за неё — это называется **False Sharing**.

**Пример:**
```cpp
struct BadMetrics {
    uint64_t counter_a; // Ядро 0 пишет сюда
    uint64_t counter_b; // Ядро 1 пишет сюда
}; // Обе переменные лежат на одной кэш-линии → False Sharing!
```

**Решение:** Выравнивание на 64 байта (размер кэш-линии).

```cpp
struct GoodMetrics {
    alignas(64) uint64_t counter_a; // Ядро 0
    alignas(64) uint64_t counter_b; // Ядро 1
};
```

### 6.3. Micro-benchmark для измерения межъядерной задержки
Пишем тест, где два потока обмениваются указателем через `std::atomic`, замеряя время.

```cpp
std::atomic<void*> g_data{nullptr};
std::atomic<bool> g_ready{false};

// Поток A (пишет)
void thread_writer() {
    pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpu_0);
    
    void* dummy = (void*)0xDEADBEEF;
    
    // Ждем готовности читателя
    while (!g_ready.load(std::memory_order_acquire)) {
        _mm_pause();
    }
    
    // Замеряем время записи
    for (int i = 0; i < 1000000; ++i) {
        g_data.store(dummy, std::memory_order_release);
    }
}

// Поток B (читает)
void thread_reader() {
    pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpu_1);
    
    // Готовность
    g_ready.store(true, std::memory_order_release);
    
    // Замеряем время чтения
    uint64_t sum_latency = 0;
    for (int i = 0; i < 1000000; ++i) {
        uint64_t t0 = __rdtsc();
        void* val = g_data.load(std::memory_order_acquire);
        uint64_t t1 = __rdtsc();
        sum_latency += (t1 - t0);
    }
    // sum_latency / 1000000 = средняя Inter-Core Latency
}
```

### 6.4. Код SPSC очереди с memory_order_acquire
В реальной SPSC очереди критична операция `pop()` — она читает `head`, записанный другим ядром.

```cpp
template<typename T, size_t N>
class SPSCQueue {
    T buffer[N];
    std::atomic<size_t> head{0}, tail{0};
    
public:
    inline T* pop() {
        // Замер №1: до чтения tail (записано продюсером на другом ядре)
        uint64_t t0 = __rdtsc();
        
        size_t t = tail.load(std::memory_order_acquire); // <-- Барьер!
        size_t h = head.load(std::memory_order_relaxed);
        
        if (h == t) return nullptr;
        
        T* ptr = &buffer[h % N];
        head.store(h + 1, std::memory_order_release);
        
        // Замер №2: после завершения чтения
        uint64_t t1 = __rdtsc();
        g_inter_core_latency.push(t1 - t0); // Это и есть Inter-Core Latency
        
        return ptr;
    }
};
```

**memory_order_acquire** заставляет CPU дождаться завершения всех предыдущих чтений и синхронизировать кэш. Это и есть источник задержки — время ожидания ответа от другого ядра по кольцевой шине.

### 6.5. Как привязать потоки к ядрам (pthread_setaffinity_np)

```cpp
#include <pthread.h>
#include <sched.h>

void pin_thread_to_core(int core_id) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);
    
    pthread_t thread = pthread_self();
    if (pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset) != 0) {
        // Ошибка: core_id не существует
    }
}

// Использование
void strategy_thread() {
    pin_thread_to_core(2); // Стратегия на ядре 2
    // ...
}

void market_data_thread() {
    pin_thread_to_core(4); // Парсер на ядре 4
    // ...
}
```

**Важно:** В HFT используют **изолированные ядра** (kernel parameter `isolcpus=2,3,4,5`), чтобы шедулер Linux не клал на них другие процессы.

---

## 7. Где конкретно в C++ коде ставятся замеры (Карта модулей)

### 7.1. Таблица: модуль → точка замера → метрика

| Модуль | Где ставится замер | Какая метрика |
|--------|-------------------|---------------|
| **Network RX (NIC driver)** | В колбэке `efx_rx_event` (аппаратно) | RX Timestamp |
| **Packet Dispatcher** | На входе `pop()` из SPSC-очереди | Ingress-to-Application |
| **Market Data Parser** | Вход/выход из `decode()` | Parsing Latency |
| **Strategy Engine** | Вход/выход из `onBookUpdate()` | Tick-to-Decision |
| **Risk Manager** | Вход/выход из `validateOrder()` | Risk Check Latency |
| **Order Manager** | Вход/выход из `submitOrder()` | Order Entry Latency |
| **Order Encoder (CRC)** | Вокруг `fast_crc32()` | CRC Latency |
| **Network TX** | В колбэке `efx_tx_event` (аппаратно) | TX Timestamp |
| **Агрегатор** | После вычисления `TX - RX` | Tick-to-Trade |

### 7.2. Что должно быть в горячем пути, а что нет

**В горячем пути (выполняется на каждые пакет):**
- Только `__rdtsc()` для замера (без системных вызовов).
- Запись в TLS-массив (без мьютексов, без атомарных операций с `seq_cst`).
- Минимум условий — только `if (__builtin_expect(...))`.

**Вне горячего пути (фоновые потоки):**
- Агрегация гистограмм.
- Отправка метрик по UDP.
- Калибровка TSC по PTP.
- Запись логов.

---

## 8. Как НЕ НУЖНО замерять Latency в C++ (Смертельные грехи)

### 8.1. Чек-лист запрещенных практик

| ❌ Запрещено | ✅ Как правильно |
|-------------|------------------|
| `std::chrono::high_resolution_clock::now()` в горячем цикле | `__rdtsc()` или аппаратный timestamp из NIC |
| `std::cout << latency` | Запись в lock-free SPSC-буфер или TLS |
| `std::mutex` для защиты счетчиков | `std::atomic` с `memory_order_relaxed` |
| `malloc()`/`new` в парсере | Pre-allocated пулы или стековые массивы |
| `memcpy()` для разбора пакетов | `reinterpret_cast` на сырые данные |
| `std::function` и лямбды в горячем пути | Инлайн-функции или шаблоны |
| `virtual` функции в обработчиках | Статический полиморфизм (CRTP) |
| `std::map`/`std::unordered_map` для поиска | Открытые хеш-таблицы с битовыми операциями |
| `volatile` для счетчиков | `std::atomic` с явным memory order |

### 8.2. Почему volatile бесполезен
`volatile` говорит компилятору: "не оптимизируй это, читай из памяти каждый раз". Но он **не дает атомарности** и **не запрещает переупорядочивание** инструкций CPU.

```cpp
volatile uint64_t counter = 0;
// Поток 1
counter++; // Это не атомарно! Два потока могут одновременно читать/писать.

// Поток 2
if (counter == 100) { ... } // Может быть прочитано устаревшее значение из кэша.
```

### 8.3. Ошибки с memory ordering
- ❌ `std::atomic::store(val, std::memory_order_seq_cst)` — полный барьер, синхронизирует все ядра (~100 нс).
- ✅ `std::atomic::store(val, std::memory_order_release)` — только запрещает переупорядочивание записи (быстрее).
- ✅ `std::atomic::load(std::memory_order_acquire)` — только запрещает переупорядочивание чтения.

**Правило:** В SPSC-очереди используйте `release` на запись, `acquire` на чтение. Это минимально необходимые барьеры.

### 8.4. Debug vs Release билды
На `-O0` компилятор не инлайнит функции, добавляет проверки стека и не применяет оптимизации. Замеры на Debug-билде покажут задержку в 2-5 раз больше, чем на Release.

**Настройки компилятора для HFT:**
```bash
g++ -O3 -march=native -mtune=native -fno-exceptions -fno-rtti -flto -fno-stack-protector -fomit-frame-pointer -funroll-loops -finline-functions
```

---

## 9. Эталонная архитектура замера в C++ (Production-код)

### 9.1. Thread-Local Storage (TLS) для счетчиков
Каждый поток пишет в свой экземпляр метрик, чтобы избежать конкуренции за кэш-линии (False Sharing).

```cpp
// Структура метрик для одного потока (выравнена на 64 байта)
struct alignas(64) ThreadMetrics {
    // Счетчики для разных метрик
    uint64_t parse_cnt = 0;
    uint64_t parse_sum = 0;
    uint64_t parse_max = 0;
    uint64_t parse_hist[64] = {}; // Гистограмма
    
    uint64_t ingress_cnt = 0;
    uint64_t ingress_sum = 0;
    // ...
};

// TLS-переменная (каждый поток имеет свою)
static thread_local ThreadMetrics tl_metrics;
```

### 9.2. Выравнивание на 64 байта (cache line alignment)
`alignas(64)` гарантирует, что структура не будет разделена между кэш-линиями.

### 9.3. Гистограмма через __builtin_clz (бинарный логарифм)
Чтобы не вызывать `std::log2` (медленно), используем встроенную функцию подсчета ведущих нулей.

```cpp
inline void add_to_histogram(uint64_t value) {
    // Находим бинарный логарифм (индекс бакета)
    int bin = 63 - __builtin_clzll(value); // CLZ = Count Leading Zeros
    
    // Обновляем гистограмму в TLS (без атомарных операций!)
    tl_metrics.parse_hist[bin]++;
}
```

**Пример:** Если `value = 64` (0b1000000), CLZ = 57, bin = 63 - 57 = 6. Значит, значение попадает в бакет 2^6 = 64..127 нс.

### 9.4. Фоновый агрегатор с отправкой по UDP
Фоновый поток раз в 1 мс собирает все TLS-метрики и отправляет их по UDP в мониторинг (Grafana/Prometheus через StatsD).

```cpp
void metrics_aggregator() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        
        // Глобальный сборщик метрик (имеет доступ ко всем TLS)
        for (auto& metrics : g_all_thread_metrics) {
            // Схлопываем гистограммы (суммируем бакеты)
            for (int i = 0; i < 64; ++i) {
                g_global_hist[i] += metrics.parse_hist[i];
            }
            // Обнуляем локальные счетчики
            metrics.parse_cnt = 0;
            metrics.parse_sum = 0;
            // ...
        }
        
        // Отправка по UDP (неблокирующая, без TCP-соединений)
        send_udp_metrics(g_global_hist);
    }
}
```

### 9.5. Полный пример класса MetricCollector

```cpp
class MetricCollector {
public:
    // Вызов из горячего пути (инлайн)
    static inline void emit_parse_latency(uint64_t latency) {
        tl_metrics.parse_cnt++;
        tl_metrics.parse_sum += latency;
        if (latency > tl_metrics.parse_max) {
            tl_metrics.parse_max = latency;
        }
        int bin = 63 - __builtin_clzll(latency);
        tl_metrics.parse_hist[bin]++;
    }
    
    // Вызов из фонового агрегатора
    static void aggregate_and_send() {
        // Очищаем глобальные счетчики
        memset(g_global_hist, 0, sizeof(g_global_hist));
        uint64_t total_sum = 0, total_cnt = 0;
        
        // Проходим по всем потокам (регистрируем при создании)
        for (auto& metrics : g_registered_threads) {
            total_cnt += metrics.parse_cnt;
            total_sum += metrics.parse_sum;
            for (int i = 0; i < 64; ++i) {
                g_global_hist[i] += metrics.parse_hist[i];
            }
        }
        
        // Формируем сообщение для StatsD (UDP)
        std::string msg = "hft.parse.avg:" + std::to_string(total_sum / total_cnt) + "|g\n";
        msg += "hft.parse.p99:" + compute_percentile(g_global_hist, 0.99) + "|g\n";
        
        // Отправка по UDP (неблокирующая)
        udp_socket.send(msg.c_str(), msg.size());
    }
};

// Использование в парсере
void parse_packet(const char* data) {
    uint64_t t0 = __rdtsc();
    // ... парсинг ...
    uint64_t t1 = __rdtsc();
    MetricCollector::emit_parse_latency(t1 - t0);
}
```