#include "driver.h"

NTSTATUS 
CreateResult(
    PVOID Buffer, 
    ULONG BufferLength,
    PULONG_PTR Information,
    PRESULT *Result
)
{
    *Information = sizeof(RESULT);
    if (BufferLength < sizeof(RESULT)) {
        return STATUS_BUFFER_TOO_SMALL;
    }
    RtlZeroMemory(Buffer, sizeof(RESULT));
    *Result = (PRESULT)Buffer;
    (*Result)->Status = STATUS_SUCCESS;
    (*Result)->SizeOfData = 0;
    return STATUS_SUCCESS;
}

VOID
WriteResult(
    ULONG OutputBufferLength,
    PULONG_PTR Information,
    PRESULT Result,
    PVOID Data,
    SIZE_T SizeOfData
) {
    if (Information && Result) {
        SIZE_T SizeOfMeta = sizeof(RESULT);
        SIZE_T SizeOfResult = SizeOfMeta + SizeOfData;
        // acture output length
        (*Information) = (ULONG_PTR)(SizeOfResult);

        PVOID OutputBuffer = (PVOID)((ULONG_PTR)Result + SizeOfMeta);
        if (Data && SizeOfData) {
            if (OutputBufferLength >= SizeOfResult) {
				RtlZeroMemory(OutputBuffer, SizeOfData);
				RtlCopyMemory(OutputBuffer, Data,SizeOfData);
				Result->SizeOfData = SizeOfData;
            }
            else {
				Result->Status = STATUS_BUFFER_TOO_SMALL;
            }
        }
    }
}

VOID 
HandleDriverLoad(
    PDRIVER_OBJECT 
    DriverObject
) {
    UNREFERENCED_PARAMETER(DriverObject);
}

VOID 
HandleDriverUnload(
    PDRIVER_OBJECT DriverObject
) {
    UNREFERENCED_PARAMETER(DriverObject);
}

NTSTATUS 
IoctlDispatchRoutine(
    PDEVICE_OBJECT pDevObj,
    PIRP pIrp
) {
    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    PIO_STACK_LOCATION IoStackLocation = IoGetCurrentIrpStackLocation(pIrp);
    PVOID Input = pIrp->AssociatedIrp.SystemBuffer;
    PVOID Output = pIrp->AssociatedIrp.SystemBuffer;
    ULONG Code = IoStackLocation->Parameters.DeviceIoControl.IoControlCode;
    ULONG InputBufferLength = IoStackLocation->Parameters.DeviceIoControl.InputBufferLength;
    ULONG OutputBufferLength = IoStackLocation->Parameters.DeviceIoControl.OutputBufferLength;
    ULONG_PTR Information = 0;

    PAGED_CODE();
    DbgPrint("Call Driver Start Code: %x\n", Code);
    DbgPrint("-> Input: %p\n", Input);
    DbgPrint("-> Output: %p\n", Output);
    DbgPrint("-> InputBufferLength: %d\n", InputBufferLength);
    DbgPrint("-> OutputBufferLength: %d\n", OutputBufferLength);

    switch (Code)
    {
    case CTL_CODE_ECHO:
        Status = HandleEcho(
            pDevObj,
            InputBufferLength,
            OutputBufferLength,
            Input,
            Output,
            &Information
        );
        break;
    case CTL_CODE_INIT_CONTEXT:
        Status = HandleInitializationGlobalContext(
            pDevObj,
            InputBufferLength,
            OutputBufferLength,
            Input,
            Output,
            &Information
        );
        break;
    case CTL_CODE_QUERY_KERNEL_MODULE_INFO:
        Status = HandleQueryNtosKrnlModuleInformation(
            pDevObj,
            InputBufferLength,
            OutputBufferLength,
            Input,
            Output,
            &Information
        );
        break;
    case CTL_CODE_READ_PROCESS_MEMORY:
        Status = HandleReadProcessMemory(
            pDevObj,
            InputBufferLength,
            OutputBufferLength,
            Input,
            Output,
            &Information
        );
        break;
    case CTL_CODE_WRITE_PROCESS_MEMORY:
        Status = HandleWriteProcessMemory(
            pDevObj,
            InputBufferLength,
            OutputBufferLength,
            Input,
            Output,
            &Information
        );
        break;
    case CTL_CODE_ALLOC_PHYSICAL_MEMORY:
        Status = HandleAllocatePhysicalMemory(
            pDevObj,
            InputBufferLength,
            OutputBufferLength,
            Input,
            Output,
            &Information
        );
        break;
    case CTL_CODE_FREE_PHYSICAL_MEMORY:
        Status = HandleFreePhysicalMemory(
            pDevObj,
            InputBufferLength,
            OutputBufferLength,
            Input,
            Output,
            &Information
        );
        break;
    case CTL_CODE_READ_PHYSICAL_MEMORY:
        Status = HandleReaPhysicalMemory(
            pDevObj,
            InputBufferLength,
            OutputBufferLength,
            Input,
            Output,
            &Information
        );
        break;
    case CTL_CODE_WRITE_PHYSICAL_MEMORY:
        Status = HandleWritePhysicalMemory(
            pDevObj,
            InputBufferLength,
            OutputBufferLength,
            Input,
            Output,
            &Information
        );
        break;
    default:
        break;
    }
    pIrp->IoStatus.Status = Status;
    pIrp->IoStatus.Information = Information;
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    DbgPrint("Call Driver End Information: %lld\n", Information);

    return Status;
}

NTSTATUS
HandleEcho(
    PDEVICE_OBJECT DeviceObject,
    ULONG InputBufferLength,
    ULONG OutputBufferLength,
    PVOID InputBuffer,
    PVOID OutputBuffer,
    PULONG_PTR Information
) {
    UNREFERENCED_PARAMETER(DeviceObject);

	PRESULT Result;
    NTSTATUS Status = CreateResult(OutputBuffer, OutputBufferLength,Information, &Result);
	if (!NT_SUCCESS(Status)) {
		return Status;
	}
	WriteResult(OutputBufferLength, Information, Result, InputBuffer, InputBufferLength);
    return STATUS_SUCCESS;
}

NTSTATUS
HandleInitializationGlobalContext(
    PDEVICE_OBJECT DeviceObject,
    ULONG InputBufferLength,
    ULONG OutputBufferLength,
    PVOID InputBuffer,
    PVOID OutputBuffer,
    PULONG_PTR Information
) {
    PAGED_CODE();

    GLOBAL_CONTEXT GlobalContext;

	PRESULT Result;
    NTSTATUS Status = CreateResult(OutputBuffer, OutputBufferLength, Information, &Result);
	if (!NT_SUCCESS(Status)) {
		return Status;
	}

    Result->Status = InitializationGlobalContext(
        DeviceObject->DriverObject,
        InputBuffer,
        InputBufferLength,
        &GlobalContext);

    if (!InitSystemPageTableInformation()) {
        Result->Status = STATUS_UNSUCCESSFUL;
    }

	WriteResult(OutputBufferLength, Information, Result, NULL, 0);

  	return STATUS_SUCCESS;
}

NTSTATUS
HandleQueryNtosKrnlModuleInformation(
    PDEVICE_OBJECT DeviceObject,
    ULONG InputBufferLength,
    ULONG OutputBufferLength,
    PVOID InputBuffer,
    PVOID OutputBuffer,
    PULONG_PTR Information
) {
    UNREFERENCED_PARAMETER(InputBufferLength);
    UNREFERENCED_PARAMETER(InputBuffer);

    SYSTEM_MODULE_ENTRY Entry;
	PRESULT Result;
    NTSTATUS Status = CreateResult(OutputBuffer, OutputBufferLength,Information, &Result);
	if (!NT_SUCCESS(Status)) {
		return Status;
	}
    Result->Status = QueryNtosKrnlModuleInformation(DeviceObject->DriverObject, &Entry);
    if (NT_SUCCESS(Result->Status)) {
        WriteResult(OutputBufferLength, Information, Result, (PVOID)&Entry, sizeof(SYSTEM_MODULE_ENTRY));
    }
    return STATUS_SUCCESS;
}

NTSTATUS
HandleReadProcessMemory(
    PDEVICE_OBJECT DeviceObject,
    ULONG InputBufferLength,
    ULONG OutputBufferLength,
    PVOID InputBuffer,
    PVOID OutputBuffer,
    PULONG_PTR Information
)
{
    UNREFERENCED_PARAMETER(DeviceObject);

    typedef struct _PARAM {
        HANDLE ProcessId;
        PVOID Address;
        SIZE_T NumOfBytes;
    }PARAM;

    if (InputBufferLength < sizeof(PARAM)) {
        return STATUS_BUFFER_TOO_SMALL;
    }
    // Copy Param
    PARAM Param = *(PARAM*)InputBuffer;

	PRESULT Result;
    NTSTATUS Status = CreateResult(OutputBuffer, OutputBufferLength,Information, &Result);
	if (!NT_SUCCESS(Status)) {
		return Status;
	}

    SIZE_T NeedSize = sizeof(RESULT) + Param.NumOfBytes;
    if (OutputBufferLength < NeedSize) {
        *Information = NeedSize;
        goto Exit;
    }
    PVOID ReadBuffer = PTR_ADD_OFFSET(OutputBuffer, sizeof(RESULT));
    Result->Status = ReadProcessVirtualMemory(Param.ProcessId, Param.Address, ReadBuffer, Param.NumOfBytes,&Result->SizeOfData);
    *Information = sizeof(RESULT) + Result->SizeOfData;
Exit:
    return STATUS_SUCCESS;
}

NTSTATUS
HandleWriteProcessMemory(
    PDEVICE_OBJECT DeviceObject,
    ULONG InputBufferLength,
    ULONG OutputBufferLength,
    PVOID InputBuffer,
    PVOID OutputBuffer,
    PULONG_PTR Information
)
{
    UNREFERENCED_PARAMETER(DeviceObject);
    typedef struct _PARAM {
        HANDLE ProcessId;
        PVOID Address;
        SIZE_T NumOfBytes;
    }PARAM;
    if (InputBufferLength < sizeof(PARAM)) {
        return STATUS_BUFFER_TOO_SMALL;
    }
    // Copy Param
    PARAM Param = *(PARAM*)InputBuffer;
    SIZE_T NeedSize = sizeof(PARAM);
    if (InputBufferLength < NeedSize) {
        return STATUS_BUFFER_TOO_SMALL;
    }

    // Write Memory
    PVOID WriteBuffer = PTR_ADD_OFFSET(InputBuffer, sizeof(PARAM));
    SIZE_T BytesToWrite = 0;
    NTSTATUS CallStatus = WriteProcessVirtualMemory(Param.ProcessId, Param.Address, WriteBuffer, Param.NumOfBytes, &BytesToWrite);

    // Fill Result
	PRESULT Result;
    NTSTATUS Status = CreateResult(OutputBuffer, OutputBufferLength,Information, &Result);
	if (!NT_SUCCESS(Status)) {
		return Status;
	}
    Result->Status = CallStatus;
    WriteResult(OutputBufferLength, Information, Result, (PVOID)&BytesToWrite, sizeof(SIZE_T));

    return STATUS_SUCCESS;
}

NTSTATUS
HandleAllocatePhysicalMemory(
    PDEVICE_OBJECT DeviceObject,
    ULONG InputBufferLength,
    ULONG OutputBufferLength,
    PVOID InputBuffer,
    PVOID OutputBuffer,
    PULONG_PTR Information
)
{
    UNREFERENCED_PARAMETER(DeviceObject);
   
    typedef struct _PARAM {
        SIZE_T NumOfBytes;
    }PARAM;
    if (InputBufferLength < sizeof(PARAM)) {
        return STATUS_BUFFER_TOO_SMALL;
    }
    // Copy Param
    PARAM Param = *(PARAM*)InputBuffer;
    SIZE_T NeedSize = sizeof(PARAM);
    if (InputBufferLength < NeedSize) {
        return STATUS_BUFFER_TOO_SMALL;
    }

    PRESULT Result;
    NTSTATUS Status = CreateResult(OutputBuffer, OutputBufferLength, Information, &Result);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    // Allocate Physical Memory
    PHYSICAL_ADDRESS PhysicallAddress = { .QuadPart = MAXULONG64 };
    PVOID VirtuaAddress = MmAllocateContiguousMemory(Param.NumOfBytes, PhysicallAddress);

    if (VirtuaAddress == NULL) {
        Result->Status = STATUS_NO_MEMORY;
    }
    else {
        Result->Status = STATUS_SUCCESS;
        PHYSICAL_ADDRESS AllocateAddress = MmGetPhysicalAddress(VirtuaAddress);
		WriteResult(OutputBufferLength, Information, Result, (PVOID)&AllocateAddress, sizeof(PHYSICAL_ADDRESS));
    }

    return STATUS_SUCCESS;
}

NTSTATUS
HandleFreePhysicalMemory(
    PDEVICE_OBJECT DeviceObject,
    ULONG InputBufferLength,
    ULONG OutputBufferLength,
    PVOID InputBuffer,
    PVOID OutputBuffer,
    PULONG_PTR Information
)
{
    DbgBreakPoint();
    UNREFERENCED_PARAMETER(DeviceObject);

    typedef struct _PARAM {
        PVOID Address;
    }PARAM;
    if (InputBufferLength < sizeof(PARAM)) {
        return STATUS_BUFFER_TOO_SMALL;
    }
    // Copy Param
    PARAM Param = *(PARAM*)InputBuffer;
    SIZE_T NeedSize = sizeof(PARAM);
    if (InputBufferLength < NeedSize) {
        return STATUS_BUFFER_TOO_SMALL;
    }

    PRESULT Result;
    NTSTATUS Status = CreateResult(OutputBuffer, OutputBufferLength, Information, &Result);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

	PHYSICAL_ADDRESS PhysicallAddress = { .QuadPart = (ULONG_PTR)Param.Address };
    if (Param.Address != NULL) {
		// Free Physical Memory
        PVOID VirtualAddress = MmGetVirtualForPhysical(PhysicallAddress);
        if (MmIsAddressValid(VirtualAddress)) {
			MmFreeContiguousMemory(VirtualAddress);
        }
		Result->Status = STATUS_SUCCESS;
    }
    else {
        Result->Status = STATUS_INVALID_ADDRESS;
    }

    return STATUS_SUCCESS;
}

NTSTATUS
HandleReaPhysicalMemory(
    PDEVICE_OBJECT DeviceObject,
    ULONG InputBufferLength,
    ULONG OutputBufferLength,
    PVOID InputBuffer,
    PVOID OutputBuffer,
    PULONG_PTR Information
)
{
    UNREFERENCED_PARAMETER(DeviceObject);

    typedef struct _PARAM {
        PVOID Address;
        SIZE_T NumOfBytes;
    }PARAM;

    if (InputBufferLength < sizeof(PARAM)) {
        return STATUS_BUFFER_TOO_SMALL;
    }
    // Copy Param
    PARAM Param = *(PARAM*)InputBuffer;

    PRESULT Result;
    NTSTATUS Status = CreateResult(OutputBuffer, OutputBufferLength, Information, &Result);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    SIZE_T NeedSize = sizeof(RESULT) + Param.NumOfBytes;
    if (OutputBufferLength < NeedSize) {
        *Information = NeedSize;
        goto Exit;
    }
    PVOID ReadBuffer = PTR_ADD_OFFSET(OutputBuffer, sizeof(RESULT));
    Result->Status = ReadPhysicalMemory(Param.Address, ReadBuffer, Param.NumOfBytes, &Result->SizeOfData);
    *Information = sizeof(RESULT) + Result->SizeOfData;
Exit:
    return STATUS_SUCCESS;
}


NTSTATUS
HandleWritePhysicalMemory(
    PDEVICE_OBJECT DeviceObject,
    ULONG InputBufferLength,
    ULONG OutputBufferLength,
    PVOID InputBuffer,
    PVOID OutputBuffer,
    PULONG_PTR Information
)
{
    UNREFERENCED_PARAMETER(DeviceObject);
    typedef struct _PARAM {
        PVOID Address;
        SIZE_T NumOfBytes;
    }PARAM;
    if (InputBufferLength < sizeof(PARAM)) {
        return STATUS_BUFFER_TOO_SMALL;
    }
    // Copy Param
    PARAM Param = *(PARAM*)InputBuffer;
    SIZE_T NeedSize = sizeof(PARAM);
    if (InputBufferLength < NeedSize) {
        return STATUS_BUFFER_TOO_SMALL;
    }

    // Write Memory
    PVOID WriteBuffer = PTR_ADD_OFFSET(InputBuffer, sizeof(PARAM));
    SIZE_T BytesToWrite = 0;
    NTSTATUS CallStatus = WritePhysicalMemory(Param.Address, WriteBuffer, Param.NumOfBytes, &BytesToWrite);

    // Fill Result
    PRESULT Result;
    NTSTATUS Status = CreateResult(OutputBuffer, OutputBufferLength, Information, &Result);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }
    Result->Status = CallStatus;
    WriteResult(OutputBufferLength, Information, Result, (PVOID)&BytesToWrite, sizeof(SIZE_T));

    return STATUS_SUCCESS;
}
