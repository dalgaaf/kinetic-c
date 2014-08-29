#ifndef _SYSTEM_TEST_FIXTURE
#define _SYSTEM_TEST_FIXTURE

#include "kinetic_types.h"

typedef struct _SystemTestInstance
{
    bool testIgnored;
    int expectedSequence;
    bool nonBlocking;
    KineticOperation operation;
    KineticPDU request;
    KineticPDU response;
    KineticMessage requestMsg;
    ByteArray value;
    uint8_t data[PDU_VALUE_MAX_LEN];
} SystemTestInstance;

typedef struct _SystemTestFixture
{
    KineticConnectionConfig config;
    KineticConnection connection;
    SystemTestInstance instance;
    int64_t expectedSequence;
} SystemTestFixture;

void SystemTestSetup(SystemTestFixture* fixture);
void SystemTestTearDown(SystemTestFixture* fixture);
void SystemTestSuiteTearDown(SystemTestFixture* fixture);

#define SYSTEM_TEST_SUITE_TEARDOWN(_fixture) \
void test_Suite_TearDown(void) \
{ \
    TEST_ASSERT_NOT_NULL_MESSAGE((_fixture), "System test fixture passed to 'SYSTEM_TEST_SUITE_TEARDOWN' is NULL!"); \
    if ((_fixture)->connection.connected) \
        KineticClient_Disconnect(&(_fixture)->connection); \
    (_fixture)->connection.connected = false; \
    (_fixture)->instance.testIgnored = true; \
}

#endif // _SYSTEM_TEST_FIXTURE
