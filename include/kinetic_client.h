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

/**
 * Initializes the Kinetic API, onfigures logging destination and
 * returns a session handle
 *
 * @param logFile Path to log file. Specify NULL to log to STDOUT.
 */
int KineticClient_Init(KineticSession* session);

/**
 * @brief Configures the session and establishes a socket connection to a Kinetic Device
 *
 * @param session        KineticSession instance to configure with connection info
 * @param host              Host name or IP address to connect to
 * @param port              Port to establish socket connection on
 * @param nonBlocking       Set to true for non-blocking or false for blocking I/O
 * @param clusterVersion    Cluster version to use for the session
 * @param identity          Identity to use for the session
 * @param key               Key to use for HMAC calculations (NULL-terminated string)
 *
 * @return                  Returns true if connection succeeded
 */
bool KineticClient_Connect(KineticSession* session);

/**
 * @brief Closes the socket connection to a host and terminates the session
 *
 * @param session    KineticSession instance
 */
void KineticClient_Disconnect(KineticSession* session);

/**
 * @brief Executes a NOOP command to test whether the Kinetic Device is operational
 *
 * @param operation     KineticOperation instance to use for the operation
 *
 * @return              Returns 0 upon succes, -1 or the Kinetic status code upon failure
 */
Kinetic_Status KineticClient_NoOp(KineticSession* session);

/**
 * @brief Executes a PUT command to write data to the Kinetic Device
 *
 * @param operation     KineticOperation instance to use for the operation
 *
 * @return              Returns 0 upon succes, -1 or the Kinetic status code upon failure
 */
Kinetic_Status KineticClient_Put(KineticSession* session,
    const Kinetic_KeyValue* metadata,
    ByteArray value);

/**
 * @brief Executes a GET command to read data from the Kinetic Device
 *
 * @param operation     KineticOperation instance to use for the operation
 *
 * @return              Returns 0 upon succes, -1 or the Kinetic status code upon failure
 */
Kinetic_Status KineticClient_Get(KineticSession* session,
    const Kinetic_KeyValue* metadata,
    const ByteArray value);

#endif // _KINETIC_CLIENT_H
