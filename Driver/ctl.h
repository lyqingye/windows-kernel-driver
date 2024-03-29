#pragma once

#ifndef CTL
#define CTL

#include "driver.h"

#define CTL_CODE_ECHO CTL_CODE(FILE_DEVICE_UNKNOWN,0x800,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define CTL_CODE_INIT_CONTEXT CTL_CODE(FILE_DEVICE_UNKNOWN,0x801,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define CTL_CODE_QUERY_KERNEL_MODULE_INFO CTL_CODE(FILE_DEVICE_UNKNOWN,0x802,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define CTL_CODE_READ_PROCESS_MEMORY CTL_CODE(FILE_DEVICE_UNKNOWN,0x803,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define CTL_CODE_WRITE_PROCESS_MEMORY CTL_CODE(FILE_DEVICE_UNKNOWN,0x804,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define CTL_CODE_READ_VIRTUAL_MEMORY CTL_CODE(FILE_DEVICE_UNKNOWN,0x805,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define CTL_CODE_WRITE_VIRTUAL_MEMORY CTL_CODE(FILE_DEVICE_UNKNOWN,0x806,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define CTL_CODE_READ_PHYSICAL_MEMORY CTL_CODE(FILE_DEVICE_UNKNOWN,0x807,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define CTL_CODE_WRITE_PHYSICAL_MEMORY CTL_CODE(FILE_DEVICE_UNKNOWN,0x808,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define CTL_CODE_ALLOC_PHYSICAL_MEMORY CTL_CODE(FILE_DEVICE_UNKNOWN,0x809,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define CTL_CODE_FREE_PHYSICAL_MEMORY CTL_CODE(FILE_DEVICE_UNKNOWN,0x810,METHOD_BUFFERED,FILE_ANY_ACCESS)

typedef struct _RESULT {
	NTSTATUS Status;
	SIZE_T SizeOfData;
}RESULT, *PRESULT;

FORCEINLINE
NTSTATUS
CreateResult(
	PVOID Buffer,
	ULONG BufferLength,
	PULONG_PTR Information,
	PRESULT* Result
);

FORCEINLINE
VOID
WriteResult(
	ULONG OutputBufferLength,
	PULONG_PTR Information,
	PRESULT Result,
	PVOID Data,
	SIZE_T SizeOfData
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

NTSTATUS
HandleReadProcessMemory(
	PDEVICE_OBJECT DeviceObject,
	ULONG InputBufferLength,
	ULONG OutputBufferLength,
	PVOID InputBuffer,
	PVOID OutputBuffer,
	PULONG_PTR Information
);

NTSTATUS
HandleWriteProcessMemory(
	PDEVICE_OBJECT DeviceObject,
	ULONG InputBufferLength,
	ULONG OutputBufferLength,
	PVOID InputBuffer,
	PVOID OutputBuffer,
	PULONG_PTR Information
);

NTSTATUS
HandleAllocatePhysicalMemory(
	PDEVICE_OBJECT DeviceObject,
	ULONG InputBufferLength,
	ULONG OutputBufferLength,
	PVOID InputBuffer,
	PVOID OutputBuffer,
	PULONG_PTR Information
);

NTSTATUS
HandleFreePhysicalMemory(
	PDEVICE_OBJECT DeviceObject,
	ULONG InputBufferLength,
	ULONG OutputBufferLength,
	PVOID InputBuffer,
	PVOID OutputBuffer,
	PULONG_PTR Information
);

NTSTATUS
HandleReaPhysicalMemory(
	PDEVICE_OBJECT DeviceObject,
	ULONG InputBufferLength,
	ULONG OutputBufferLength,
	PVOID InputBuffer,
	PVOID OutputBuffer,
	PULONG_PTR Information
);

NTSTATUS
HandleWritePhysicalMemory(
	PDEVICE_OBJECT DeviceObject,
	ULONG InputBufferLength,
	ULONG OutputBufferLength,
	PVOID InputBuffer,
	PVOID OutputBuffer,
	PULONG_PTR Information
);

#endif // !CTL

