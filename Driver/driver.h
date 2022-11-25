#pragma once

#ifndef DRIVER
#define DRIVER

#include <ntddk.h>
#include "ex.h"
#include "ctl.h"
#include "ctx.h"

#define DEVICE_NAME  L"\\Device\\WindowsKernelResearch"
#define SYMBOL_LINK_NAME L"\\??\\WindowsKernelResearch"

#define PTR_ADD_OFFSET(Pointer, Offset) ((PVOID)((ULONG_PTR)(Pointer) + (ULONG_PTR)(Offset)))
#define PTR_SUB_OFFSET(Pointer, Offset) ((PVOID)((ULONG_PTR)(Pointer) - (ULONG_PTR)(Offset)))

//
// Global variable
// 
ULONG_PTR NtosKrnlBase;
EXF_UNBLOCK_PUSH_LOCK ExfUnblockPushLock;
EXP_BLOCK_ON_LOCKED_HANDLE_ENTRY ExpBlockOnLockedHandleEntry;

NTSTATUS
DispatchCreate(
	PDEVICE_OBJECT pDevObj, 
	PIRP pIrp
);

NTSTATUS
DispatchClose(
	PDEVICE_OBJECT pDevObj,
	PIRP pIrp
);

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



