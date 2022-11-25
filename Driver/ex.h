#pragma once

#ifndef EX
#define EX

#include "driver.h"

#pragma warning(disable : 4201)
typedef ULONG ACCESS_MASK;

#define HANDLE_VALUE_INC 4 

#define LEVEL_CODE_MASK 3

#define EXHANDLE_TABLE_ENTRY_LOCK_BIT    1

#define LOWLEVEL_COUNT (PAGE_SIZE / sizeof(HANDLE_TABLE_ENTRY))

#define MIDLEVEL_COUNT (PAGE_SIZE / sizeof(PHANDLE_TABLE_ENTRY))

#define EX_ADDITIONAL_INFO_SIGNATURE (-2)

#define ExpIsValidObjectEntry(Entry) \
    ( (Entry != NULL) && (Entry->VolatileLowValue != NULL) && (Entry->NextFreeHandleEntry != EX_ADDITIONAL_INFO_SIGNATURE) )

typedef struct _EXHANDLE
{
	union {
		struct {
			UCHAR TagBits : 2;
			ULONG Index : 30;
		};
		PVOID GenericHandleOverlay;
		ULONGLONG Value;
	};				// 0x0
} EXHANDLE, * PEXHANDLE;
static_assert(sizeof(struct _EXHANDLE) == 8, "sizeof(_EXHANDLE) == 8");

typedef struct _HANDLE_TABLE_ENTRY_INFO {
	ACCESS_MASK AuditMask;   	// 0x0
	ACCESS_MASK MaxRelativeAccessMask; // 0x4
} HANDLE_TABLE_ENTRY_INFO, * PHANDLE_TABLE_ENTRY_INFO;
static_assert(sizeof(struct _HANDLE_TABLE_ENTRY_INFO) == 8, "sizeof(_HANDLE_TABLE_ENTRY_INFO) == 8");

typedef struct _HANDLE_TABLE_ENTRY
{
	union {
		LONGLONG RefCountField;
		union {
			UCHAR Unlocked : 1;
			USHORT RefCnt : 16;
			UCHAR Attributes : 3;
			LONGLONG ObjectPointerBits : 44;
		};
		PVOID VolatileLowValue;
		LONGLONG LowValue;
		PHANDLE_TABLE_ENTRY_INFO InfoTable;
	};				// 0x0
	// 0x8
	union {
		LONGLONG HighValue;
		LONGLONG NextFreeHandleEntry; // PHANDLE_TABLE_ENTRY_INFO
		EXHANDLE LeafHandleValue;
		struct {
			// 0x8
			union {
				ACCESS_MASK GrantedAccessBits : 25;
				UCHAR NoRightsUpgrade : 1;
				UCHAR Spare1 : 6;
			};
			ULONG Spare2; // 0xc
		};
	};
} HANDLE_TABLE_ENTRY, * PHANDLE_TABLE_ENTRY;
static_assert(sizeof(struct _HANDLE_TABLE_ENTRY) == 0x10, "sizeof(_HANDLE_TABLE_ENTRY) == 0x10");

typedef struct _HANDLE_TABLE_FREE_LIST
{
	EX_PUSH_LOCK FreeListLock;				// 0x0
	PHANDLE_TABLE_ENTRY FirstFreeHandleEntry;				// 0x8
	PHANDLE_TABLE_ENTRY LastFreeHandleEntry;				// 0x10
	LONG HandleCount;				// 0x18
	ULONG HighWaterMark;				// 0x1c
}  HANDLE_TABLE_FREE_LIST, * PHANDLE_TABLE_FREE_LIST;
static_assert(sizeof(struct _HANDLE_TABLE_FREE_LIST) == 0x20, "sizeof(_HANDLE_TABLE_FREE_LIST) == 0x20");

typedef struct _HANDLE_TRACE_DB_ENTRY
{
	CLIENT_ID ClientId;				// 0x0
	PVOID Handle;				// 0x10
	ULONG Type;				// 0x18
	PVOID StackTrace[16];				// 0x20
} HANDLE_TRACE_DB_ENTRY, * PHANDLE_TRACE_DB_ENTRY;
static_assert(sizeof(struct _HANDLE_TRACE_DB_ENTRY) == 0xa0, "sizeof(_HANDLE_TRACE_DB_ENTRY) == 0xa0");

typedef struct _HANDLE_TRACE_DEBUG_INFO
{
	LONG RefCount;				// 0x0
	ULONG TableSize;				// 0x4
	ULONG BitMaskFlags;				// 0x8
	FAST_MUTEX CloseCompactionLock;				// 0x10
	ULONG CurrentStackIndex;				// 0x48
	HANDLE_TRACE_DB_ENTRY TraceDb[1];				// 0x50
} HANDLE_TRACE_DEBUG_INFO, * PHANDLE_TRACE_DEBUG_INFO;
static_assert(sizeof(struct _HANDLE_TRACE_DEBUG_INFO) == 0xf0, "sizeof(_HANDLE_TRACE_DEBUG_INFO) == 0xf0");

typedef struct _HANDLE_TABLE
{
	ULONG NextHandleNeedingPool;				// 0x0
	LONG ExtraInfoPages;				// 0x4
	ULONGLONG TableCode;				// 0x8
	PEPROCESS QuotaProcess;				// 0x10
	LIST_ENTRY HandleTableList;				// 0x18
	ULONG UniqueProcessId;				// 0x28
	union {
		ULONG Flags;
		union {
			UCHAR StrictFIFO : 1;
			UCHAR EnableHandleExceptions : 1;
			UCHAR Rundown : 1;
			UCHAR Duplicated : 1;
			UCHAR RaiseUMExceptionOnInvalidHandleClose : 1;
		};
	};				// 0x2c
	EX_PUSH_LOCK HandleContentionEvent;				// 0x30
	EX_PUSH_LOCK HandleTableLock;				// 0x38
	union {
		HANDLE_TABLE_FREE_LIST FreeLists[1];
		UCHAR ActualEntry[32];
	};				// 0x40
	PHANDLE_TRACE_DEBUG_INFO DebugInfo;				// 0x60
} HANDLE_TABLE, * PHANDLE_TABLE;
static_assert(sizeof(struct _HANDLE_TABLE) == 0x68, "sizeof(_HANDLE_TABLE) == 0x80");

typedef VOID(*EXP_BLOCK_ON_LOCKED_HANDLE_ENTRY)(
	IN PEX_PUSH_LOCK EntryLock,
	IN PHANDLE_TABLE_ENTRY HandleTableEntry,
	IN HANDLE Handle
	);

typedef VOID(*EXF_UNBLOCK_PUSH_LOCK)(
	IN PEX_PUSH_LOCK EntryLock,
	IN ULONGLONG Unknown1 // default 0
	);

typedef BOOLEAN(*EX_ENUMERATE_HANDLE_ROUTINE)(
	IN PHANDLE_TABLE_ENTRY HandleTableEntry,
	IN HANDLE Handle,
	IN PVOID EnumParameter
	);

PHANDLE_TABLE_ENTRY
ExpLookupHandleTableEntry(
	IN PHANDLE_TABLE HandleTable,
	IN EXHANDLE tHandle
);

BOOLEAN
FORCEINLINE
ExpLockHandleTableEntry(
	PHANDLE_TABLE HandleTable,
	PHANDLE_TABLE_ENTRY HandleTableEntry
);

VOID
FORCEINLINE
ExUnlockHandleTableEntry(
	__inout PHANDLE_TABLE HandleTable,
	__inout PHANDLE_TABLE_ENTRY HandleTableEntry
);

VOID
ExEnumHandleTable(
	__in PHANDLE_TABLE HandleTable,
	__in EX_ENUMERATE_HANDLE_ROUTINE EnumHandleProcedure,
	__in PVOID EnumParameter
);
EXF_UNBLOCK_PUSH_LOCK ExfUnBlockPushLock;



#endif EX
