#include "driver.h"

VOID HandleDriverLoad(PDRIVER_OBJECT DriverObject) {
    UNREFERENCED_PARAMETER(DriverObject);
}

VOID HandleDriverUnload(PDRIVER_OBJECT DriverObject) {
    UNREFERENCED_PARAMETER(DriverObject);
}


NTSTATUS IoctlDispatchRoutine(PDEVICE_OBJECT pDevObj, PIRP pIrp) {
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
        //Status = HandleEcho(pDevObj, InputBufferLength, OutputBufferLength, Input, Output, &Information);
        break;
    case CTL_CODE_INIT_CONTEXT:
        // Status = HandleInitializationGlobalContext(pDevObj, InputBufferLength, OutputBufferLength, Input, Output, &Information);
        break;
    case CTL_CODE_QUERY_KERNEL_MODULE_INFO:
        // Status = HandleQueryNtosKrnlModuleInformation(pDevObj, InputBufferLength, OutputBufferLength, Input, Output, &Information);
        break;
    default:
        break;
    }
    pIrp->IoStatus.Status = Status;
    pIrp->IoStatus.Information = Information;
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);
    return Status;
}


