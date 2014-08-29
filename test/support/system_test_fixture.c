#include "system_test_fixture.h"
#include "unity_helper.h"
#include "kinetic_client.h"

uint8_t hmacKeyBuffer[KINETIC_MAX_KEY_LEN];

void SystemTestSetup(SystemTestFixture* fixture)
{
    TEST_ASSERT_NOT_NULL_MESSAGE(fixture, "System test fixture is NULL!");

    fixture->instance = (SystemTestInstance) {
        .value = {
            .data = fixture->instance.data,
            .len = PDU_VALUE_MAX_LEN
        }
    };
    fixture->instance.request.message = &fixture->instance.requestMsg;

    KINETIC_MESSAGE_INIT(fixture->instance.request.message);

    int i;
    for (i = 0; i < fixture->instance.value.len; i++)
    {
        fixture->instance.data[i] = (uint8_t)(0x0ff & i);
    }

    if (!fixture->connection.connected)
    {
        KineticClient_Init(NULL);

        KINETIC_CONNECTION_INIT(&fixture->connection, &fixture->config);

        bool success = KineticClient_Connect(
            &fixture->connection,
            &fixture->connection.config);
        TEST_ASSERT_TRUE(success);
        TEST_ASSERT(fixture->connection.socketDescriptor >= 0);
        fixture->instance.expectedSequence = 0;
        fixture->connection.connected = true;
    }

    fixture->instance.operation =
        KineticClient_CreateOperation(
            &fixture->connection,
            &fixture->instance.request,
            &fixture->instance.requestMsg,
            &fixture->instance.response);

    if (!fixture->instance.testIgnored)
    {
        TEST_ASSERT_EQUAL_MESSAGE(
            fixture->expectedSequence,
            fixture->connection.sequence,
            "Failed validating starting sequence count for the operation w/session!");
    }

    fixture->instance.testIgnored = false;
}

void SystemTestTearDown(SystemTestFixture* fixture)
{
    TEST_ASSERT_NOT_NULL_MESSAGE(fixture, "System test fixture is NULL!");

    if (!fixture->instance.testIgnored)
    {
        fixture->expectedSequence++;
        TEST_ASSERT_EQUAL_MESSAGE(
            fixture->expectedSequence,
            fixture->connection.sequence,
            "Sequence should post-increment for every operation on the session!");
    }
}
