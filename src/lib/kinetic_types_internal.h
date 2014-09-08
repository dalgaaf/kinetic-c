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

#ifndef _KINETIC_TYPES_INTERNAL_H
#define _KINETIC_TYPES_INTERNAL_H

#include "kinetic_types_internal.h"
#include "kinetic_proto.h"
#include <time.h>

#define KINETIC_SESSIONS_MAX (6)
#define KINETIC_PDUS_PER_SESSION_DEFAULT (3)
#define KINETIC_PDUS_PER_SESSION_MAX (10)

typedef struct _KineticPDU KineticPDU;

// Kinetic Device Client Connection
typedef struct _KineticConnection
{
    KineticSession* session;

    bool    connected;
    int     socketDescriptor;
    int64_t connectionID;

    // Required field
    // A monotonically increasing number for each request in a TCP connection.
    int64_t sequence;

    KineticPDU* pdus[KINETIC_PDUS_PER_SESSION_MAX];
} KineticConnection;
#define KINETIC_CONNECTION_INIT(_con, _id, _key) { \
    (*_con) = (KineticConnection) { \
        .socketDescriptor = -1, \
        .connectionID = time(NULL), \
        .identity = (_id), \
        .sequence = 0, \
    }; \
    (*_con).key = (ByteArray){.data = (*_con).keyData, .len = (_key).len}; \
    if ((_key).data != NULL && (_key).len > 0) { \
        memcpy((_con)->keyData, (_key).data, (_key).len); } \
}


// Kinetic Message HMAC
typedef struct _KineticHMAC
{
    KineticProto_Security_ACL_HMACAlgorithm algorithm;
    uint32_t len;
    uint8_t data[KINETIC_HMAC_MAX_LEN];
} KineticHMAC;


// Kinetic Device Message Request
typedef struct _KineticMessage
{
    // Kinetic Protocol Buffer Elements
    KineticProto                proto;
    KineticProto_Command        command;
    KineticProto_Header         header;
    KineticProto_Body           body;
    KineticProto_Status         status;
    KineticProto_Security       security;
    KineticProto_Security_ACL   acl;
    KineticProto_KeyValue       keyValue;
    uint8_t                     hmacData[KINETIC_HMAC_MAX_LEN];
} KineticMessage;
#define KINETIC_MESSAGE_HEADER_INIT(_hdr, _con) { \
    assert((_hdr) != NULL); \
    assert((_con) != NULL); \
    *(_hdr) = (KineticProto_Header) { \
        .base = PROTOBUF_C_MESSAGE_INIT(&KineticProto_header__descriptor), \
        .has_clusterVersion = true, \
        .clusterVersion = (_con)->clusterVersion, \
        .has_identity = true, \
        .identity = (_con)->identity, \
        .has_connectionID = true, \
        .connectionID = (_con)->connectionID, \
        .has_sequence = true, \
        .sequence = (_con)->sequence, \
    }; \
}
#define KINETIC_MESSAGE_INIT(msg) { \
    KineticProto__init(&(msg)->proto); \
    KineticProto_command__init(&(msg)->command); \
    KineticProto_header__init(&(msg)->header); \
    KineticProto_status__init(&(msg)->status); \
    KineticProto_body__init(&(msg)->body); \
    KineticProto_key_value__init(&(msg)->keyValue); \
    memset((msg)->hmacData, 0, SHA_DIGEST_LENGTH); \
    (msg)->proto.hmac.data = (msg)->hmacData; \
    (msg)->proto.hmac.len = KINETIC_HMAC_MAX_LEN; \
    (msg)->proto.has_hmac = true; \
    (msg)->command.header = &(msg)->header; \
    (msg)->proto.command = &(msg)->command; \
}


// Kinetic PDU Header
#define PDU_HEADER_LEN              (1 + (2 * sizeof(int32_t)))
#define PDU_PROTO_MAX_LEN           (1024 * 1024)
#define PDU_PROTO_MAX_UNPACKED_LEN  (PDU_PROTO_MAX_LEN * 2)
#define PDU_VALUE_MAX_LEN           (1024 * 1024)
#define PDU_MAX_LEN                 (PDU_HEADER_LEN + \
                                    PDU_PROTO_MAX_LEN + PDU_VALUE_MAX_LEN)
typedef struct __attribute__ ((__packed__)) _KineticPDUHeader
{
    uint8_t     versionPrefix;
    uint32_t    protobufLength;
    uint32_t    valueLength;
} KineticPDUHeader;
#define KINETIC_PDU_HEADER_INIT \
    (KineticPDUHeader) {.versionPrefix = 'F'}


// Kinetic PDU
typedef struct _KineticPDU
{
    // Binary PDU header
    KineticPDUHeader header;    // Header struct in native byte order
    KineticPDUHeader headerNBO; // Header struct in network-byte-order

    // Message associated with this PDU instance
    union {
        // Pre-structured message w/command
        KineticMessage message;
        KineticProto protoBase;

        // Pad protobuf to remaining fields
        uint8_t protoData[PDU_PROTO_MAX_UNPACKED_LEN];
    };        // Proto will always be first
    KineticProto* proto;
    // bool rawProtoEnabled;
    uint8_t protobufRaw[PDU_PROTO_MAX_LEN];

    // Object meta-data to be used/populated if provided and pertinent to the opearation
    Kinetic_KeyValue* metadata;

    // Value data associated with PDU (if any)
    uint8_t valueBuffer[PDU_VALUE_MAX_LEN];
    ByteArray value;

    // Embedded HMAC instance
    KineticHMAC hmac;

    // Exchange associated with this PDU instance (info gets embedded in protobuf message)
    KineticConnection* connection;
};

#define KINETIC_PDU_INIT(_pdu, _con) { \
    assert((_pdu) != NULL); \
    assert((_con) != NULL); \
    (_pdu)->connection = (_con); \
    (_pdu)->header = KINETIC_PDU_HEADER_INIT; \
    (_pdu)->headerNBO = KINETIC_PDU_HEADER_INIT; \
    (_pdu)->value = BYTE_ARRAY_NONE; \
    (_pdu)->proto = &(_pdu)->message.proto; \
    KINETIC_MESSAGE_HEADER_INIT(&((_pdu)->message.header), (_con)); \
}
#define KINETIC_PDU_INIT_WITH_MESSAGE(_pdu, _con) { \
    KINETIC_PDU_INIT((_pdu), (_con)) \
    KINETIC_MESSAGE_INIT(&((_pdu)->message)); \
    (_pdu)->proto->command = &(_pdu)->message.command; \
    (_pdu)->proto->command->header = &(_pdu)->message.header; \
    KINETIC_MESSAGE_HEADER_INIT(&(_pdu)->message.header, (_con)); \
}

// Kinetic Operation
typedef struct _KineticOperation
{
    KineticConnection* connection;
    KineticPDU* request;
    KineticPDU* response;
} KineticOperation;
#define KINETIC_OPERATION_INIT(_op, _con, _req, _resp) \
*(_op) = (KineticOperation) { \
    .connection = (_con), \
    .request = (_req), \
    .response = (_resp), \
}

#endif // _KINETIC_TYPES_INTERNAL_H
