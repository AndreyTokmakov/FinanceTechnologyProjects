/**============================================================================
Name        : UdsServerAsynch.h
Created on  : 14.04.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : UdsServerAsynch.h
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_UDSSERVERASYNCH__H
#define FINANCETECHNOLOGYPROJECTS_UDSSERVERASYNCH__H

#include <iostream>
#include <string_view>
#include <expected>
#include <thread>

#include <unistd.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/poll.h>
#include <netinet/in.h>
#include <fcntl.h>


#include "BaseService.h"


namespace Servers
{
    using Socket = int32_t;

    constexpr int32_t RESULT_SUCCESS = 0;
    constexpr int32_t INVALID_HANDLE = -1;
    constexpr int32_t SOCKET_ERROR = -1;
}

// TODO: Remove ---> Move to SERVER_UTILS .cpp file ??
namespace Servers::Utils
{
    std::string err2String(int32_t errorCode);

    template<typename ResultType = bool , ResultType value = false>
    ResultType error(const std::string_view message)
    {
        std::cerr << message << ". Error (" << errno << "): " << err2String(errno) << std::endl;
        return value;
    }

    template<typename ResultType = bool , ResultType value = false>
    ResultType error(const std::string_view message, const int32_t errorCode)
    {
        std::cerr << message << errorCode << std::endl;
        return value;
    }

}

namespace Servers
{
    using Common::Buffer;
    using Common::BaseServer;
    using Common::BaseService;

    template<typename ServiceType>
    struct UdsServerAsynch final: BaseServer<ServiceType>
    {
        using BaseServer<ServiceType>::process;

        // TODO: size_t --> size_type --> INT ?

        // TODO: Rename ???
        constexpr static size_t BUFFER_SIZE { 2 * 1024 };

        constexpr static size_t MAX_DESCRIPTORS { 256 };
        constexpr static int32_t TIMEOUT { 3 * 60 * 1000 };

        Socket serverSocket { INVALID_HANDLE };
        std::string filePath ;

        // TODO: Make one BUFFER per SESSION
        Buffer buffer;
        std::array<pollfd, MAX_DESCRIPTORS> fds {};
        uint32_t handlesCount { 0 };

        explicit UdsServerAsynch(std::string udmSockPath): filePath { std::move( udmSockPath ) }
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

        ~UdsServerAsynch() override
        {
            if (const int32_t result = ::remove(filePath.data()); INVALID_HANDLE == result) {
                Utils::error("remove() failed. Result = ", result);
            }
            if (const int32_t result = ::close(serverSocket); INVALID_HANDLE == result) {
                Utils::error("close() failed. Result = ", result);
            }
        }

        static void closeEvent(pollfd& pollEvent)
        {
            ::close(pollEvent.fd);
            pollEvent.fd = -1;
        }

        [[nodiscard]]
        std::expected<bool, std::string> init() const
        {
            if (const auto result = setSocketToNonBlock(serverSocket); !result.has_value()) {
                return std::unexpected{"setSocketToNonBlock() failed. Error: " + result.error()};
            }
            int32_t yes { 1 };
            if (SOCKET_ERROR == ::setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))) {
                return std::unexpected{"setsockopt(SOL_SOCKET, SO_REUSEADDR) failed. Error: " + Utils::err2String(errno)};
            }

            sockaddr_un serverAddr { .sun_family = AF_UNIX };
            std::copy(filePath.cbegin(), filePath.end(), serverAddr.sun_path);

            uint32_t len = sizeof(serverAddr);
            if (RESULT_SUCCESS != ::bind(serverSocket, reinterpret_cast<const sockaddr*>(&serverAddr), len)) {
                return std::unexpected {"bind() failed. Error: " + Utils::err2String(errno)};
            }

            if (RESULT_SUCCESS != ::listen(serverSocket, 32)) {
                return std::unexpected {"listen() failed. Error: " + Utils::err2String(errno)};
            }

            return true;
        }

        [[noreturn]] void run() override
        {
            uint32_t currentSize { 0 };
            while (true)
            {
                if (const int32_t result = ::poll(fds.data(), handlesCount, TIMEOUT); SOCKET_ERROR == result) {
                    Utils::error("poll() failed ");
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
                                    Utils::error("accept() failed = " + std::to_string(pollEvent.revents));
                                }
                                break;
                            }
                            if (const auto result = setSocketToNonBlock(clientSocket); !result) {
                                std::cerr << result.error() << std::endl;
                            }

                            fds[handlesCount].fd = clientSocket;
                            fds[handlesCount].events = POLLIN | POLLHUP;
                            ++handlesCount;
                            break;
                        }
                    }
                    else
                    {
                        const Socket clientSocket = pollEvent.fd;
                        int32_t bytesRead { SOCKET_ERROR };
                        while (true)
                        {
                            buffer.validateCapacity(BUFFER_SIZE);
                            bytesRead = ::recv(clientSocket, buffer.data(), BUFFER_SIZE, 0);
                            if (SOCKET_ERROR == bytesRead)
                            {
                                if (errno != EWOULDBLOCK || EAGAIN != errno) {
                                    Utils::error("recv() failed = " + std::to_string(pollEvent.revents));
                                }
                                break;
                            }
                            if (0 == bytesRead)
                            {
                                closeEvent(pollEvent);
                                buffer.reset(); // TODO: ????
                                break;
                            }

                            buffer.incrementLength(bytesRead);
                            if (BUFFER_SIZE > bytesRead)
                            {
                                process(buffer);
                                buffer.reset();
                            }
                        }
                    }
                }
            }
        }

        [[nodiscard]]
        static std::expected<bool, std::string> setSocketToNonBlock(const Socket socket)
        {
            if (const int flags = ::fcntl(socket, F_GETFL, 0);
                SOCKET_ERROR == ::fcntl(socket, F_SETFL, flags | O_NONBLOCK)) {
                return std::unexpected{"fcntl(F_SETFL, O_NONBLOCK) failed. Error: " + Utils::err2String(errno)};
            }
            return true;
        }
    };
}



#endif //FINANCETECHNOLOGYPROJECTS_UDSSERVERASYNCH__H
