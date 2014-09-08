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

#include "unity_helper.h"
#include "kinetic_types_internal.h"
#include "kinetic_socket.h"
#include "kinetic_logger.h"
#include "kinetic_proto.h"
#include "kinetic_message.h"

#include "protobuf-c/protobuf-c.h"
#ifndef _BSD_SOURCE
    #define _BSD_SOURCE
#endif // _BSD_SOURCE
#include <unistd.h>
#include <sys/types.h>
#include "socket99/socket99.h"

#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

static int FileDesc;
static int KineticTestPort = KINETIC_PORT;
static ByteArray TestData;
static bool LogInitialized = false;

void setUp(void)
{
    FileDesc = -1;
    if (!LogInitialized)
    {
        KineticLogger_Init(NULL);//"test_kinetic_socket.log");
        LogInitialized = true;
    }
    TestData = BYTE_ARRAY_INIT_FROM_CSTRING("Some like it hot!");
}

void tearDown(void)
{
    if (FileDesc >= 0)
    {
        LOG("Shutting down socket...");
        KineticSocket_Close(FileDesc);
        FileDesc = 0;
    }
}

void test_KineticSocket_KINETIC_PORT_should_be_8123(void) {LOG_LOCATION;
    TEST_ASSERT_EQUAL(8123, KINETIC_PORT);
}

void test_KineticSocket_Connect_should_create_a_socket_connection(void) {LOG_LOCATION;
    FileDesc = KineticSocket_Connect("localhost", KineticTestPort, true);
    TEST_ASSERT_TRUE_MESSAGE(FileDesc >= 0, "File descriptor invalid");
}


#if 0
// Disabling socket read/write tests in not OSX, since Linux TravisCI builds
// fail, but system test passes. Most likely an issue with KineticRuby server
#if defined(__APPLE__)

void test_KineticSocket_Write_should_write_the_data_to_the_specified_socket(void)
{   LOG_LOCATION;
    bool success = false;
    uint8_t bufferData[40];
    ByteArray buffer = {.data = bufferData, .len = sizeof(bufferData)};

    FileDesc = KineticSocket_Connect("localhost", KineticTestPort, true);
    TEST_ASSERT_TRUE_MESSAGE(FileDesc >= 0, "File descriptor invalid");

#if defined(__unix__) && !defined(__APPLE__)
    TEST_IGNORE_MESSAGE("Disabled on Linux until KineticRuby server client connection cleanup is fixed!");
#endif

    success = KineticSocket_Write(FileDesc, TestData);
    TEST_ASSERT_TRUE_MESSAGE(success, "Failed to write to socket!");

    LOG("Flushing socket read pipe...");
    KineticSocket_Read(FileDesc, buffer);
}


void test_KineticSocket_WriteProtobuf_should_write_serialized_protobuf_to_the_specified_socket(void)
{
    LOG_LOCATION;
    bool success = false;
    uint8_t bufferData[5];
    ByteArray buffer = {.data = bufferData, .len = sizeof(bufferData)};

    KineticMessage_Init(&Msg);
    Msg.header.clusterVersion = 12345678;
    Msg.header.has_clusterVersion = true;
    Msg.header.identity = -12345678;
    Msg.header.has_identity = true;

    FileDesc = KineticSocket_Connect("localhost", KineticTestPort, true);
    TEST_ASSERT_TRUE_MESSAGE(FileDesc >= 0, "File descriptor invalid");

#if defined(__unix__) && !defined(__APPLE__)
    TEST_IGNORE_MESSAGE("Disabled on Linux until KineticRuby server client connection cleanup is fixed!");
#endif

    LOG("Writing a dummy protobuf...");
    success = KineticSocket_WriteProtobuf(FileDesc, &Msg.proto);
    TEST_ASSERT_TRUE_MESSAGE(success, "Failed to write to socket!");

    LOG("Flushing socket read pipe...");
    KineticSocket_Read(FileDesc, buffer);
}



void test_KineticSocket_Read_should_read_data_from_the_specified_socket(void)
{
    LOG_LOCATION;
    bool success = false;
    ByteArray readRequest = BYTE_ARRAY_INIT_FROM_CSTRING("read(5)");
    uint8_t bufferData[5];
    ByteArray buffer = {.data = bufferData, .len = sizeof(bufferData)};

    FileDesc = KineticSocket_Connect("localhost", KineticTestPort, true);
    TEST_ASSERT_TRUE_MESSAGE(FileDesc >= 0, "File descriptor invalid");

    // Send request to test server to send us some data
    success = KineticSocket_Write(FileDesc, readRequest);
    TEST_ASSERT_TRUE(success);

    success = KineticSocket_Read(FileDesc, buffer);

    TEST_ASSERT_TRUE_MESSAGE(success, "Failed to read from socket!");
}

void test_KineticSocket_Read_should_timeout_if_requested_data_is_not_received_within_configured_timeout(void)
{
    LOG_LOCATION;
    bool success = false;
    uint8_t bufferData[64];
    ByteArray buffer = {.data = bufferData, .len = sizeof(bufferData)};

    FileDesc = KineticSocket_Connect("localhost", KineticTestPort, true);
    TEST_ASSERT_TRUE_MESSAGE(FileDesc >= 0, "File descriptor invalid");

    success = KineticSocket_Read(FileDesc, buffer);

    TEST_ASSERT_FALSE_MESSAGE(success,
        "Expected socket to timeout waiting on data!");
}

void test_KineticSocket_ReadProtobuf_should_read_the_specified_length_of_an_encoded_protobuf_from_the_specified_socket(void)
{
    LOG_LOCATION;
    bool success = false;
    const ByteArray readRequest = BYTE_ARRAY_INIT_FROM_CSTRING("readProto()");
    uint8_t bufferData[PDU_VALUE_MAX_LEN];
    size_t expectedLength = 125; // This would normally be extracted from the PDU header
    ByteArray buffer = {.data = bufferData, .len = expectedLength};

    FileDesc = KineticSocket_Connect("localhost", KineticTestPort, true);
    TEST_ASSERT_TRUE_MESSAGE(FileDesc >= 0, "File descriptor invalid");

    // Send request to test server to send us a Kinetic protobuf
    success = KineticSocket_Write(FileDesc, readRequest);
    TEST_ASSERT_TRUE(success);

    // Receive the response
    success = KineticSocket_ReadProtobuf(FileDesc, &pProto, buffer);

    TEST_ASSERT_TRUE(success);
    TEST_ASSERT_NOT_NULL_MESSAGE(pProto,
        "Protobuf pointer was NULL, but expected dynamic memory allocation!");
    LOG( "Received Kinetic protobuf:");
    LOGF("  command: (0x%zX)", (size_t)pProto->command);
    LOGF("    header: (0x%zX)", (size_t)pProto->command->header);
    LOGF("      identity: %016llX",
        (unsigned long long)pProto->command->header->identity);
    KineticLogger_LogByteArray("  hmac", pProto->hmac);

    LOG("Kinetic ProtoBuf read successfully!");
}

void test_KineticSocket_ReadProtobuf_should_return_false_if_KineticProto_of_specified_length_fails_to_be_read_within_timeout(void)
{
    LOG_LOCATION;
    bool success = false;
    ByteArray readRequest = BYTE_ARRAY_INIT_FROM_CSTRING("readProto()");
    uint8_t bufferData[256];
    size_t expectedLength = 150; 
    // This would normally be extracted from the PDU header
    ByteArray buffer = {.data = bufferData, .len = expectedLength};

    FileDesc = KineticSocket_Connect("localhost", KineticTestPort, true);
    TEST_ASSERT_TRUE_MESSAGE(FileDesc >= 0, "File descriptor invalid");

    // Send request to test server to send us a Kinetic protobuf
    success = KineticSocket_Write(FileDesc, readRequest);
    TEST_ASSERT_TRUE(success);

    success = KineticSocket_ReadProtobuf(FileDesc, &pProto, buffer);
    TEST_ASSERT_FALSE_MESSAGE(success, "Expected timeout!");
    TEST_ASSERT_NULL_MESSAGE(pProto,
        "Protobuf pointer should not have gotten set, "
        "since no memory allocated.");
}

#endif // defined(__APPLE__)

#endif
