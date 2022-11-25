#pragma once

#ifndef CTL
#define CTL

#include "driver.h"

#define CTL_CODE_ECHO CTL_CODE(FILE_DEVICE_UNKNOWN,0x800,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define CTL_CODE_INIT_CONTEXT CTL_CODE(FILE_DEVICE_UNKNOWN,0x801,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define CTL_CODE_QUERY_KERNEL_MODULE_INFO CTL_CODE(FILE_DEVICE_UNKNOWN,0x802,METHOD_BUFFERED,FILE_ANY_ACCESS)

typedef struct _RESULT {
	NTSTATUS Status;
	SIZE_T SizeOfData;
	PVOID Data;
}RESULT, *PRESULT;

VOID
WriteResult(
	PVOID OutputBuffer,
	ULONG OutputBufferLength,
	PULONG_PTR Information,
	PRESULT Result
);

VOID
HandleDriverLoad(
	PDRIVER_OBJECT DriverObject
);

VOID
HandleDriverUnload(
	PDRIVER_OBJECT DriverObject
);

NTSTATUS
IoctlDispatchRoutine(
	PDEVICE_OBJECT pDevObj,
	PIRP pIrp
);

NTSTATUS
HandleEcho(
	PDEVICE_OBJECT DeviceObject,
	ULONG InputBufferLength,
	ULONG OutputBufferLength,
	PVOID InputBuffer,
	PVOID OutputBuffer,
	PULONG_PTR Information
);

NTSTATUS
HandleInitializationGlobalContext(
	PDEVICE_OBJECT DeviceObject,
	ULONG InputBufferLength,
	ULONG OutputBufferLength,
	PVOID InputBuffer,
	PVOID OutputBuffer,
	PULONG_PTR Information
);

NTSTATUS
HandleQueryNtosKrnlModuleInformation(
	PDEVICE_OBJECT DeviceObject,
	ULONG InputBufferLength,
	ULONG OutputBufferLength,
	PVOID InputBuffer,
	PVOID OutputBuffer,
	PULONG_PTR Information
);

#endif // !CTL

