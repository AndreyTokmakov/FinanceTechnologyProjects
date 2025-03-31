/**============================================================================
Name        : UDSServer.cpp
Created on  : 31.03.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : UDSServer.cpp
============================================================================**/

#include "UDSServer.h"


#include <unistd.h>
#include <cerrno>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/poll.h>
#include <netinet/in.h>
#include <fcntl.h>

#include <iostream>
#include <string_view>

namespace Utils
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
}
