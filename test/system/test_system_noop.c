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

void setUp(void)
{
    SystemTestSetup(&Fixture);
}

void tearDown(void)
{
    SystemTestTearDown(&Fixture);
}

// -----------------------------------------------------------------------------
// NoOp Command - Basic 'ping' to check if a given Kinetic Device is online
//
// Inspected Request: (8/5/21014)
// -----------------------------------------------------------------------------
//
// Received 48 bytes
// Receiving a PDU...
//   request[48]:
//      4600000027000000000A0F0A0D10E807
//      18EFA7859F052000381E1A141B507D08
//      4524F1C0902CC962D9FFB911F58787FC
// PDU Header:
//   versionPrefix: F
//   protobufLength: 39
//   valueLength: 0
// PDU Protobuf:
//   --- !ruby/object:Seagate::Kinetic::Message
//   command: !ruby/object:Seagate::Kinetic::Message::Command
//     header: !ruby/object:Seagate::Kinetic::Message::Header
//       clusterVersion: <????????????????>
//       identity: 1000 <????????????????>
//       connectionID: 1407276015
//       sequence: 0 <????????????????>
//       ackSequence:
//       messageType: 30
//       timeout:
//       earlyExit:
//       backgroundScan:
//     body:
//     status:
//   hmac: !binary |-
//     G1B9CEUk8cCQLMli2f+5EfWHh/w=
// Received PDU successfully!
//
// -----------------------------------------------------------------------------
// Jim: You can use the python code and turn client.debug on.
//      You will get this from a noop.
//      NOTE: The outer header is assumed correct.
// -----------------------------------------------------------------------------
// Request from Kinetic-Python
// -----------------------------------------------------------------------------
// command {
//   header {
//     clusterVersion: 0
//     identity: 1
//     connectionID: 1406836231
//     sequence: 1
//     messageType: NOOP
//   }
// }
// hmac: "g\325\226\266\227\017\300\270\266\233%\3569\027\037\336\026\322D\214"
// -----------------------------------------------------------------------------
// Response from the drive
// -----------------------------------------------------------------------------
// command {
//   header {
//     connectionID: 7
//     ackSequence: 1
//     messageType: NOOP_RESPONSE
//   }
//   status {
//     code: SUCCESS
//   }
// }
// hmac: "\332\254\262@8\334\366G\344\323\227\323\366m^A<q\353|"
//
// -----------------------------------------------------------------------------
// Response from the simulator.
// -----------------------------------------------------------------------------
// command {
//   header {
//     identity: 1
//     ackSequence: 1
//     messageType: NOOP_RESPONSE
//   }
//   status {
//     code: SUCCESS
//   }
// }
// hmac: "\367qZ\025\266\301\221FM*0\341\303C\361M\006)#\004"
// -----------------------------------------------------------------------------
void test_NoOp_should_succeed(void)
{
    KineticProto_Status_StatusCode status =
        KineticClient_NoOp(&Fixture.instance.operation);

    TEST_ASSERT_EQUAL_KINETIC_STATUS(
        KINETIC_PROTO_STATUS_STATUS_CODE_SUCCESS, status);
}

/*******************************************************************************
* ENSURE THIS IS AFTER ALL TESTS IN THE TEST SUITE
*******************************************************************************/
SYSTEM_TEST_SUITE_TEARDOWN(&Fixture)
