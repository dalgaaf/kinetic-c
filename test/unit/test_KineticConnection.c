/*
 * kinetic-c-client
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

#include "KineticConnection.h"
#include "unity.h"
#include "mock_KineticSocket.h"
#include <string.h>

static KineticConnection Connection, Expected;

void setUp(void)
{
    Connection = KineticConnection_Create();
    Expected = KineticConnection_Create();
}

void tearDown(void)
{
}

void test_KineticConnection_Create_should_create_a_default_connection_object(void)
{
    TEST_ASSERT(sizeof(KineticConnection) > 0);
    TEST_ASSERT_FALSE(Connection.Connected);
    TEST_ASSERT_TRUE(Connection.Blocking);
    TEST_ASSERT_EQUAL(0, Connection.Port);
    TEST_ASSERT_EQUAL(-1, Connection.FileDescriptor);
    TEST_ASSERT_EQUAL_STRING("", Connection.Host);
}

void test_KineticConnection_Connect_should_report_a_failed_connection(void)
{
    Connection.Connected = true;
    Connection.Blocking = true;
    Connection.Port = 1234;
    Connection.FileDescriptor = -1;
    strcpy(Connection.Host, "invalid-host.com");
    Expected = Connection;

    KineticSocket_Connect_ExpectAndReturn(Expected.Host, Expected.Port, true, -1);

    TEST_ASSERT_FALSE(KineticConnection_Connect(&Connection, "invalid-host.com", 1234, true));
    TEST_ASSERT_FALSE(Connection.Connected);
    TEST_ASSERT_EQUAL(-1, Connection.FileDescriptor);
}

void test_KineticConnection_Connect_should_connect_to_specified_host_with_a_blocking_connection(void)
{
    KineticSocket_Connect_ExpectAndReturn("valid-host.com", 1234, true, 24);

    KineticConnection_Connect(&Connection, "valid-host.com", 1234, true);

    TEST_ASSERT_TRUE(Connection.Connected);
    TEST_ASSERT_TRUE(Connection.Blocking);
    TEST_ASSERT_EQUAL(1234, Connection.Port);
    TEST_ASSERT_EQUAL(24, Connection.FileDescriptor);
    TEST_ASSERT_EQUAL_STRING("valid-host.com", Connection.Host);
}

void test_KineticConnection_Connect_should_connect_to_specified_host_with_a_non_blocking_connection(void)
{
    KineticSocket_Connect_ExpectAndReturn("valid-host.com", 2345, false, 48);

    KineticConnection_Connect(&Connection, "valid-host.com", 2345, false);

    TEST_ASSERT_TRUE(Connection.Connected);
    TEST_ASSERT_FALSE(Connection.Blocking);
    TEST_ASSERT_EQUAL(2345, Connection.Port);
    TEST_ASSERT_EQUAL(48, Connection.FileDescriptor);
    TEST_ASSERT_EQUAL_STRING("valid-host.com", Connection.Host);
}


void test_KineticConnection_SendMessage_should_send_the_specified_message_and_report_success(void)
{
    TEST_IGNORE();
}

void test_KineticConnection_SendMessage_should_send_the_specified_message_and_report_failure(void)
{
    TEST_IGNORE();
}


void test_KineticConnection_SendMessage_should_receive_a_message_for_the_exchange_and_report_success(void)
{
    TEST_IGNORE();
}

void test_KineticConnection_SendMessage_should_receive_a_message_for_the_exchange_and_report_failure(void)
{
    TEST_IGNORE();
}








