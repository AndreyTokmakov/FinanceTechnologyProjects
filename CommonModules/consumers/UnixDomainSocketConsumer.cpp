/**============================================================================
Name        : UnixDomainSocketConsumer.cpp
Created on  : 29.01.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : UnixDomainSocketConsumer.cpp
============================================================================**/

#include "UnixDomainSocketConsumer.h"

#include <unistd.h>
#include <cerrno>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>

#include <iostream>
#include <string_view>
#include <thread>
#include <chrono>
#include <utility>

#define RESULT_SUCCESS   (0)
#define INVALID_HANDLE   (-1)
#define SOCKET_ERROR     (-1)
#define SERVER_SOCK_PATH "/tmp/unix_socket"

namespace
{
    using namespace std::string_view_literals;
    using Socket = int32_t;

    std::string_view err2String(const int32_t errorCode)
    {
        switch (errorCode)
        {
            case EPERM: return "EPERM: Operation not permitted"sv;
            case ENOENT: return "ENOENT: No such file or directory"sv;
            case ESRCH: return "ESRCH: No such process"sv;
            case EINTR: return "EINTR: Interrupted system call"sv;
            case EIO: return "EIO: I/O error"sv;
            case ENXIO: return "ENXIO: No such device or address"sv;
            case E2BIG: return "E2BIG: Argument list too long"sv;
            case ENOEXEC: return "ENOEXEC: Exec format error"sv;
            case EBADF: return "EBADF: Bad file number"sv;
            case ECHILD: return "ECHILD: No child processes"sv;
            case EAGAIN: return "EAGAIN: Try again"sv;
            case ENOMEM: return "ENOMEM: Out of memory"sv;
            case EACCES: return "EACCES:  Permission denied"sv;
            case EFAULT: return "EFAULT: Bad address"sv;
            case ENOTBLK: return "ENOTBLK: Block device required"sv;
            case EBUSY: return "EBUSY: Device or resource busy"sv;
            case EEXIST: return "EEXIST: File exists"sv;
            case EXDEV: return "EXDEV: Cross-device link"sv;
            case ENODEV: return "ENODEV: No such device"sv;
            case ENOTDIR: return "ENOTDIR: Not a directory"sv;
            case EISDIR: return "EISDIR: Is a directory "sv;
            case EINVAL: return "EINVAL: Invalid argument"sv;
            case ENFILE: return "ENFILE: File table overflow"sv;
            case EMFILE: return "EMFILE: Too many open files"sv;
            case ENOTTY: return "ENOTTY: Not a typewriter"sv;
            case ETXTBSY: return "ETXTBSY: Text file busy "sv;
            case EFBIG: return "EFBIG: File too large"sv;
            case ENOSPC: return "ENOSPC: No space left on device"sv;
            case ESPIPE: return "ESPIPE: Illegal seek"sv;
            case EROFS: return "EROFS: Read-only file system"sv;
            case EMLINK: return "EMLINK: Too many links"sv;
            case EPIPE: return "EPIPE: Broken pipe"sv;
            case EDOM: return "EDOM: Math argument out of domain of func"sv;
            case ERANGE: return "ERANGE: Math result not representable"sv;
            default: return "Unknown"sv;
        }
    }

    template<typename ResultType = bool , ResultType value = false>
    ResultType error(std::string_view message)
    {
        std::cerr << message << ". Error (" << errno << "): " << err2String(errno) << std::endl;
        return value;
    }

    template<typename ResultType = bool , ResultType value = false>
    ResultType error(std::string_view message, const int32_t errorCode)
    {
        std::cerr << message << errorCode << std::endl;
        return value;
    }
}

namespace UnixDomainSocketConsumer
{
    struct UDSAsynchServer
    {
        constexpr static size_t BUFFER_SIZE { 10 * 1024 };
        constexpr static size_t MAX_DESCRIPTORS { 256 };
        constexpr static int32_t TIMEOUT { 3 * 60 * 1000 };

        Socket serverSocket { INVALID_HANDLE };
        std::string filePath { SERVER_SOCK_PATH };

        std::array<char, BUFFER_SIZE> buffer {};
        std::array<pollfd, MAX_DESCRIPTORS> fds {};
        int32_t handlesCount { 0 };

        explicit UDSAsynchServer(std::string udmSockPath);
        ~UDSAsynchServer();

        void removeClosedHandles();

        // TODO: std::expected<R,E>
        [[nodiscard]]
        bool init() const;

        bool start();

        static bool setSocketToNonBlock(Socket socket) ;
    };
}



namespace UnixDomainSocketConsumer
{
    UDSAsynchServer::UDSAsynchServer(std::string udmSockPath): filePath { std::move( udmSockPath ) }
    {
        serverSocket = ::socket(AF_UNIX, SOCK_STREAM, 0);
        if (INVALID_HANDLE == serverSocket) {
            throw std::runtime_error("CLIENT: Create socket failed.");
        }

        if (const int32_t result = ::unlink(filePath.data()); INVALID_HANDLE == result)
        {
            if (const int error = errno; ENOENT != error && RESULT_SUCCESS != error) {
                throw std::runtime_error("Failed to unlink " + filePath + " file");
            }
        }

        fds[0].fd = serverSocket;
        fds[0].events = POLLIN;

        handlesCount = 1;
    }

    UDSAsynchServer::~UDSAsynchServer()
    {
        if (const int32_t result = ::remove(filePath.data()); INVALID_HANDLE == result) {
            error("remove() failed. Result = ", result);
        }
        if (const int32_t result = ::close(serverSocket); INVALID_HANDLE == result) {
            error("close() failed. Result = ", result);
        }
    }

    bool UDSAsynchServer::setSocketToNonBlock(const Socket socket)
    {
        const int flags = ::fcntl(socket, F_GETFL, 0);
        if (SOCKET_ERROR == ::fcntl(socket, F_SETFL, flags | O_NONBLOCK)) {
            return error("fcntl() failed for client = " + std::to_string(socket));
        }
        return false;
    }

    // TODO: std::expected<R,E>
    [[nodiscard]]
    bool UDSAsynchServer::init() const
    {
        if (setSocketToNonBlock(serverSocket)) {
            return error("setSocketToNonBlock() failed");
        }
        int32_t yes { 1 };
        if (SOCKET_ERROR == ::setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))) {
            return error("setsockopt() failed");
        }

        const sockaddr_un serverAddr { .sun_family = AF_UNIX, .sun_path = SERVER_SOCK_PATH };
        uint32_t len = sizeof(serverAddr);
        if (RESULT_SUCCESS != ::bind(serverSocket, reinterpret_cast<const sockaddr*>(&serverAddr), len)) {
            return error("bind() failed");
        }

        if (RESULT_SUCCESS != ::listen(serverSocket, 32)) {
            return error("listen() failed");
        }

        return true;
    }

    void UDSAsynchServer::removeClosedHandles()
    {
        uint32_t fdCount = 1;
        for (uint32_t idx = fdCount, size = fds.size(); idx < size; ++idx ) {
            if (fds[idx].fd != -1)
                std::swap(fds[idx], fds[fdCount++]);
        }
    }

    bool UDSAsynchServer::start()
    {
        std::string message;
        int32_t currentSize { 0 };
        while (true)
        {
            if (const int32_t result = ::poll(fds.data(), handlesCount, TIMEOUT); SOCKET_ERROR == result) {
                return error("poll() failed ");
            } else if (0 == result) {
                return error("poll() timeout ");
            }

            currentSize = handlesCount;
            for (int32_t idx = 0; idx < currentSize; idx++)
            {
                if (0 == fds[idx].revents)
                    continue;
                if (fds[idx].revents != POLLIN)
                {
                    const Socket hSocket = fds[idx].fd;

                    // Close socket if we got event with POLLHUP flag set
                    if (fds[idx].revents & POLLHUP)
                    {
                        // FIXME
                        ::close(hSocket);
                        fds[idx].fd = -1;

                        removeClosedHandles();
                        break;
                    }
                }
                else if (fds[idx].fd == serverSocket) /** Listening descriptor is readable. **/
                {
                    Socket clientSocket { INVALID_HANDLE };
                    while (true)
                    {
                        clientSocket = ::accept(serverSocket, nullptr, nullptr);
                        if (SOCKET_ERROR == clientSocket)
                        {
                            if (errno != EWOULDBLOCK /*|| EAGAIN != errno*/) {
                                return error("accept() failed = " + std::to_string(fds[idx].revents));
                            } else {
                                // std::cout << "Accept ==> EWOULDBLOCK" << std::endl;
                                break;
                            }
                        }
                        else
                        {
                            std::cout << "New incoming connection. Client socket = " << clientSocket << std::endl;
                            setSocketToNonBlock(clientSocket);

                            fds[handlesCount].fd = clientSocket;
                            fds[handlesCount].events = POLLIN | POLLHUP;

                            ++handlesCount;
                            break;
                        }
                    }
                }
                else
                {
                    const Socket clientSocket = fds[idx].fd;
                    while (true)
                    {
                        const int64_t bytesRead = ::recv(clientSocket, buffer.data(), BUFFER_SIZE, 0);
                        if (SOCKET_ERROR == bytesRead)
                        {
                            if (errno != EWOULDBLOCK /*|| EAGAIN != errno*/) {
                                return error("recv() failed = " + std::to_string(fds[idx].revents));
                            } else {
                                // std::cout << "recv ==> EWOULDBLOCK" << std::endl;
                                break;
                            }
                        }
                        else if (0 == bytesRead)
                        {
                            std::cout << "Close connection for client = " << clientSocket << std::endl;
                            // FIXME
                            ::close(clientSocket);
                            fds[idx].fd = -1;

                            removeClosedHandles();
                            break;
                        }
                        else
                        {
                            message.assign(buffer.data(), bytesRead);
                            // std::cout << clientSocket << " | " <<  message << std::endl;
                        }
                    }
                }
            }
        }
    }


    void runServer()
    {
        UDSAsynchServer server (SERVER_SOCK_PATH);
        if (!server.init()) {
            std::cerr << "Failed to initialize server" << std::endl;
        }
        server.start();
    }
}

void UnixDomainSocketConsumer::TestAll()
{
    runServer();
}