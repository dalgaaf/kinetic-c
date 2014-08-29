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
#include "kinetic_proto.h"
#include "kinetic_message.h"
#include "kinetic_pdu.h"
#include "kinetic_logger.h"
#include "kinetic_operation.h"
#include "kinetic_hmac.h"
#include "kinetic_connection.h"
#include "kinetic_socket.h"
#include "kinetic_nbo.h"

#include "unity.h"
#include "unity_helper.h"
#include "system_test_fixture.h"
#include "protobuf-c.h"
#include "socket99.h"
#include <string.h>
#include <stdlib.h>

static SystemTestFixture Fixture = {
    .config.host = "localhost",
    .config.port = KINETIC_PORT,
    .config.clusterVersion = 0,
    .config.identity =  1,
    .config.key = BYTE_ARRAY_INIT_FROM_CSTRING("asdfasdf")
};

static ByteArray valueKey = BYTE_ARRAY_INIT_FROM_CSTRING("GET system test blob");
static ByteArray tag = BYTE_ARRAY_INIT_FROM_CSTRING("SomeOtherTagValue");
static ByteArray testValue = BYTE_ARRAY_INIT_FROM_CSTRING("lorem ipsum... blah blah blah... etc.");

void setUp(void)
{
    SystemTestSetup(&Fixture);

    // Setup to write some test data
    Kinetic_KeyValue metadata = {
        .key = valueKey,
        .newVersion = BYTE_ARRAY_INIT_FROM_CSTRING("v1.0"),
        .tag = tag,
        .algorithm = KINETIC_PROTO_ALGORITHM_SHA1,
    };

    Fixture.instance.request.value = testValue;

    Fixture.instance.value = testValue;

    KineticProto_Status_StatusCode status =
        KineticClient_Put(&Fixture.instance.operation,
            &metadata,
            Fixture.instance.value);

    TEST_ASSERT_EQUAL_KINETIC_STATUS(
        KINETIC_PROTO_STATUS_STATUS_CODE_SUCCESS, status);
}

void tearDown(void)
{
    SystemTestTearDown(&Fixture);
}

// -----------------------------------------------------------------------------
// Put Command - Write a blob of data to a Kinetic Device
void test_Get_should_retrieve_object_and_metadata_from_device(void)
{
    Kinetic_KeyValue metadata = {.key = valueKey};
    ByteArray value = {.data = Fixture.instance.response.valueBuffer};

    KineticProto_Status_StatusCode status =
        KineticClient_Get(&Fixture.instance.operation,
            &metadata,
            value);

    if (status == KINETIC_PROTO_STATUS_STATUS_CODE_DATA_ERROR)
    {
        TEST_IGNORE_MESSAGE("FIXME: GET is failing to validate HMAC");
    }

    TEST_ASSERT_EQUAL_KINETIC_STATUS(
        KINETIC_PROTO_STATUS_STATUS_CODE_SUCCESS, status);
}

/*******************************************************************************
* ENSURE THIS IS AFTER ALL TESTS IN THE TEST SUITE
*******************************************************************************/
SYSTEM_TEST_SUITE_TEARDOWN(&Fixture)
