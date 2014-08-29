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

#include "unity.h"
#include "kinetic_connection.h"
#include "protobuf-c.h"
#include "kinetic_proto.h"
#include "unity_helper.h"
#include "mock_kinetic_socket.h"
#include "kinetic_message.h"
#include "kinetic_pdu.h"
#include "kinetic_hmac.h"
#include "kinetic_logger.h"
#include "kinetic_logger.h"
#include "kinetic_nbo.h"
#include <string.h>
#include <time.h>

static const KineticConnectionConfig DefaultConfig = {
    .host = "localhost",
    .port = KINETIC_PORT,
    .clusterVersion = 12,
    .identity = 1234,
    .key = BYTE_ARRAY_INIT_FROM_CSTRING("arbitrary key value..."),
};
static KineticConnection Connection, Expected;

void setUp(void)
{
    KINETIC_CONNECTION_INIT(&Connection, &DefaultConfig);
    Expected = Connection;
}

void tearDown(void)
{
}

void test_KineticConnection_Init_should_create_a_default_connection_object(void)
{
    KineticConnection connection;
    time_t curTime = time(NULL);
    KineticConnectionConfig config = {
        .identity = 1234,
        .key = BYTE_ARRAY_INIT_FROM_CSTRING("arbitrary key value..."),
    };
    KINETIC_CONNECTION_INIT(&connection, &config);

    TEST_ASSERT_FALSE(connection.connected);
    TEST_ASSERT_FALSE(connection.config.nonBlocking);
    TEST_ASSERT_EQUAL(0, connection.config.port);
    TEST_ASSERT_EQUAL(-1, connection.socketDescriptor);
    TEST_ASSERT_EQUAL_STRING("", connection.config.host);
    // Give 1-second flexibility in the rare case that
    // we were on a second boundary
    TEST_ASSERT_INT64_WITHIN(curTime, connection.connectionID, 1);
    TEST_ASSERT_EQUAL_INT64(0, connection.config.clusterVersion);
    TEST_ASSERT_EQUAL_INT64(1234, connection.config.identity);
    TEST_ASSERT_EQUAL_BYTE_ARRAY(DefaultConfig.key, Connection.config.key);
    TEST_ASSERT_EQUAL_INT64(0, Connection.sequence);
}

#if 0
void test_KineticConnection_Connect_should_report_a_failed_connection(void)
{
    Connection = (KineticConnection){
        .connected = true,
        .nonBlocking = false,
        .port = 1234,
        .socketDescriptor = -1,
        .host = "invalid-host.com",
        .key = BYTE_ARRAY_INIT_FROM_CSTRING("some_hmac_key"),
        .identity = 456789,
    };
    Expected = Connection;

    KineticSocket_Connect_ExpectAndReturn(Expected.host, Expected.port, false, -1);

    bool success = KineticConnection_Connect(&Connection, "invalid-host.com", Expected.port,
        Expected.nonBlocking, Expected.clusterVersion, Expected.identity, Expected.key);

    TEST_ASSERT_FALSE(success);
    TEST_ASSERT_FALSE(Connection.connected);
    TEST_ASSERT_EQUAL(-1, Connection.socketDescriptor);
}

void test_KineticConnection_Connect_should_connect_to_specified_host_with_a_blocking_connection(void)
{
    Connection = (KineticConnection){
        .host = "invalid-host.com",
        .nonBlocking = true,
        .key = BYTE_ARRAY_INIT_FROM_CSTRING("invalid"),
        .socketDescriptor = -1,
        .connected = false,
    };
    Expected = Connection;
    Expected.nonBlocking = false;
    Expected.port = 1234;
    Expected.clusterVersion = 17;
    Expected.identity = 12;
    Expected.socketDescriptor = 24;
    strcpy(Expected.host, "valid-host.com");
    // strcpy(Expected.key.data, "some_hmac_key");
    // Expected.key.len =

    KineticSocket_Connect_ExpectAndReturn(Expected.host, Expected.port, false, Expected.socketDescriptor);

    bool success = KineticConnection_Connect(&Connection, "valid-host.com", Expected.port,
        Expected.nonBlocking, Expected.clusterVersion, Expected.identity, Expected.key);

    TEST_ASSERT_TRUE(success);
    TEST_ASSERT_TRUE(Connection.connected);
    TEST_ASSERT_EQUAL_STRING("valid-host.com", Connection.host);
    TEST_ASSERT_EQUAL(Expected.port, Connection.port);
    TEST_ASSERT_FALSE(Connection.nonBlocking);
    TEST_ASSERT_EQUAL_INT64(Expected.clusterVersion, Connection.clusterVersion);
    TEST_ASSERT_EQUAL_INT64(Expected.identity, Connection.identity);
    TEST_ASSERT_EQUAL_BYTE_ARRAY(Expected.key, Connection.key);
    TEST_ASSERT_EQUAL(Expected.socketDescriptor, Connection.socketDescriptor);
}

void test_KineticConnection_Connect_should_connect_to_specified_host_with_a_non_blocking_connection(void)
{
    Connection = (KineticConnection){
        .connected = false,
        .nonBlocking = false,
        .port = 1234,
        .socketDescriptor = -1 ,
        .host = "valid-host.com",
        .key = Key,
        .identity = Identity,
    };
    Expected = Connection;

    KineticSocket_Connect_ExpectAndReturn("valid-host.com", 2345, true, 48);

    bool success = KineticConnection_Connect(&Connection, "valid-host.com", 2345, true, ClusterVersion, Identity, Key);

    TEST_ASSERT_TRUE(success);
    TEST_ASSERT_TRUE(Connection.connected);
    TEST_ASSERT_TRUE(Connection.nonBlocking);
    TEST_ASSERT_EQUAL(2345, Connection.port);
    TEST_ASSERT_EQUAL(48, Connection.socketDescriptor);
    TEST_ASSERT_EQUAL_STRING("valid-host.com", Connection.host);
}

void test_KineticConnection_IncrementSequence_should_increment_the_sequence_count(void)
{
    Connection.sequence = 57;

    KineticConnection_IncrementSequence(&Connection);

    TEST_ASSERT_EQUAL_INT64(58, Connection.sequence);

    Connection.sequence = 57;

    KineticConnection_IncrementSequence(&Connection);

    TEST_ASSERT_EQUAL_INT64(58, Connection.sequence);
}
#endif
