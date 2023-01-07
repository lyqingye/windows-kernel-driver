#pragma once

#ifndef DRIVER
#define DRIVER

//#include <ntddk.h>
#include <ntifs.h>
#include "ex.h"
#include "ctl.h"
#include "ctx.h"
#include "ob.h"
#include "ps.h"

//
// Global variable
// 
ULONG_PTR NtosKrnlBase;
EXF_UNBLOCK_PUSH_LOCK ExfUnblockPushLock;
EXP_BLOCK_ON_LOCKED_HANDLE_ENTRY ExpBlockOnLockedHandleEntry;
EX_DESTROY_HANDLE ExDestroyHandle;
PSP_LOCK_PROCESS_LIST_EXCLUSIVE PspLockProcessListExclusive;
PSP_UNLOCK_PROCESS_LIST_EXCLUSIVE PspUnlockProcessListExclusive;

PHANDLE_TABLE *PspCidTable;
ULONG_PTR ObTypeIndexTable;
ULONG_PTR ObHeaderCookie;
ULONG_PTR ObpRootDirectoryObject;
PLIST_ENTRY *PsLoadedModuleList;
PERESOURCE *PsLoadedModuleResource;
PLIST_ENTRY *PsActiveProcessHead;

ULONG_PTR SizeOfObjectHeader;
ULONG_PTR OffsetTypeIndexOfObjectHeader;
ULONG_PTR OffsetTypeNameOfObjectType;
ULONG_PTR OffsetTypeInfoOfObjectType;
ULONG_PTR OffsetDirectoryTableBaseOfEProcess;

#define DEVICE_NAME  L"\\Device\\WindowsKernelResearch"
#define SYMBOL_LINK_NAME L"\\??\\WindowsKernelResearch"

#define PTR_ADD_OFFSET(Pointer, Offset) ((PVOID)((ULONG_PTR)(Pointer) + (ULONG_PTR)(Offset)))
#define PTR_SUB_OFFSET(Pointer, Offset) ((PVOID)((ULONG_PTR)(Pointer) - (ULONG_PTR)(Offset)))

NTSTATUS 
DefDispatchRoutine(
	PDEVICE_OBJECT pDevObj,
	PIRP pIrp
);

NTSTATUS 
DriverUnload(
	PDRIVER_OBJECT DriverObject
);

NTSTATUS 
DriverEntry(
	PDRIVER_OBJECT DriverObject, 
	PUNICODE_STRING LoadPath
);

#endif // !DRIVER



