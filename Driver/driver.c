#include "driver.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,DriverEntry)
#pragma alloc_text(PAGE,IoctlDispatchRoutine)
#pragma alloc_text(PAGE,DispatchCreate)
#endif

e(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
    UNREFERENCED_PARAMETER(pDevObj);
    pIrp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

NTSTATUS DispatchCreate(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
    UNREFERENCED_PARAMETER(pDevObj);
    pIrp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

NTSTATUS DispatchClose(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
    UNREFERENCED_PARAMETER(pDevObj);
    pIrp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

NTSTATUS DefDispatchRoutine(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
    UNREFERENCED_PARAMETER(pDevObj);
    NTSTATUS status = STATUS_SUCCESS;
    pIrp->IoStatus.Status = status;
    pIrp->IoStatus.Information = 0;
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);
    return status;
}

NTSTATUS DriverUnload(PDRIVER_OBJECT DriverObject) {
    DbgPrint("Call DriverUnload\n");
    UNICODE_STRING SymbolLinkName;
    if (DriverObject->DeviceObject) {
        IoDeleteDevice(DriverObject->DeviceObject);
    }
    // Callback
    HandleDriverUnload(DriverObject);
    RtlInitUnicodeString(&SymbolLinkName, SYMBOL_LINK_NAME);
    return IoDeleteSymbolicLink(&SymbolLinkName);
}

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING LoadPath) {
    UNREFERENCED_PARAMETER(LoadPath);
    NTSTATUS status = STATUS_SUCCESS;
    PDEVICE_OBJECT Device = NULL;
    UNICODE_STRING DeviceName, SymbolLinkName;

    RtlInitUnicodeString(&DeviceName, DEVICE_NAME);
    RtlInitUnicodeString(&SymbolLinkName, SYMBOL_LINK_NAME);
    status = IoCreateDevice(DriverObject, 200, &DeviceName, FILE_DEVICE_UNKNOWN, 0, 1, &Device);
    if (!NT_SUCCESS(status)) {
        DbgPrint("Call IoCreateDevice Error: %X\n", status);
        return status;
    }
    Device->Flags |= DO_BUFFERED_IO;
    DbgPrint("Create Device Successful!\n");
    status = IoCreateSymbolicLink(&SymbolLinkName, &DeviceName);
    if (!NT_SUCCESS(status)) {
        DbgPrint("Call IoCreateSymbolicLink Error: %X\n", status);
        IoDeleteDevice(Device);
        return status;
    }
    DbgPrint("Create SymbolLink Successful!\n");

    DriverObject->DriverUnload = DriverUnload;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = IoctlDispatchRoutine;
    DriverObject->MajorFunction[IRP_MJ_CREATE] = DispatchCreate;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = DispatchClose;
    DriverObject->MajorFunction[IRP_MJ_WRITE] = DefDispatchRoutine;
    DriverObject->MajorFunction[IRP_MJ_READ] = DefDispatchRoutine;

    // Callback
    HandleDriverLoad(DriverObject);
    return status;
}