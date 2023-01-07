#include "driver.h"

NTSTATUS
RemoveSystemModuleFromLoadedModuleList(
	PUNICODE_STRING ModuleBaseName,
	PLIST_ENTRY* RemovedEntry
)
{
	NTSTATUS Status = STATUS_NOT_FOUND;
	PLIST_ENTRY NextEntry = NULL;

	KeEnterCriticalRegion();
	if (ExAcquireResourceExclusiveLite(*PsLoadedModuleResource, TRUE)) {
		NextEntry = (*PsLoadedModuleList)->Flink;

		while (NextEntry != NULL && NextEntry != *PsLoadedModuleList) {
			PKLDR_DATA_TABLE_ENTRY DataTableEntry = CONTAINING_RECORD(NextEntry,
				KLDR_DATA_TABLE_ENTRY,
				InLoadOrderLinks);

			DbgPrint("%wZ %p\n", &DataTableEntry->BaseDllName, DataTableEntry->DllBase);
			if (RtlCompareUnicodeString(&DataTableEntry->BaseDllName, ModuleBaseName, TRUE) == 0) {
				RemoveEntryList(NextEntry);
				if (RemovedEntry != NULL) {
					*RemovedEntry = NextEntry;
				}
				Status = STATUS_SUCCESS;
				break;
			}
			NextEntry = NextEntry->Flink;
		}

		ExReleaseResourceLite(*PsLoadedModuleResource);
	}

	KeLeaveCriticalRegion();
	return Status;
}

VOID
InsertModuleEntryToLoadedModuleList(
	PLIST_ENTRY* Entry
)
{
	KeEnterCriticalRegion();
	if (ExAcquireResourceExclusiveLite(*PsLoadedModuleResource, TRUE)) {
		InsertTailList(*PsLoadedModuleList, *Entry);
		ExReleaseResourceLite(*PsLoadedModuleResource);
	}
	KeLeaveCriticalRegion();
}

NTSTATUS
ReadVirtualMemoryById(
	HANDLE ProcessId,
	PVOID VirtualAddress,
	PVOID Buffer,
	SIZE_T Bytes,
	PSIZE_T BytesRead
)
{
	PEPROCESS Eprocess = NULL;
	if (!NT_SUCCESS(PsLookupProcessByProcessId(ProcessId, &Eprocess))) {
		return STATUS_UNSUCCESSFUL;
	}
	return ReadVirtualMemory(Eprocess, VirtualAddress, Buffer, Bytes, BytesRead);
}

NTSTATUS
WriteVirtualMemoryById(
	HANDLE ProcessId,
	PVOID VirtualAddress,
	PVOID Buffer,
	SIZE_T Bytes,
	PSIZE_T BytesWrite
)
{
	PEPROCESS Eprocess = NULL;
	if (!NT_SUCCESS(PsLookupProcessByProcessId(ProcessId, &Eprocess))) {
		return STATUS_UNSUCCESSFUL;
	}
	return WriteVirtualMemory(Eprocess, VirtualAddress, Buffer, Bytes, BytesWrite);
}


NTSTATUS
ReadVirtualMemory(
	PEPROCESS Eprocess,
	PVOID VirtualAddress,
	PVOID Buffer,
	SIZE_T Bytes,
	PSIZE_T BytesRead
)
{
	PVOID PageTable = *(PVOID *)PTR_ADD_OFFSET(Eprocess, OffsetDirectoryTableBaseOfEProcess);
	if (PageTable == NULL) {
		return STATUS_INVALID_PARAMETER;
	}

	PVOID PA = TranslateVirtualAddressToPhysicalAddress(PageTable,VirtualAddress);
	if (PA == NULL) {
		return STATUS_UNSUCCESSFUL;
	}

	return ReadPhysicalMemory(PA, Buffer, Bytes, BytesRead);
}

NTSTATUS
WriteVirtualMemory(
	PEPROCESS Eprocess,
	PVOID VirtualAddress,
	PVOID Buffer,
	SIZE_T Bytes,
	PSIZE_T BytesWrite
)
{
	PVOID PageTable = *(PVOID *)PTR_ADD_OFFSET(Eprocess, OffsetDirectoryTableBaseOfEProcess);
	if (PageTable == NULL) {
		return STATUS_INVALID_PARAMETER;
	}

	PVOID PA = TranslateVirtualAddressToPhysicalAddress(PageTable,VirtualAddress);
	if (PA == NULL) {
		return STATUS_UNSUCCESSFUL;
	}

	return WritePhysicalMemory(PA, Buffer, Bytes, BytesWrite);
}

NTSTATUS
ReadPhysicalMemory(
	PVOID PhysicalAddress,
	PVOID Buffer,
	SIZE_T Bytes,
	PSIZE_T BytesRead
)
{
	MM_COPY_ADDRESS AddressToRead = { 0 };
	AddressToRead.PhysicalAddress.QuadPart = (ULONG_PTR)PhysicalAddress;
	return MmCopyMemory(Buffer, AddressToRead, Bytes, MM_COPY_MEMORY_PHYSICAL, BytesRead);
}

NTSTATUS
WritePhysicalMemory(
	PVOID PhysicalAddress,
	PVOID Buffer,
	SIZE_T Bytes,
	PSIZE_T BytesWrite
)
{
	PHYSICAL_ADDRESS AddressToWrite = { 0 };
	AddressToWrite.QuadPart = (ULONG_PTR)PhysicalAddress;

	PVOID Mapped = MmMapIoSpaceEx(AddressToWrite, Bytes, PAGE_READWRITE);
	if (!Mapped) {
		return STATUS_UNSUCCESSFUL;
	}
	RtlCopyMemory(Buffer, Mapped, Bytes);
	*BytesWrite = Bytes;
	MmUnmapIoSpace(Mapped, Bytes);
	return STATUS_SUCCESS;
}

PVOID
TranslateVirtualAddressToPhysicalAddress(
	PVOID PageTable,
	PVOID VirtualAddress
)
{
	const ULONG_PTR PMASK = 0x0000FFFFFFFFF000;
	ULONG_PTR Cr3 = (ULONG_PTR)PageTable;
	ULONG_PTR VA = (ULONG_PTR)VirtualAddress;
	Cr3 &= ~0xf;
	ULONG_PTR PageOffset = VA & 0xfff;
	ULONG_PTR IndexOfPTE = ((VA >> 12) & (0x1ffll));
	ULONG_PTR IndexOfPDE = ((VA >> 21) & (0x1ffll));
	ULONG_PTR IndexOfPDPTE = ((VA >> 30) & (0x1ffll));
	ULONG_PTR IndexOfPML4 = ((VA >> 39) & (0x1ffll));

	SIZE_T ReadSize = 0;
	ULONG_PTR PDPTE = 0;
	ULONG_PTR PDE = 0;
	ULONG_PTR PTE = 0;
	ULONG_PTR Page = 0;
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	Status = ReadPhysicalMemory((PVOID)(Cr3 + IndexOfPML4 * 8), (PVOID)&PDPTE, 8, &ReadSize);
	if (!NT_SUCCESS(Status) || ReadSize != 8 || ((PDPTE & 1) == 0)) {
		return NULL;
	}

	PDPTE = PDPTE & PMASK;
	Status = ReadPhysicalMemory((PVOID)(PDPTE + IndexOfPDPTE * 8), (PVOID)&PDE, 8, &ReadSize);
	if (!NT_SUCCESS(Status) || ReadSize != 8 || ((PDE & 1) == 0)) {
		return NULL;
	}

	// 1GB Page
	if (PDE & 0x80) {
		PageOffset = VA & 0x3FFFFFFF;
		return (PVOID)(PDE & PMASK + PageOffset);
	}
	
	PDE &= PMASK;
	Status = ReadPhysicalMemory((PVOID)(PDE + IndexOfPDE * 8), (PVOID)&PTE, 8, &ReadSize);
	if (!NT_SUCCESS(Status) || ReadSize != 8 || ((PTE & 1) == 0)) {
		return NULL;
	}

	// 2MB Page
	if (PTE & 0x80) {
		PTE &= PMASK;
		PageOffset = VA & 0x1FFFFF;
		return (PVOID)(PTE + PageOffset);
	}

	// 4KB Page
	PTE &= PMASK;
	Status = ReadPhysicalMemory((PVOID)(PTE + IndexOfPTE * 8), (PVOID)&Page, 8, &ReadSize);
	if (!NT_SUCCESS(Status) || ReadSize != 8) {
		return NULL;
	}
	
	Page &= PMASK;
	if (Page == 0) {
		return NULL;
	}

	return (PVOID)(Page + PageOffset);
}