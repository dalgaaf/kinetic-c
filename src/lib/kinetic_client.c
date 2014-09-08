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

#include "kinetic_client.h"
#include "kinetic_types_internal.h"
#include "kinetic_pdu.h"
#include "kinetic_operation.h"
#include "kinetic_connection.h"
#include "kinetic_message.h"
#include "kinetic_pdu.h"
#include "kinetic_logger.h"
#include <stdio.h>

static KineticConnection* Connections[KINETIC_SESSIONS_MAX];

KineticProto_Status_StatusCode KineticClient_ExecuteOperation(KineticOperation* operation);

/**
 * @brief Creates and initializes a Kinetic operation.
 *
 * @param connection    KineticConnection instance to associate with operation
 * @param request       KineticPDU instance to use for request
 * @param requestMsg    KineticMessage instance to use for request
 * @param response      KineticPDU instance to use for reponse
 *
 * @return              Returns a configured operation instance
 */
KineticOperation KineticClient_CreateOperation(
    KineticConnection* connection,
    KineticPDU* request,
    KineticPDU* response);

int KineticClient_Init(KineticSession* session)
{
    KineticLogger_Init(logFile);
    for(int i = 0; i < KINETIC_PDUS_PER_SESSION_MAX; i++)
    {
        if (Connections[i] != NULL)
        {
            Connections[i] = malloc(sizeof(Connections));
            session->handle = i;
            return i;
        }
    }
    return -1;
}

bool KineticClient_Connect(KineticSession* session)
{
    if (session == NULL)
    {
        LOG("Specified KineticSession is NULL!");
        return false;
    }

    if (host == NULL)
    {
        LOG("Specified host is NULL!");
        return false;
    }

    if (key.len < 1)
    {
        LOG("Specified HMAC key is empty!");
        return false;
    }

    if (key.data == NULL)
    {
        LOG("Specified HMAC key is NULL!");
        return false;
    }

    if (!KineticConnection_Connect(&Connections[session->handle],
        session->host, session->port, session->nonBlocking,
        session->clusterVersion, session->identity, session->key))
    {
        Connections[session->handle].connected = false;
        Connections[session->handle].socketDescriptor = -1;
        LOGF("Failed creating connection to %s:%d",
            session->host, session->port);
        return false;
    }

    &Connections[session->handle].connected = true;

    return true;
}

void KineticClient_Disconnect(KineticSession* session)
{
    if (session->handle >= 0)
    {
        KineticConnection_Disconnect(&Connections[session->handle]);
        free(Connections[session->handle]);
        Connections[session->handle] = NULL;
    }
}

KineticOperation KineticClient_CreateOperation(
    KineticConnection* connection,
    KineticPDU* request,
    KineticPDU* response)
{
    KineticOperation op;

    if (connection == NULL)
    {
        LOG("Specified KineticConnection is NULL!");
        assert(connection != NULL);
    }

    if (request == NULL)
    {
        LOG("Specified KineticPDU request is NULL!");
        assert(request != NULL);
    }

    if (response == NULL)
    {
        LOG("Specified KineticPDU response is NULL!");
        assert(response != NULL);
    }

    KineticPDU_Init(request, connection);
    KINETIC_PDU_INIT_WITH_MESSAGE(request, connection);
    KineticPDU_Init(response, connection);

    op.connection = connection;
    op.request = request;
    request->proto->command = &request->message.command;
    op.response = response;

    return op;
}

Kinetic_Status KineticClient_NoOp(KineticSession* session)
{
    assert(operation->connection != NULL);
    assert(operation->request != NULL);
    assert(operation->response != NULL);

    // Initialize request
    KineticOperation_BuildNoop(operation);

    // Execute the operation
    return (Kinetic_Status)KineticClient_ExecuteOperation(operation);
}

Kinetic_Status KineticClient_Put(KineticSession* session,
    const Kinetic_KeyValue* metadata,
    const ByteArray value)
{
    assert(operation->connection != NULL);
    assert(operation->request != NULL);
    assert(operation->response != NULL);
    assert(value.data != NULL);
    assert(value.len <= PDU_VALUE_MAX_LEN);

    // Initialize request
    KineticOperation_BuildPut(operation, metadata, value);

    // Execute the operation
    return (Kinetic_Status)KineticClient_ExecuteOperation(operation);
}

Kinetic_Status KineticClient_Get(KineticSession* session,
    const Kinetic_KeyValue* metadata,
    const ByteArray value)
{
    assert(operation->connection != NULL);
    assert(operation->request != NULL);
    assert(operation->response != NULL);
    assert(metadata != NULL);
    assert(metadata->key.data != NULL);
    assert(metadata->key.len <= KINETIC_MAX_KEY_LEN);

    ByteArray responseValue = BYTE_ARRAY_NONE;
    if (!metadata->metadataOnly)
    {
        if (value.data != NULL)
        {
            responseValue = value;
        }
        else
        {
            responseValue = (ByteArray){.data = operation->response->valueBuffer};
        }
    }

    // Initialize request
    KineticOperation_BuildGet(operation, metadata, responseValue);

    // Execute the operation
    return (Kinetic_Status)KineticClient_ExecuteOperation(operation);
}

Kinetic_Status KineticClient_ExecuteOperation(KineticOperation* operation)
{
    KineticProto_Status_StatusCode status =
        KINETIC_PROTO_STATUS_STATUS_CODE_INVALID_STATUS_CODE;

    // Send the request
    if (KineticPDU_Send(operation->request))
    {
        // Associate response with same exchange as request
        operation->response->connection = operation->request->connection;

        // Receive the response
        if (KineticPDU_Receive(operation->response))
        {
            status = KineticPDU_Status(operation->response);
        }
    }

    return (Kinetic_Status)status;
}

