
/*
Код даётся в одном файле md_feeder.cpp. Он поддерживает два режима:

	feeder — симулирует обменную ленту и шлёт UDP-multicast пакеты (массив сообщений) на указанный адрес/порт;
	receiver — слушает multicast-адрес и получает пакеты с recvmmsg (batch recieve), печатает/агрегирует.
	Компилировать: g++ -O3 -march=native -std=gnu++17 md_feeder.cpp -o md_feeder

Запуск-пример:

	В одном терминале (приёмник): ./md_feeder receiver 239.0.0.1 30001
	В другом (фидер): sudo ./md_feeder feeder 239.0.0.1 30001 100000 (посылает 100k сообщений/сек печатать не будет)

В примере демонстрируется

	статическая (POD) структура сообщения с выравниванием — минимальный парсинг на стороне приёмника;
	batch отправка (sendmmsg) и batch приём (recvmmsg) для снижения системных вызовов;
	пред-аллокированные iovec/mmsg_hdr структуры для повторного использования (zero alloc in hot path);
	busy-spin loop с nanosleep/CPU relaxing вариант (обычно в HFT используется спин с PAUSE);
	базовые socket tuning (SO_SNDBUF, IP_MULTICAST_TTL).

Для реального HFT настроить
	- rmem/sndbuf
	- NIC offloads
	- IRQ/CPU affinity
	- hugepages
*/


// md_feeder.cpp
// Simple HFT-style market data feeder (feeder + receiver) using UDP multicast,
// batch send/recv via sendmmsg/recvmmsg, preallocated buffers and busy-spin loop.
//
// Compile:
//   g++ -O3 -march=native -std=gnu++17 md_feeder.cpp -o md_feeder
//
// Usage:
//   ./md_feeder receiver 239.0.0.1 30001
//   sudo ./md_feeder feeder   239.0.0.1 30001 100000   # send target, rate messages/sec

#define _GNU_SOURCE
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <unistd.h>
#include <linux/if.h>
#include <linux/ip.h>
#include <cstring>
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <atomic>
#include <cassert>

// Message format: keep it POD and fixed-size for low-latency copying.
// Packed so no padding surprises.
#pragma pack(push,1)
struct MDMessage {
    uint64_t seq;          // sequence number
    uint64_t ts_ns;        // producer timestamp in ns (CLOCK_MONOTONIC)
    uint32_t symbol_id;    // symbol id (small integer)
    double   price;        // price
    uint32_t size;         // quantity
};
#pragma pack(pop)

static_assert(sizeof(MDMessage) == 8+8+4+8+4, "Unexpected MDMessage size");

// Helpers
inline uint64_t now_ns() {
    using namespace std::chrono;
    return duration_cast<nanoseconds>(steady_clock::now().time_since_epoch()).count();
}

void set_socket_coalesce_opts(int sock) {
    // Try to bump sndbuf for high throughput (may require root or sysctl)
    int sndbuf = 4 * 1024 * 1024; // 4MB
    if (setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &sndbuf, sizeof(sndbuf)) != 0) {
        std::cerr << "Warning: SO_SNDBUF set failed: " << strerror(errno) << "\n";
    }
}

int create_multicast_sender(const char* mcast_addr, uint16_t port) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) { perror("socket"); return -1; }

    set_socket_coalesce_opts(sock);

    // Set multicast TTL (how many routers)
    int ttl = 1;
    setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl));

    // Optional: disable loopback (don't receive your own multicast)
    // int loop = 0; setsockopt(sock, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop));

    // For sendmmsg, no bind required usually, kernel will choose source address.
    return sock;
}

int create_multicast_receiver(const char* mcast_addr, uint16_t port, const char* if_addr = nullptr) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) { perror("socket"); return -1; }

    // Reuse address/port
    int reuse = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        perror("setsockopt SO_REUSEADDR");
    }

    sockaddr_in local{};
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = htonl(INADDR_ANY);
    local.sin_port = htons(port);

    if (bind(sock, reinterpret_cast<sockaddr*>(&local), sizeof(local)) < 0) {
        perror("bind");
        close(sock);
        return -1;
    }

    // Join multicast group
    ip_mreq mreq{};
    inet_pton(AF_INET, mcast_addr, &mreq.imr_multiaddr);
    if (if_addr) inet_pton(AF_INET, if_addr, &mreq.imr_interface);
    else mreq.imr_interface.s_addr = htonl(INADDR_ANY);

    if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
        perror("IP_ADD_MEMBERSHIP");
        close(sock);
        return -1;
    }

    // Increase recv buffer
    int rcvbuf = 8 * 1024 * 1024;
    setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &rcvbuf, sizeof(rcvbuf));

    return sock;
}

// --------------------------- Feeder (sender) ---------------------------
//
// We preallocate an array of mmsg_hdr + iovec + buffers and reuse them. Use sendmmsg
// to batch several UDP datagrams per syscall.

// Default batch sizes for sendmmsg/recvmmsg
constexpr int BATCH_SIZE = 32;
constexpr size_t MAX_MSGS = 1024;

void run_feeder(const char* mcast_addr, uint16_t port, uint64_t target_rate) {
    int sock = create_multicast_sender(mcast_addr, port);
    if (sock < 0) return;

    sockaddr_in dst{};
    dst.sin_family = AF_INET;
    dst.sin_port = htons(port);
    inet_pton(AF_INET, mcast_addr, &dst.sin_addr);

    // Pre-allocate messages and mmsghdr structures
    std::vector<struct mmsghdr> mmsgs(BATCH_SIZE);
    std::vector<struct iovec> iov(BATCH_SIZE);
    std::vector<MDMessage> messages(BATCH_SIZE);

    for (int i = 0; i < BATCH_SIZE; ++i) {
        memset(&mmsgs[i], 0, sizeof(mmsgs[i]));
        iov[i].iov_base = &messages[i];
        iov[i].iov_len = sizeof(MDMessage);

        mmsgs[i].msg_hdr.msg_iov = &iov[i];
        mmsgs[i].msg_hdr.msg_iovlen = 1;
        mmsgs[i].msg_hdr.msg_name = &dst;
        mmsgs[i].msg_hdr.msg_namelen = sizeof(dst);
    }

    uint64_t seq = 1;
    const uint64_t interval_ns = target_rate ? (1000000000ULL * BATCH_SIZE / target_rate) : 0;
    std::cout << "Feeder start. batch=" << BATCH_SIZE << " target_rate=" << target_rate << " msg/s\n";

    // Tight sending loop
    while (true) {
        // fill batch
        for (int i = 0; i < BATCH_SIZE; ++i) {
            MDMessage &m = messages[i];
            m.seq = seq++;
            m.ts_ns = now_ns();
            m.symbol_id = (i % 8) + 1; // few symbols
            m.price = 100.0 + (m.seq % 1000) * 0.01;
            m.size = 100 + (m.seq % 100);
            // iovec already points to m
        }

        // send batch
        int sent = sendmmsg(sock, mmsgs.data(), BATCH_SIZE, 0);
        if (sent < 0) {
            if (errno == EAGAIN || errno == EINTR) {
                // transient
                continue;
            }
            perror("sendmmsg");
            break;
        }
        // Optionally: measure latency, log, update metrics...
        // Rate control
        if (target_rate) {
            // simple sleep to throttle. In real HFT use busy-spin + precise timers
            std::this_thread::sleep_for(std::chrono::nanoseconds(interval_ns));
        }
    }

    close(sock);
}

// --------------------------- Receiver (listener) ---------------------------
//
// Uses recvmmsg to fetch batch of messages per syscall and process them.
// Minimal processing: count messages and optionally print sample.

void run_receiver(const char* mcast_addr, uint16_t port) {
    int sock = create_multicast_receiver(mcast_addr, port);
    if (sock < 0) return;

    // Pre-allocate recvmmsg arrays
    std::vector<struct mmsghdr> mmsgs(BATCH_SIZE);
    std::vector<struct iovec> iov(BATCH_SIZE);
    std::vector<MDMessage> buffers(BATCH_SIZE);

    for (int i = 0; i < BATCH_SIZE; ++i) {
        memset(&mmsgs[i], 0, sizeof(mmsgs[i]));
        iov[i].iov_base = &buffers[i];
        iov[i].iov_len = sizeof(MDMessage);
        mmsgs[i].msg_hdr.msg_iov = &iov[i];
        mmsgs[i].msg_hdr.msg_iovlen = 1;
    }

    uint64_t total = 0;
    auto last_print = std::chrono::steady_clock::now();

    std::cout << "Receiver listening on " << mcast_addr << ":" << port << "\n";

    while (true) {
        int received = recvmmsg(sock, mmsgs.data(), BATCH_SIZE, 0, nullptr);
        if (received < 0) {
            if (errno == EINTR) continue;
            perror("recvmmsg");
            break;
        }
        total += received;

        // process batch
        for (int i = 0; i < received; ++i) {
            MDMessage &m = buffers[i];
            // minimal validation/processing
            // e.g. compute latency:
            uint64_t now = now_ns();
            uint64_t latency_ns = now - m.ts_ns;
            // For demo, print the first message of every second
            (void)latency_ns;
        }

        auto now = std::chrono::steady_clock::now();
        if (now - last_print >= std::chrono::seconds(1u)) {
            std::cout << "Received total messages: " << total << " last_batch=" << received << "\n";
            last_print = now;
        }
    }

    close(sock);
}

// --------------------------- main ---------------------------

int main(int argc, char** argv)
{
    if (argc < 4) {
        std::cerr << "Usage:\n  " << argv[0] << " feeder <mcast_addr> <port> [rate_msgs_per_sec]\n";
        std::cerr << "  " << argv[0] << " receiver <mcast_addr> <port>\n";
        return 1;
    }

    std::string mode = argv[1];
    const char* addr = argv[2];
    uint16_t port = static_cast<uint16_t>(std::stoi(argv[3]));

    if (mode == "feeder") {
        uint64_t rate = 0;
        if (argc >= 5) rate = std::stoull(argv[4]);
        run_feeder(addr, port, rate);
    } else if (mode == "receiver") {
        run_receiver(addr, port);
    } else {
        std::cerr << "Unknown mode: " << mode << "\n";
        return 1;
    }
    return 0;
}
