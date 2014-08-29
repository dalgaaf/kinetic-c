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
#include "kinetic_message.h"
#include "kinetic_pdu.h"
#include "kinetic_logger.h"
#include "kinetic_operation.h"
#include "kinetic_hmac.h"
#include "kinetic_nbo.h"
#include "mock_kinetic_connection.h"
#include "mock_kinetic_socket.h"

void setUp(void)
{
    KineticClient_Init(NULL);
}

void tearDown(void)
{
}

void test_Get_should_retrieve_an_object_from_the_device_and_store_in_supplied_byte_array(void)
{
    KineticPDU request, response;
    KineticMessage requestMsg;
    KineticConnection connection;
    const KineticConnectionConfig connectionConfig = {
        .host = "nicehost.org",
        .port = 8899,
        .clusterVersion = 9876,
        .identity = 1234,
        .key = BYTE_ARRAY_INIT_FROM_CSTRING("123abcXYZ"),
    };
    KINETIC_CONNECTION_INIT(&connection, &connectionConfig);
    connection.socketDescriptor = 777;

    // Establish connection
    KineticConnection_Connect_ExpectAndReturn(&connection, &connectionConfig, true);
    bool success = KineticClient_Connect(&connection, &connectionConfig);
    TEST_ASSERT_TRUE(success);

    // Send the request
    KineticConnection_IncrementSequence_Expect(&connection);
    ByteArray requestHeader = {.data = (uint8_t*)&request.header, .len = sizeof(KineticPDUHeader)};
    KineticSocket_Write_ExpectAndReturn(connection.socketDescriptor, requestHeader, true);
    KineticSocket_WriteProtobuf_ExpectAndReturn(connection.socketDescriptor, &requestMsg.proto, true);

    //--------------------------------------------------------------------------
    // Create a fake response
    //--------------------------------------------------------------------------

    // Create protobuf message
    KineticProto responseProto = KINETIC_PROTO__INIT;
    response.proto = &responseProto;

        // Create command
        KineticProto_Command responseCommand = KINETIC_PROTO_COMMAND__INIT;
        responseProto.command = &responseCommand;

            // Create header
            KineticProto_Header responseHeader = KINETIC_PROTO_HEADER__INIT;
            responseCommand.header = &responseHeader;

            // Create operation status
            KineticProto_Status responseStatus = KINETIC_PROTO_STATUS__INIT;
            responseStatus.code = KINETIC_PROTO_STATUS_STATUS_CODE_SUCCESS;
            responseStatus.has_code = true;
            responseCommand.status = &responseStatus;

        // Create HMAC
        uint8_t hmacData[64];
        responseProto.has_hmac = true;
        responseProto.hmac.data = hmacData;
        KineticHMAC respTempHMAC;
        KineticHMAC_Populate(&respTempHMAC, &responseProto, connection.config.key);

    // Create value paylod
    ByteArray valueKey = BYTE_ARRAY_INIT_FROM_CSTRING("my_key_3.1415927");
    const Kinetic_KeyValue metadata = {.key = valueKey};
    ByteArray value = BYTE_ARRAY_INIT_FROM_CSTRING("lorem ipsum...");
    KineticPDU_AttachValuePayload(&response, value);

    // Create messasge header
    response.headerNBO = (KineticPDUHeader) {
        .versionPrefix = (uint8_t)'F',
        .protobufLength = KineticNBO_FromHostU32(response.header.protobufLength),
        .valueLength = value.len,
    };

    //--------------------------------------------------------------------------
    // Expect to receive the response
    //--------------------------------------------------------------------------

    KineticConnection_IncrementSequence_Expect(&connection);
    ByteArray responseHeaderRaw = {.data = (uint8_t*)&response.headerNBO, .len = sizeof(KineticPDUHeader)};
    KineticSocket_Read_ExpectAndReturn(connection.socketDescriptor, responseHeaderRaw, true);

    ByteArray responseProtobuf = {.data = response.protobufScratch, .len = response.header.protobufLength};
    KineticSocket_ReadProtobuf_ExpectAndReturn(connection.socketDescriptor, &response.proto, responseProtobuf, true);
    KineticSocket_Read_ExpectAndReturn(connection.socketDescriptor, value, true);

    // TEST_IGNORE_MESSAGE("FIXME: Buffer reference issue in the GET integration test");

    //--------------------------------------------------------------------------
    // Execute the operation and validate the result
    //--------------------------------------------------------------------------
    KineticOperation operation = KineticClient_CreateOperation(&connection, &request, &requestMsg, &response);
    KineticProto_Status_StatusCode status = KineticClient_Get(&operation, &metadata, value);

    TEST_ASSERT_EQUAL_KINETIC_STATUS(KINETIC_PROTO_STATUS_STATUS_CODE_SUCCESS, status);
    TEST_ASSERT_EQUAL_PTR(value.data, response.value.data);
}

void test_Get_should_create_retrieve_an_object_from_the_device_and_store_in_embedded_ByteArray(void)
{
    TEST_IGNORE_MESSAGE("TODO: Is this test necessary at integration level?");
#if 0
    KineticPDU request, response;
    KineticMessage requestMsg;

    // Establish connection
    KineticConnection connection;
    const KineticConnectionConfig connectionConfig = {
        .host = "localhost",
        .port = KINETIC_PORT,
        .clusterVersion = 9876,
        .identity = 1234,
        .key = BYTE_ARRAY_INIT_FROM_CSTRING("123abcXYZ"),
    };
    KINETIC_CONNECTION_INIT(&connection, &connectionConfig);
    connection.socketDescriptor = 783;
    KineticConnection_Connect_ExpectAndReturn(&connection, &connectionConfig, true);
    bool success = KineticClient_Connect(&connection, &connectionConfig);
    TEST_ASSERT_TRUE(success);

    //--------------------------------------------------------------------------
    // Create a fake response
    //--------------------------------------------------------------------------

    // Create protobuf message
    KineticProto responseProto = KINETIC_PROTO__INIT;
    response.proto = &responseProto;

            // Create command
            KineticProto_Command responseCommand = KINETIC_PROTO_COMMAND__INIT;
            responseProto.command = &responseCommand;

                // Create header
                KineticProto_Header responseHeader = KINETIC_PROTO_HEADER__INIT;
                responseCommand.header = &responseHeader;

                // Create operation status
                KineticProto_Status responseStatus = KINETIC_PROTO_STATUS__INIT;
                responseStatus.code = KINETIC_PROTO_STATUS_STATUS_CODE_SUCCESS;
                responseStatus.has_code = true;
                responseCommand.status = &responseStatus;

        // Create HMAC
        uint8_t hmacData[64];
        responseProto.has_hmac = true;
        responseProto.hmac.data = hmacData;
        KineticHMAC respTempHMAC;
        KineticHMAC_Populate(&respTempHMAC, &responseProto, connection.config.key);

    // Create value paylod
    ByteArray valueKey = BYTE_ARRAY_INIT_FROM_CSTRING("my_key_3.1415927");
    const Kinetic_KeyValue metadata = {.key = valueKey};
    ByteArray value = BYTE_ARRAY_INIT_FROM_CSTRING("lorem ipsum...");
    KineticPDU_AttachValuePayload(&response, value);

    // Create messasge header
    response.header.protobufLength = 123;
    response.headerNBO = (KineticPDUHeader) {
        .versionPrefix = (uint8_t)'F',
        .protobufLength = KineticNBO_FromHostU32(response.header.protobufLength),
        .valueLength = KineticNBO_FromHostU32(value.len),
    };

    // Send the request
    KineticConnection_IncrementSequence_Expect(&connection);
    ByteArray requestHeader = {.data = (uint8_t*)&request.header, .len = sizeof(KineticPDUHeader)};
    KineticSocket_Write_ExpectAndReturn(connection.socketDescriptor, requestHeader, true);
    KineticSocket_WriteProtobuf_ExpectAndReturn(connection.socketDescriptor, &requestMsg.proto, true);


    //--------------------------------------------------------------------------
    // Expect to receive the response
    //--------------------------------------------------------------------------

    ByteArray responseHeaderRaw = {.data = (uint8_t*)&response.headerNBO, .len = sizeof(KineticPDUHeader)};
    KineticSocket_Read_ExpectAndReturn(connection.socketDescriptor, responseHeaderRaw, true);
    ByteArray responseProtobuf = {.data = response.protobufScratch, .len = response.header.protobufLength};
    KineticSocket_ReadProtobuf_ExpectAndReturn(connection.socketDescriptor, &response.proto, responseProtobuf, true);
    KineticSocket_Read_ExpectAndReturn(connection.socketDescriptor, value, true);

    // TEST_IGNORE_MESSAGE("FIXME: Buffer reference issue in the GET integration test");

    // Execute the operation
    KineticOperation operation = KineticClient_CreateOperation(
        &connection, &request, &requestMsg, &response);
    KineticProto_Status_StatusCode status =
        KineticClient_Get(&operation, &metadata, BYTE_ARRAY_NONE);

    TEST_ASSERT_EQUAL_KINETIC_STATUS(KINETIC_PROTO_STATUS_STATUS_CODE_SUCCESS, status);
    TEST_ASSERT_EQUAL_PTR(response.valueBuffer, response.value.data);
#endif
}
