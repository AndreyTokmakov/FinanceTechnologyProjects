/**============================================================================
Name        : EventConsumerUDS.cpp
Created on  : 29.01.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : EventConsumerUDS.cpp
============================================================================**/

#include "EventConsumerUDS.h"

#include <iostream>

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

#define SERVER_SOCK_PATH "/tmp/unix_socket"


namespace
{
    using namespace std::string_literals;
    using namespace std::string_view_literals;

    std::string err2String(const int32_t errorCode)
    {
        switch (errorCode)
        {
            case EPERM: return "EPERM: Operation not permitted"s;
            case ENOENT: return "ENOENT: No such file or directory"s;
            case ESRCH: return "ESRCH: No such process"s;
            case EINTR: return "EINTR: Interrupted system call"s;
            case EIO: return "EIO: I/O error"s;
            case ENXIO: return "ENXIO: No such device or address"s;
            case E2BIG: return "E2BIG: Argument list too long"s;
            case ENOEXEC: return "ENOEXEC: Exec format error"s;
            case EBADF: return "EBADF: Bad file number"s;
            case ECHILD: return "ECHILD: No child processes"s;
            case EAGAIN: return "EAGAIN: Try again"s;
            case ENOMEM: return "ENOMEM: Out of memory"s;
            case EACCES: return "EACCES:  Permission denied"s;
            case EFAULT: return "EFAULT: Bad address"s;
            case ENOTBLK: return "ENOTBLK: Block device required"s;
            case EBUSY: return "EBUSY: Device or resource busy"s;
            case EEXIST: return "EEXIST: File exists"s;
            case EXDEV: return "EXDEV: Cross-device link"s;
            case ENODEV: return "ENODEV: No such device"s;
            case ENOTDIR: return "ENOTDIR: Not a directory"s;
            case EISDIR: return "EISDIR: Is a directory "s;
            case EINVAL: return "EINVAL: Invalid argument"s;
            case ENFILE: return "ENFILE: File table overflow"s;
            case EMFILE: return "EMFILE: Too many open files"s;
            case ENOTTY: return "ENOTTY: Not a typewriter"s;
            case ETXTBSY: return "ETXTBSY: Text file busy "s;
            case EFBIG: return "EFBIG: File too large"s;
            case ENOSPC: return "ENOSPC: No space left on device"s;
            case ESPIPE: return "ESPIPE: Illegal seek"s;
            case EROFS: return "EROFS: Read-only file system"s;
            case EMLINK: return "EMLINK: Too many links"s;
            case EPIPE: return "EPIPE: Broken pipe"s;
            case EDOM: return "EDOM: Math argument out of domain of func"s;
            case ERANGE: return "ERANGE: Math result not representable"s;
            default: return "Unknown"s;
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

namespace EventConsumerUDS
{
    UDSAsynchServer::UDSAsynchServer(Common::Queue<std::string>& queue,
                                     std::string udmSockPath):
            filePath { std::move( udmSockPath ) },
            eventQueue { queue }
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

    [[nodiscard]]
    std::expected<bool, std::string> UDSAsynchServer::setSocketToNonBlock(const Socket socket)
    {
        const int flags = ::fcntl(socket, F_GETFL, 0);
        if (SOCKET_ERROR == ::fcntl(socket, F_SETFL, flags | O_NONBLOCK)) {
            return std::unexpected{"fcntl(F_SETFL, O_NONBLOCK) failed. Error: " + err2String(errno)};
        }
        return true;
    }

    [[nodiscard]]
    std::expected<bool, std::string> UDSAsynchServer::init() const
    {
        if (const auto result = setSocketToNonBlock(serverSocket); !result.has_value()) {
            return std::unexpected{"setSocketToNonBlock() failed. Error: " + result.error()};
        }
        int32_t yes { 1 };
        if (SOCKET_ERROR == ::setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))) {
            return std::unexpected{"setsockopt(SOL_SOCKET, SO_REUSEADDR) failed. Error: " + err2String(errno)};
        }

        const sockaddr_un serverAddr { .sun_family = AF_UNIX, .sun_path = SERVER_SOCK_PATH };
        uint32_t len = sizeof(serverAddr);
        if (RESULT_SUCCESS != ::bind(serverSocket, reinterpret_cast<const sockaddr*>(&serverAddr), len)) {
            return std::unexpected{"bind() failed. Error: " + err2String(errno)};
        }

        if (RESULT_SUCCESS != ::listen(serverSocket, 32)) {
            return std::unexpected{"listen() failed. Error: " + err2String(errno)};
        }

        return true;
    }

    void UDSAsynchServer::removeClosedHandles()
    {
        uint32_t pos = 1;
        for (uint32_t idx = pos; idx < handlesCount; ++idx ) {
            if (fds[idx].fd != -1)
                std::swap(fds[idx], fds[pos++]);
        }
        handlesCount = pos;
    }

    void UDSAsynchServer::closeEvent(pollfd& pollEvent)
    {
        ::close(pollEvent.fd);
        pollEvent.fd = -1;
    }

    bool UDSAsynchServer::start()
    {
        std::string message;
        uint32_t currentSize { 0 };
        while (true)
        {
            if (const int32_t result = ::poll(fds.data(), handlesCount, TIMEOUT); SOCKET_ERROR == result) {
                return error("poll() failed ");
            } else if (0 == result) {
                /** Timeout **/
            }

            currentSize = handlesCount;
            for (uint32_t idx = 0; idx < currentSize; idx++)
            {
                pollfd& pollEvent { fds[idx] };
                if (0 == pollEvent.revents)
                    continue;
                if (pollEvent.revents != POLLIN)
                {
                    if (pollEvent.revents & POLLHUP)
                    {
                        closeEvent(pollEvent);
                        break;
                    }
                }
                else if (pollEvent.fd == serverSocket) /** Listening descriptor is readable. **/
                {
                    Socket clientSocket { INVALID_HANDLE };
                    while (true)
                    {
                        clientSocket = ::accept(serverSocket, nullptr, nullptr);
                        if (SOCKET_ERROR == clientSocket)
                        {
                            if (errno != EWOULDBLOCK || EAGAIN != errno) {
                                return error("accept() failed = " + std::to_string(pollEvent.revents));
                            } else {
                                break;
                            }
                        }
                        else
                        {
                            std::cout << "New incoming connection. Client socket = " << clientSocket << std::endl;
                            if (const auto result = setSocketToNonBlock(clientSocket); !result) {
                                std::cerr << result.error() << std::endl;
                            }

                            fds[handlesCount].fd = clientSocket;
                            fds[handlesCount].events = POLLIN | POLLHUP;
                            ++handlesCount;
                            break;
                        }
                    }
                }
                else
                {
                    const Socket clientSocket = pollEvent.fd;
                    while (true)
                    {
                        const int64_t bytesRead = ::recv(clientSocket, buffer.data(), BUFFER_SIZE, 0);
                        if (SOCKET_ERROR == bytesRead)
                        {
                            if (errno != EWOULDBLOCK || EAGAIN != errno) {
                                return error("recv() failed = " + std::to_string(pollEvent.revents));
                            } else {
                                break;
                            }
                        }
                        else if (0 == bytesRead)
                        {
                            closeEvent(pollEvent);
                            break;
                        }
                        else
                        {
                            message.assign(buffer.data(), bytesRead);
                            eventQueue.push(std::move(message));
                        }
                    }
                }
            }
        }
    }
}