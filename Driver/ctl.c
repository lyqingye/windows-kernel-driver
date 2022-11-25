#include "driver.h"

VOID 
WriteResult(
    PVOID OutputBuffer, 
    ULONG OutputBufferLength, 
    PULONG_PTR Information, 
    PRESULT Result
) {
    if (Information && Result) {
        SIZE_T SizeOfMeta = sizeof(NTSTATUS) + sizeof(SIZE_T);
        SIZE_T SizeOfResult = SizeOfMeta + Result->SizeOfData;
        // acture output length
        (*Information) = (ULONG_PTR)(SizeOfResult);

        if (OutputBuffer && (OutputBufferLength >= SizeOfMeta)) {
            // copy Status & SizeOfData
            RtlZeroMemory(OutputBuffer, SizeOfMeta);
            PUINT8 Out = (PUINT8)OutputBuffer;
            RtlCopyMemory((PVOID)Out, (PVOID)Result, sizeof(NTSTATUS));
            Out += sizeof(NTSTATUS);
            RtlCopyMemory((PVOID)Out, (PVOID)(&(Result->SizeOfData)), sizeof(SIZE_T));

            // copy data
            if (OutputBufferLength >= SizeOfResult && Result->Data && Result->SizeOfData) {
                Out += sizeof(SIZE_T);
                RtlZeroMemory((PVOID)Out, Result->SizeOfData);
                RtlCopyMemory((PVOID)Out, Result->Data, Result->SizeOfData);
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
    UNREFERENCED_PARAMETER(pDevObj);

    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    PIO_STACK_LOCATION IoStackLocation = IoGetCurrentIrpStackLocation(pIrp);
    PVOID Input = pIrp->AssociatedIrp.SystemBuffer;
    PVOID Output = pIrp->AssociatedIrp.SystemBuffer;
    ULONG Code = IoStackLocation->Parameters.DeviceIoControl.IoControlCode;
    ULONG InputBufferLength = IoStackLocation->Parameters.DeviceIoControl.InputBufferLength;
    ULONG OutputBufferLength = IoStackLocation->Parameters.DeviceIoControl.OutputBufferLength;
    ULONG_PTR Information = 0;

    UNREFERENCED_PARAMETER(Input);
    UNREFERENCED_PARAMETER(Output);
    UNREFERENCED_PARAMETER(InputBufferLength);
    UNREFERENCED_PARAMETER(OutputBufferLength);

    PAGED_CODE();
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
    default:
        break;
    }
    pIrp->IoStatus.Status = Status;
    pIrp->IoStatus.Information = Information;
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);
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

    NTSTATUS Status = STATUS_SUCCESS;
#ifdef DBG
    DbgPrint("[Dispatch][Echo] InputBufferLength: %d\n", InputBufferLength);
    DbgPrint("[Dispatch][Echo] OutputBufferLength: %d\n", OutputBufferLength);
#endif 

    if (InputBuffer && InputBufferLength > 0) {
        PVOID EchoBuffer = ExAllocatePoolZero(NonPagedPool, InputBufferLength, 'echo');
        if (EchoBuffer == NULL) {
            Status = STATUS_NO_MEMORY;
#ifdef DBG
            DbgPrint("[Dispatch][Echo] allocate echo buffer error!\n");
#endif 
        }
        else {
            // copy input data to buffer
            RtlCopyMemory(EchoBuffer, InputBuffer, InputBufferLength);
            RESULT Result;
            Result.Status = STATUS_SUCCESS;
            Result.SizeOfData = 0;

            if (OutputBuffer && OutputBufferLength > 0) {
                Result.SizeOfData = InputBufferLength;
                Result.Data = EchoBuffer;
            }
            WriteResult(OutputBuffer, OutputBufferLength, Information, &Result);
            ExFreePool(EchoBuffer);
        }
    }

    return Status;
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

    RESULT Result;
    Result.Status = InitializationGlobalContext(
        DeviceObject->DriverObject,
        InputBuffer,
        InputBufferLength,
        &GlobalContext);
    Result.SizeOfData = 0;
    Result.Data = NULL;
    WriteResult(OutputBuffer, OutputBufferLength, Information, &Result);
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
    RESULT Result;
    Result.Status = QueryNtosKrnlModuleInformation(DeviceObject->DriverObject, &Entry);
    if (NT_SUCCESS(Result.Status)) {
        Result.SizeOfData = sizeof(SYSTEM_MODULE_ENTRY);
        Result.Data = (PVOID)&Entry;
    }
    WriteResult(OutputBuffer, OutputBufferLength, Information, &Result);
    return STATUS_SUCCESS;
}

