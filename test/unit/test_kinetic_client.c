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
    KineticLogger_Init_Expect("some/file.log");
    KineticClient_Init("some/file.log");
}

void tearDown(void)
{
}

void test_KineticClient_Init_should_initialize_the_logger(void)
{
    KineticLogger_Init_Expect("some/file.log");

    KineticClient_Init("some/file.log");
}

void test_KineticClient_Connect_should_configure_a_session_and_connect_to_specified_host(void)
{
    KineticConnection connection;
    const KineticConnectionConfig connectionConfig = {
        .host = "somehost.com",
        .port = KINETIC_PORT,
        .clusterVersion = 12,
        .identity = 1234,
        .key = BYTE_ARRAY_INIT_FROM_CSTRING("some_hmac_key"),
    };
    KINETIC_CONNECTION_INIT(&connection, &connectionConfig);

    // Ensure gets set appropriately by internal connect call
    connection.connected = false;

    KineticConnection_Connect_ExpectAndReturn(&connection, &connectionConfig, true);

    bool success = KineticClient_Connect(&connection, &connectionConfig);

    TEST_ASSERT_TRUE(success);
    TEST_ASSERT_TRUE(connection.connected);
}

void test_KineticClient_Connect_should_return_false_upon_NULL_connection(void)
{
    const KineticConnectionConfig connectionConfig = {
        .host = "somehost.com",
        .port = KINETIC_PORT,
        .clusterVersion = 12,
        .identity = 1234,
        .key = BYTE_ARRAY_INIT_FROM_CSTRING("some_hmac_key"),
    };
    ByteArray key = BYTE_ARRAY_INIT_FROM_CSTRING("some_hmac_key");
    KineticLogger_Log_Expect("Specified KineticConnection is NULL!");
    bool success = KineticClient_Connect(NULL, &connectionConfig);

    TEST_ASSERT_FALSE(success);
}

void test_KineticClient_Connect_should_return_false_upon_empty_host(void)
{
    const KineticConnectionConfig connectionConfig = {
        .host = "",
        .port = KINETIC_PORT,
        .clusterVersion = 12,
        .identity = 1234,
        .key = BYTE_ARRAY_INIT_FROM_CSTRING("some_hmac_key"),
    };
    KineticConnection connection;
    ByteArray key = BYTE_ARRAY_INIT_FROM_CSTRING("some_key");

    KineticLogger_Log_Expect("No host specified!");
    bool success = KineticClient_Connect(&connection, &connectionConfig);

    TEST_ASSERT_FALSE(success);
}

void test_KineticClient_Connect_should_return_false_upon_NULL_HMAC_key(void)
{
    const KineticConnectionConfig connectionConfig = {
        .host = "somehost.com",
        .port = KINETIC_PORT,
        .clusterVersion = 12,
        .identity = 1234,
        .key = (ByteArray){.data = NULL, .len = 5},
    };
    KineticConnection connection;

    KineticLogger_Log_Expect("Specified HMAC key is NULL!");
    bool success = KineticClient_Connect(&connection, &connectionConfig);

    TEST_ASSERT_FALSE(success);
}

void test_KineticClient_Connect_should_return_false_upon_empty_HMAC_key(void)
{
    const KineticConnectionConfig connectionConfig = {
        .host = "somehost.com",
        .port = KINETIC_PORT,
        .clusterVersion = 12,
        .identity = 1234,
        .key = BYTE_ARRAY_INIT_FROM_CSTRING(""),
    };
    KineticConnection connection;

    KineticLogger_Log_Expect("Specified HMAC key is empty!");
    bool success = KineticClient_Connect(&connection, &connectionConfig);

    TEST_ASSERT_FALSE(success);
}

void test_KineticClient_Connect_should_log_a_failed_connection_and_return_false(void)
{
    const KineticConnectionConfig connectionConfig = {
        .host = "somehost.com",
        .port = KINETIC_PORT,
        .clusterVersion = 12,
        .identity = 1234,
        .key = BYTE_ARRAY_INIT_FROM_CSTRING("some_key"),
    };
    KineticConnection connection;
    KINETIC_CONNECTION_INIT(&connection, &connectionConfig);

    // Ensure appropriately updated per internal connect call result
    connection.connected = true;
    connection.socketDescriptor = 333;

    KineticConnection_Connect_ExpectAndReturn(&connection, &connectionConfig, false);
    KineticLogger_LogPrintf_IgnoreAndReturn(888);

    TEST_ASSERT_FALSE(KineticClient_Connect(&connection, &connectionConfig));

    TEST_ASSERT_FALSE(connection.connected);
    TEST_ASSERT_EQUAL(-1, connection.socketDescriptor);
}

void test_KineticClient_CreateOperation_should_create_configure_and_return_a_valid_operation_instance(void)
{
    const KineticConnectionConfig connectionConfig = {
        .host = "somehost.com",
        .port = KINETIC_PORT,
        .clusterVersion = 12,
        .identity = 1234,
        .key = BYTE_ARRAY_INIT_FROM_CSTRING("some_key"),
    };
    KineticConnection connection;
    KINETIC_CONNECTION_INIT(&connection, &connectionConfig);
    KineticOperation op;
    KineticPDU request, response;
    KineticMessage requestMsg;

    KineticMessage_Init_Expect(&requestMsg);
    KineticPDU_Init_Expect(&request, &connection, &requestMsg);
    KineticPDU_Init_Expect(&response, &connection, NULL);

    op = KineticClient_CreateOperation(&connection, &request, &requestMsg, &response);

    TEST_ASSERT_EQUAL_PTR(&connection, op.connection);
    TEST_ASSERT_EQUAL_PTR(&request, op.request);
    TEST_ASSERT_EQUAL_PTR(&requestMsg, op.request->message);
    TEST_ASSERT_EQUAL_PTR(&response, op.response);
    TEST_ASSERT_NULL(op.response->message);
    TEST_ASSERT_NULL(op.response->proto);
}
