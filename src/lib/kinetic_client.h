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

#ifndef _KINETIC_CLIENT_H
#define _KINETIC_CLIENT_H

#include "kinetic_types.h"
#include "kinetic_pdu.h"
#include "kinetic_operation.h"

/**
 * Initializes the Kinetic API andcsonfigures logging destination.
 *
 * @param logFile Path to log file. Specify NULL to log to STDOUT.
 */
void KineticClient_Init(char const* logFile);

/**
 * @brief Configures the session and establishes a socket connection to a Kinetic Device
 *
 * @param connection        KineticConnection instance to configure with connection info
 * @param host              Host name or IP address to connect to
 * @param port              Port to establish socket connection on
 * @param nonBlocking       Set to true for non-blocking or false for blocking I/O
 * @param clusterVersion    Cluster version to use for the session
 * @param identity          Identity to use for the session
 * @param key               Key to use for HMAC calculations (NULL-terminated string)
 *
 * @return                  Returns true if connection succeeded
 */
bool KineticClient_Connect(KineticConnection* connection,
    KineticConnectionConfig const* config);

/**
 * @brief Closes the socket connection to a host.
 *
 * @param connection    KineticConnection instance
 */
void KineticClient_Disconnect(KineticConnection* connection);

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
    KineticMessage* requestMsg,
    KineticPDU* response);

/**
 * @brief Executes a NOOP command to test whether the Kinetic Device is operational
 *
 * @param operation     KineticOperation instance to use for the operation
 *
 * @return              Returns 0 upon succes, -1 or the Kinetic status code upon failure
 */
KineticProto_Status_StatusCode KineticClient_NoOp(KineticOperation* operation);

/**
 * @brief Executes a PUT command to write data to the Kinetic Device
 *
 * @param operation     KineticOperation instance to use for the operation
 *
 * @return              Returns 0 upon succes, -1 or the Kinetic status code upon failure
 */
KineticProto_Status_StatusCode KineticClient_Put(KineticOperation* operation,
    const Kinetic_KeyValue* metadata,
    ByteArray value);

/**
 * @brief Executes a GET command to read data from the Kinetic Device
 *
 * @param operation     KineticOperation instance to use for the operation
 *
 * @return              Returns 0 upon succes, -1 or the Kinetic status code upon failure
 */
KineticProto_Status_StatusCode KineticClient_Get(KineticOperation* operation,
    const Kinetic_KeyValue* metadata,
    const ByteArray value);

#endif // _KINETIC_CLIENT_H
