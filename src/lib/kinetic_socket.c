/*
* kinetic-c
* Copyright (C) 2014 Seagate Technology.
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*
*/

#include "kinetic_socket.h"
#include "kinetic_logger.h"
#include "protobuf-c.h"

#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifndef _BSD_SOURCE
    #define _BSD_SOURCE
#endif // _BSD_SOURCE
#include <unistd.h>
#include <sys/types.h>
#include "socket99.h"

int KineticSocket_Connect(char* host, int port, bool nonBlocking)
{
    char port_str[32];
    struct addrinfo hints;
    struct addrinfo* ai_result = NULL;
    struct addrinfo* ai = NULL;
    socket99_result result;

    // Setup server address info
    socket99_config cfg = {
        .host = host,
        .port = port,
        .nonblocking = nonBlocking,
    };
    sprintf(port_str, "%d", port);

    // Open socket
    LOGF("\nConnecting to %s:%d", host, port);
    if (!socket99_open(&cfg, &result))
    {
        LOGF("Failed to open socket connection with host: status %d, errno %d",
            result.status, result.saved_errno);
        return -1;
    }

    // Configure the socket
    socket99_set_hints(&cfg, &hints);
    if (getaddrinfo(cfg.host, port_str, &hints, &ai_result) != 0)
    {
        LOGF("Failed to get socket address info: errno %d", errno);
        close(result.fd);
        return -1;
    }

    for (ai = ai_result; ai != NULL; ai = ai->ai_next)
    {
        // OSX won't let us set close-on-exec when creating the socket, so set it separately
        int current_fd_flags = fcntl(result.fd, F_GETFD);
        if (current_fd_flags == -1)
        {
            LOG("Failed to get socket fd flags");
            close(result.fd);
            continue;
        }
        if (fcntl(result.fd, F_SETFD, current_fd_flags | FD_CLOEXEC) == -1)
        {
            LOG("Failed to set socket close-on-exit");
            close(result.fd);
            continue;
        }

        #if defined(SO_NOSIGPIPE) && !defined(__APPLE__)
        // On BSD-like systems we can set SO_NOSIGPIPE on the socket to prevent it from sending a
        // PIPE signal and bringing down the whole application if the server closes the socket
        // forcibly
        {
            int set = 1;
            int setsockopt_result = setsockopt(socket_fd, SOL_SOCKET, SO_NOSIGPIPE, &set, sizeof(set));
            // Allow ENOTSOCK because it allows tests to use pipes instead of real sockets
            if (setsockopt_result != 0 && setsockopt_result != ENOTSOCK)
            {
                LOG("Failed to set SO_NOSIGPIPE on socket");
                close(result.fd);
                continue;
            }
        }
        #endif

        break;
    }

    freeaddrinfo(ai_result);

    if (ai == NULL)
    {
        // we went through all addresses without finding one we could bind to
        LOGF("Could not connect to %s:%d", host, port);
        return -1;
    }

    return result.fd;
}

void KineticSocket_Close(int socketDescriptor)
{
    if (socketDescriptor == -1)
    {
        LOG("Not connected so no cleanup needed");
    }
    else
    {
        LOGF("Closing socket with fd=%d", socketDescriptor);
        if (close(socketDescriptor))
        {
            LOG("Socket closed successfully");
        }
        else
        {
            LOG("Error closing socket file descriptor!");
        }
    }
}

bool KineticSocket_Read(int socketDescriptor, ByteArray buffer)
{
    LOGF("Reading %zd bytes into buffer @ 0x%zX from fd=%d", buffer.len, (size_t)buffer.data, socketDescriptor);

    size_t count;

    for (count = 0; count < buffer.len; )
    {
        int status;
        fd_set readSet;
        struct timeval timeout;

        // Time out after 5 seconds
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;

        FD_ZERO(&readSet);
        FD_SET(socketDescriptor, &readSet);
        status = select(socketDescriptor+1, &readSet, NULL, NULL, &timeout);

        if (status < 0) // Error occurred
        {
            LOGF("Failed waiting to read from socket! status=%d, errno=%d, desc='%s'",
                status, errno, strerror(errno));
            return false;
        }
        else if (status == 0) // Timeout occurred
        {
            LOG("Timed out waiting for socket data to arrive!");
            return false;
        }
        else if (status > 0) // Data available to read
        {
            // The socket is ready for reading
            status = read(socketDescriptor, &buffer.data[count], buffer.len - count);
            if (status == -1 && errno == EINTR)
            {
                continue;
            }
            else if (status <= 0)
            {
                LOGF("Failed to read from socket! status=%d, errno=%d, desc='%s'",
                status, errno, strerror(errno));
                return false;
            }
            else
            {
                count += status;
                LOGF("Received %d bytes (%zd of %zd)", status, count, buffer.len);
            }
        }
    }
    LOGF("Received %zd of %zd bytes requested", count, buffer.len);
    return true;
}

bool KineticSocket_ReadProtobuf(int socketDescriptor, KineticProto** message, const ByteArray buffer)
{
    LOGF("Reading %zd bytes of protobuf", buffer.len);
    bool success = false;
    KineticProto* unpacked = NULL;

    if (KineticSocket_Read(socketDescriptor, buffer))
    {
        LOG("Read completed!");
        unpacked = KineticProto__unpack(NULL, buffer.len, buffer.data);
        if (unpacked == NULL)
        {
            LOG("Error unpacking incoming Kinetic protobuf message!");
            *message = NULL;
        }
        else
        {
            *message = unpacked;
            success = true;
        }
    }
    else
    {
        *message = NULL;
    }
    return success;
}

bool KineticSocket_Write(int socketDescriptor, const ByteArray buffer)
{
    size_t count;

    LOGF("Writing %zu bytes to socket...", buffer.len);
    for (count = 0; count < buffer.len; )
    {
        int status;
        status = write(socketDescriptor, &buffer.data[count], buffer.len - count);
        if (status == -1 && errno == EINTR)
        {
            LOG("Write interrupted. retrying...");
            continue;
        }
        else if (status <= 0)
        {
            LOGF("Failed to write to socket! status=%d, errno=%d\n", status, errno);
            return false;
        }
        else
        {
            count += status;
            LOGF("Wrote %d bytes (%zu of %zu sent)", status, count, buffer.len);
        }
    }
    LOG("Write complete");

    return true;
}

bool KineticSocket_WriteProtobuf(int socketDescriptor, const KineticProto* proto)
{
    size_t len = KineticProto__get_packed_size(proto);
    LOGF("Writing protobuf (%zd bytes)...", len);
    uint8_t* packed = malloc(len);
    assert(packed);
    size_t packedLen = KineticProto__pack(proto, packed);
    assert(packedLen == len);
    ByteArray buffer = {.data = packed, .len = packedLen};

    bool success = KineticSocket_Write(socketDescriptor, buffer);
    free(packed);

    return success;
}
