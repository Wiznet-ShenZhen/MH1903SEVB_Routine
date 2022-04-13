#ifndef __SNMP_H
#define __SNMP_H

#define SOCK_SNMP					0

#define SUCCESS						0
#define OID_NOT_FOUND				-1
#define TABLE_FULL					-2
#define ILLEGAL_LENGTH				-3
#define INVALID_ENTRY_ID			-4
#define INVALID_DATA_TYPE			-5

#define NO_SUCH_NAME				2
#define BAD_VALUE					3

#define MAX_OID						12
#define MAX_STRING					40

#define SNMP_V1						0

#define COMMUNITY					"public\0"
#define COMMUNITY_SIZE				(strlen(COMMUNITY))

#define GET_REQUEST					0xa0
#define GET_NEXT_REQUEST			0xa1
#define GET_RESPONSE				0xa2
#define SET_REQUEST					0xa3

#define VALID_REQUEST(x)			((x == GET_REQUEST) || (x == GET_NEXT_REQUEST) || (x == SET_REQUEST))

#define SNMPDTYPE_INTEGER			0x02
#define SNMPDTYPE_OCTET_STRING		0x04
#define SNMPDTYPE_NULL_ITEM			0x05
#define SNMPDTYPE_OBJ_ID			0x06
#define SNMPDTYPE_SEQUENCE			0x30
#define SNMPDTYPE_SEQUENCE_OF		SNMPDTYPE_SEQUENCE

#define SNMPDTYPE_COUNTER			0x41
#define SNMPDTYPE_GAUGE  			0x42
#define SNMPDTYPE_TIME_TICKS		0x43
#define SNMPDTYPE_OPAQUE			0x44

#define UNUSED(x)					(void)x; // for IAR warning (declared but never referenced)
#define HTONL(x)					((((x)>>24) & 0xffL) | (((x)>>8) & 0xff00L) | (((x)<<8) & 0xff0000L) | (((x)<<24) & 0xff000000L))

typedef struct {
	uint8_t oidlen;
	uint8_t oid[MAX_OID];
	uint8_t dataType;
	uint8_t dataLen;
	union {
		uint8_t octetstring[MAX_STRING];
		uint32_t intval;
	} u;
	void (*getfunction)(void*, uint8_t*);
	void (*setfunction)(int32_t);
} dataEntryType;

struct messageStruct {
	uint8_t buffer[1025];
	int32_t len;
	int32_t index;
};

typedef struct {
	int32_t start;	/* Absolute Index of the TLV */
	int32_t len;		/* The L value of the TLV */
	int32_t vstart;   /* Absolute Index of this TLV's Value */
	int32_t nstart;   /* Absolute Index of the next TLV */
} tlvStructType;

void WDEBUG(char* fmt, ...);
int32_t findEntry(uint8_t* oid, int32_t len);
int32_t getOID(int32_t id, uint8_t* oid, uint8_t* len);
int32_t getValue(uint8_t* vptr, int32_t vlen);
int32_t getEntry(int32_t id, uint8_t* dataType, void* ptr, int32_t* len);
int32_t setEntry(int32_t id, void* val, int32_t vlen, uint8_t dataType, int32_t index);

int32_t parseLength(const uint8_t* msg, int32_t* len);
int32_t parseTLV(const uint8_t* msg, int32_t index, tlvStructType* tlv);
void insertRespLen(int32_t reqStart, int32_t respStart, int32_t size);
int32_t parseVarBind(int32_t reqType, int32_t index);
int32_t parseSequence(int32_t reqType, int32_t index);
int32_t parseSequenceOf(int32_t reqType);
int32_t parseRequest(void);
int32_t parseCommunity(void);
int32_t parseVersion(void);
int32_t parseSNMPMessage(void);

int32_t makeTrapVariableBindings(dataEntryType* oid_data, void* ptr, uint32_t* len);
int32_t SnmpXInit(void);
int32_t SnmpXTrapSend(int8_t* managerIP, int8_t* agentIP, int8_t* community, dataEntryType enterprise_oid, uint32_t genericTrap, uint32_t specificTrap, uint32_t va_count, ...);
int32_t SnmpXDaemon(void);

#endif	
