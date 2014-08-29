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

#include "kinetic_connection.h"
#include "kinetic_socket.h"
#include <string.h>

bool KineticConnection_Connect(KineticConnection * connection,
    KineticConnectionConfig const * config)
{
    KINETIC_CONNECTION_INIT(connection, config);

    connection->socketDescriptor = KineticSocket_Connect(
        (char*)config->host, config->port, config->nonBlocking);
    connection->connected = (connection->socketDescriptor >= 0);

    return connection->connected;
}

void KineticConnection_Disconnect(KineticConnection* connection)
{
    if (connection != NULL || connection->socketDescriptor >= 0)
    {
        close(connection->socketDescriptor);
        connection->socketDescriptor = -1;
    }
}

void KineticConnection_IncrementSequence(KineticConnection* const connection)
{
    connection->sequence++;
}
