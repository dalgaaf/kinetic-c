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
#include "unity.h"
#include "unity_helper.h"
#include <stdio.h>
#include "protobuf-c.h"
#include "kinetic_proto.h"
#include "mock_kinetic_connection.h"
#include "mock_kinetic_message.h"
#include "mock_kinetic_pdu.h"
#include "mock_kinetic_logger.h"
#include "mock_kinetic_operation.h"

void setUp(void)
{
}

void tearDown(void)
{
}

void test_KineticClient_Put_should_execute_PUT_operation(void)
{
    KineticConnection connection;
    const KineticConnectionConfig connectionConfig = {
        .host = "nicehost.org",
        .port = 8899,
        .clusterVersion = 9876,
        .identity = 1234,
        .key = BYTE_ARRAY_INIT_FROM_CSTRING("123abcXYZ"),
    };
    KINETIC_CONNECTION_INIT(&connection, &connectionConfig);

    KineticProto responseProto = KINETIC_PROTO__INIT;
    KineticProto_Command responseCommand = KINETIC_PROTO_COMMAND__INIT;
    KineticProto_Status responseStatus = KINETIC_PROTO_STATUS__INIT;
    ByteArray hmacKey = BYTE_ARRAY_INIT_FROM_CSTRING("some_hmac_key");

    KineticProto_Status_StatusCode status;
    BYTE_ARRAY_CREATE(value, PDU_VALUE_MAX_LEN);
    const Kinetic_KeyValue const metadata = {
        .newVersion = BYTE_ARRAY_INIT_FROM_CSTRING("v2.0"),
        .key = BYTE_ARRAY_INIT_FROM_CSTRING("my_key_3.1415927"),
        .dbVersion = BYTE_ARRAY_INIT_FROM_CSTRING("v1.0"),
        .tag = BYTE_ARRAY_INIT_FROM_CSTRING("SomeTagValue"),
        .algorithm = KINETIC_PROTO_ALGORITHM_SHA1,
    };

    KineticMessage requestMsg;
    KINETIC_MESSAGE_INIT(&requestMsg);
    KineticPDU request;
    KINETIC_PDU_INIT(&request, &connection, &requestMsg);

    KineticPDU response;

    response.message = NULL;
    response.proto = &responseProto;
    response.proto->command = &responseCommand;
    response.proto->command->status = &responseStatus;
    response.proto->command->status->has_code = true;
    response.proto->command->status->code = KINETIC_PROTO_STATUS_STATUS_CODE_SUCCESS;

    KineticMessage_Init_Expect(&requestMsg);
    KineticPDU_Init_Expect(&request, &connection, &requestMsg);
    KineticPDU_Init_Expect(&response, &connection, NULL);
    KineticOperation operation = KineticClient_CreateOperation(&connection, &request, &requestMsg, &response);

    KineticOperation_BuildPut_Expect(&operation, &metadata, value);
    KineticPDU_Send_ExpectAndReturn(&request, true);
    KineticPDU_Receive_ExpectAndReturn(&response, true);
    KineticPDU_Status_ExpectAndReturn(&response, KINETIC_PROTO_STATUS_STATUS_CODE_SUCCESS);

    status = KineticClient_Put(&operation, &metadata, value);

    TEST_ASSERT_EQUAL_KINETIC_STATUS(KINETIC_PROTO_STATUS_STATUS_CODE_SUCCESS, status);
    TEST_ASSERT_EQUAL_PTR(&connection, response.connection);
}
