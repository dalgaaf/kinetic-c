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

#ifndef _KINETIC_TYPES_H
#define _KINETIC_TYPES_H

// Include C99 bool definition, if not already defined
#if !defined(__bool_true_false_are_defined) || (__bool_true_false_are_defined == 0)
#include <stdbool.h>
#endif
#include <stdint.h>
#include <inttypes.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <limits.h>
#include <netinet/in.h>
#include <ifaddrs.h>

#define KINETIC_PORT            8123
#define KINETIC_TLS_PORT        8443
#define KINETIC_HMAC_SHA1_LEN   (SHA_DIGEST_LENGTH)
#define KINETIC_HMAC_MAX_LEN    (KINETIC_HMAC_SHA1_LEN)
#define KINETIC_MAX_KEY_LEN     128

// Ensure __func__ is defined (for debugging)
#if !defined __func__
    #define __func__ __FUNCTION__
#endif

// Define max host name length
// Some Linux environments require this, although not all, but it's benign.
#ifndef _BSD_SOURCE
    #define _BSD_SOURCE
#endif // _BSD_SOURCE
#include <unistd.h>
#include <sys/types.h>
#ifndef HOST_NAME_MAX
    #define HOST_NAME_MAX 256
#endif // HOST_NAME_MAX

typedef ProtobufCBinaryData ByteArray;
#define BYTE_ARRAY_NONE \
    (ByteArray){}
#define BYTE_ARRAY_INIT(_data) (ByteArray) \
    {.data = (uint8_t*)(_data), .len = sizeof(_data)};
#define BYTE_ARRAY_INIT_WITH_LEN(_data, _len) \
    (ByteArray){.data = (uint8_t*)(_data), .len = (_len)};
#define BYTE_ARRAY_CREATE(name, len) \
    uint8_t ( name ## _buf )[(len)]; ByteArray (name) = BYTE_ARRAY_INIT(( name ## _buf ));
#define BYTE_ARRAY_CREATE_WITH_DATA(_name, _data) \
    uint8_t ( _name ## _data )[sizeof(_data)]; ByteArray (_name) = {.data = (uint8_t*(_data)), .len = sizeof(data)};
#define BYTE_ARRAY_CREATE_WITH_BUFFER(_name, _buf) \
    ByteArray (_name) = {.data = (uint8_t*(_buf)), .len = 0};
#define BYTE_ARRAY_INIT_FROM_CSTRING(str) \
    (ByteArray){.data = (uint8_t*)(str), .len = strlen(str)}
#define BYTE_ARRAY_FILL_WITH_DUMMY_DATA(_array) \
    {int i=0; for(;i<(_array).len;++i){(_array).data[i] = (uint8_t)(i & 0xFFu);} }

typedef struct
{
    ByteArray   buffer;
    size_t      maxLen;
} ByteBuffer;
#define BYTE_BUFFER_INIT(_buf, _max) (ByteBuffer) { \
    .buffer = {.data = (_buf), .len = 0}, \
    .maxLen = sizeof(_buf) }

#define LOG_FILE_NAME_MAX (256)

// Kinetic session
typedef struct _KineticSession
{
    int     handle;
    bool    nonBlocking;
    int     port;
    char    host[HOST_NAME_MAX];
    char    logFile[LOG_FILE_NAME_MAX],

    // The version number of this cluster definition. If this is not equal to
    // the value on the device, the request is rejected and will return a
    // `VERSION_FAILURE` `statusCode` in the `Status` message.
    int64_t clusterVersion;

    // The identity associated with this request. See the ACL discussion above.
    // The Kinetic Device will use this identity value to lookup the
    // HMAC key (shared secret) to verify the HMAC.
    int64_t identity;

    // This is the identity's HMAC Key. This is a shared secret between the
    // client and the device, used to sign requests.
    uint8_t keyData[KINETIC_MAX_KEY_LEN];
    ByteArray key;
} KineticSession;

typedef enum _Kinetic_Status {
  KINETIC_STATUS_INVALID = -1,
  KINETIC_STATUS_NOT_ATTEMPTED = 0,
  KINETIC_STATUS_SUCCESS = 1,
  KINETIC_STATUS_HMAC_FAILURE = 2,
  KINETIC_STATUS_NOT_AUTHORIZED = 3,
  KINETIC_STATUS_VERSION_FAILURE = 4,
  KINETIC_STATUS_INTERNAL_ERROR = 5,
  KINETIC_STATUS_HEADER_REQUIRED = 6,
  KINETIC_STATUS_NOT_FOUND = 7,
  KINETIC_STATUS_VERSION_MISMATCH = 8,
  KINETIC_STATUS_SERVICE_BUSY = 9,
  KINETIC_STATUS_EXPIRED = 10,
  KINETIC_STATUS_DATA_ERROR = 11,
  KINETIC_STATUS_PERM_DATA_ERROR = 12,
  KINETIC_STATUS_REMOTE_CONNECTION_ERROR = 13,
  KINETIC_STATUS_NO_SPACE = 14,
  KINETIC_STATUS_NO_SUCH_HMAC_ALGORITHM = 15,
  KINETIC_STATUS_INVALID_REQUEST = 16,
  KINETIC_STATUS_NESTED_OPERATION_ERRORS = 17
} Kinetic_Status;

typedef enum _Kinetic_Algorithm {
    KINETIC_ALGORITHM_INVALID = -1,
    KINETIC_ALGORITHM_SHA1 = 1,
    KINETIC_ALGORITHM_SHA2 = 2,
    KINETIC_ALGORITHM_SHA3 = 3,
    KINETIC_ALGORITHM_CRC32 = 4,
    KINETIC_ALGORITHM_CRC64 = 5
} Kinetic_Algorithm;

typedef enum _Kinetic_Synchronization {
  KINETIC_SYNCHRONIZATION_INVALID = -1,
  KINETIC_SYNCHRONIZATION_WRITETHROUGH = 1,
  KINETIC_SYNCHRONIZATION_WRITEBACK = 2,
  KINETIC_SYNCHRONIZATION_FLUSH = 3
} Kinetic_Synchronization;

// KeyValue meta-data
typedef struct _Kinetic_KeyValue
{
    ByteArray key;
    ByteArray newVersion;
    ByteArray dbVersion;
    ByteArray tag;
    bool force;
    Kinetic_Algorithm algorithm;
    bool metadataOnly;
    Kinetic_Synchronization synchronization;
    ByteArray value;
} Kinetic_KeyValue;

#endif // _KINETIC_TYPES_H
