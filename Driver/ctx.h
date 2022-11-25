#pragma once

#ifndef CTX
#define CTX

#include "driver.h"

typedef struct _SYSTEM_MODULE_ENTRY
{
	PVOID ImageBase;
	ULONG ImageSize;
	UCHAR FullPathName[256];
} SYSTEM_MODULE_ENTRY, * PSYSTEM_MODULE_ENTRY;

// Warning: Incomplete structure
typedef struct _KLDR_DATA_TABLE_ENTRY {
	LIST_ENTRY InLoadOrderLinks;
	PVOID ExceptionTable;
	ULONG ExceptionTableSize;
	// ULONG padding on IA64
	PVOID GpValue;
	PVOID NonPagedDebugInfo;
	PVOID DllBase;
	PVOID EntryPoint;
	ULONG SizeOfImage;
	UNICODE_STRING FullDllName;
	UNICODE_STRING BaseDllName;
	ULONG Flags;
	USHORT LoadCount;
	USHORT __Unused5;
	PVOID SectionPointer;
	ULONG CheckSum;
} KLDR_DATA_TABLE_ENTRY, * PKLDR_DATA_TABLE_ENTRY;

typedef struct _GLOBAL_CONTEXT
{
	// Functions
	ULONG_PTR pfnExpBlockOnLockedHandleEntry;
	ULONG_PTR pfnExfUnblockPushLock;

	// Gloabal Variables
	ULONG_PTR PspCidTable;
	ULONG_PTR PsLoadedModuleList;
	ULONG_PTR PsLoadedModuleResource;

	ULONG_PTR ObTypeIndexTable;
	ULONG_PTR ObHeaderCookie;
	ULONG_PTR ObpRootDirectoryObject;

	// class field offset
	ULONG_PTR SizeOfObjectHeader;
	ULONG_PTR OffsetTypeIndexOfObjectHeader;
	ULONG_PTR OffsetTypeNameOfObjectType;
	ULONG_PTR OffsetTypeInfoOfObjectType;

	ULONG_PTR OffsetDumpProcOfObjectTypeInitializer;
	ULONG_PTR OffsetOpenProcOfObjectTypeInitializer;
	ULONG_PTR OffsetCloseProcOfObjectTypeInitializer;
	ULONG_PTR OffsetDeleteProcOfObjectTypeInitializer;
	ULONG_PTR OffsetParseProcOfObjectTypeInitializer;
	ULONG_PTR OffsetParseExProcOfObjectTypeInitializer;

}GLOBAL_CONTEXT, * PGLOBAL_CONTEXT;

NTSTATUS 
InitializationGlobalContext(
	PDRIVER_OBJECT DriverObject, 
	PVOID Buffer, 
	SIZE_T BufferSize, 
	PGLOBAL_CONTEXT Context
);

NTSTATUS 
QueryNtosKrnlModuleInformation(
	PDRIVER_OBJECT DriverObject, 
	PSYSTEM_MODULE_ENTRY ModuleEntry
);

NTSTATUS 
QueryModuleInformation(
	PDRIVER_OBJECT DriverObject, 
	PUNICODE_STRING BaseName, 
	PSYSTEM_MODULE_ENTRY ModuleEntry
);

#endif // !CTX
