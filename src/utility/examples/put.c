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

#include "put.h"

KineticProto_Status_StatusCode Put(Kinetic_KeyValue* metadata, const ByteArray value)
{
    KineticOperation operation;
    KineticPDU request, response;
    KineticMessage requestMsg;
    KineticProto_Status_StatusCode status =
        KINETIC_PROTO_STATUS_STATUS_CODE_INVALID_STATUS_CODE;

    operation = KineticClient_CreateOperation(
        &connection, &request, &requestMsg, &response);

    status = KineticClient_Put(&operation, metadata, value);

    if (status == KINETIC_PROTO_STATUS_STATUS_CODE_SUCCESS)
    {
        printf("Put operation completed successfully. Your data has been stored!\n");
    }

    return status;
}
